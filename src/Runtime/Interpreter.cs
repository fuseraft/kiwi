using kiwi.Runtime.Builtin.Operation;
using kiwi.Runtime.Builtin.Dispatcher;
using kiwi.Parsing;
using kiwi.Parsing.AST;
using kiwi.Typing;
using kiwi.Tracing;
using kiwi.Tracing.Error;
using kiwi.Parsing.Keyword;
using kiwi.Runtime.Builtin.Handler;
using kiwi.Settings;
using kiwi.Runtime.Builtin.Util;

namespace kiwi.Runtime;

public class Interpreter
{
    private Scope _globalScope = new();
    private readonly Stack<Scope> _scopePool = new(32);
    private FrameManager _frames = null!;

    public Interpreter()
    {
        _globalScope.Declare("global", Value.CreateHashmap());
        InitManagers();
    }

    // Used by generator threads to share the global scope and context
    internal Interpreter(Scope globalScope, KContext context)
    {
        _globalScope = globalScope;
        Context = context;
        InitManagers();
    }

    private void InitManagers()
    {
        _frames = new FrameManager(CallStack, FuncStack, _scopePool, _globalScope, () => Context);
    }

    private GeneratorRef? _activeGenerator;

    public Dictionary<string, string> CliArgs { get; set; } = [];
    public KContext Context { get; private set; } = new();
    public string ExecutionPath { get; set; } = string.Empty;
    public string EntryPath { get; set; } = string.Empty;
    /// <summary>Working directory at the time kiwi was invoked — used as the
    /// project root when resolving bare include paths.</summary>
    public string ProjectRoot { get; set; } = Directory.GetCurrentDirectory();
    public CancellationToken GeneratorCancellationToken { get; private set; } = CancellationToken.None;
    public Stack<StackFrame> CallStack { get; set; } = [];
    internal Stack<string> PackageStack { get; set; } = [];
    internal Stack<string> StructStack  { get; set; } = [];
    private Stack<string> FuncStack { get; set; } = [];
    public long CurrentTaskId { get; set; } = 0; // 0 = main thread
    public void SetContext(KContext context) => Context = context;

    private List<string> CaptureStackTrace() => _frames.CaptureStackTrace();

    /// <summary>
    /// Fallback single-node executor: compiles the node as a mini-program and
    /// runs it through the VM.  Called from AST-walk helpers that have not yet
    /// been replaced by pure-VM code (e.g. InvokeEvent, ExecuteBody).
    /// </summary>
    private Value Interpret(ASTNode? node)
    {
        if (node == null) return Value.Default;
        var prog = new ProgramNode([node!]);
        var chunk = VM.Compiler.CompileExpression(prog);
        var savedVM = VM.KiwiVM.Current;
        try   { return new VM.KiwiVM(this).Execute(chunk); }
        finally { VM.KiwiVM.Current = savedVM; }
    }

    public Value FuncToLambda(KFunction func)
    {
        // Register the function as a lambda in Context.Lambdas so InvokeCallable can find it.
        var lambdaId = func.Name;
        var lambdaDecl = func.Decl.ToLambda();
        var kl = new KLambda(lambdaDecl)
        {
            Parameters        = func.Parameters,
            DefaultParameters = func.DefaultParameters,
            TypeHints         = func.TypeHints,
            ReturnTypeHint    = func.ReturnTypeHint,
            CapturedScope     = func.CapturedScope,
            VariadicParamName = func.VariadicParamName,
            VMChunk           = func.VMChunk,
            VMUpvalues        = func.VMUpvalues,
            Ref               = new LambdaRef { Identifier = lambdaId }
        };
        Context.Lambdas[lambdaId] = kl;
        return Value.CreateLambda(new LambdaRef { Identifier = lambdaId, VMChunk = func.VMChunk, VMUpvalues = func.VMUpvalues });
    }

    public Value InvokeCallable(Callable callable, List<Value> args, Token token, string displayName, InstanceRef? instance = null)
    {
        // If this callable was compiled as a VM function/lambda, delegate to the VM so the
        // bytecode body executes instead of the synthetic (empty) AST Decl.Body.
        var vm = VM.KiwiVM.Current;
        if (vm != null)
        {
            if (callable is KFunction kf && kf.VMChunk != null)
                return vm.InvokeVMCallable(kf, args, token, instance);
            if (callable is KLambda kl && kl.VMChunk != null)
                return vm.InvokeVMCallable(kl, args, token, instance);
        }

        // Rent a Scope from the pool (or allocate if empty).
        // Reset() sets IsBlockScope=true so the scope is eligible to be returned after the call.
        //
        // Package functions run in an isolated scope with no parent chain into the
        // caller's scope.  This prevents their internal local variables (e.g. a loop
        // counter named `count`) from silently clobbering same-named variables in
        // the caller.  Non-package callables (closures, lambdas, …) keep the normal
        // captured-scope parent so they can close over outer bindings.
        var parentScope = (callable is KFunction { IsPackageFunction: true })
            ? null
            : (callable.CapturedScope ?? _globalScope);
        var scope = _scopePool.Count > 0 ? _scopePool.Pop() : new Scope();
        scope.Reset(parentScope);

        var frame = new StackFrame(displayName, scope, token);

        BindParameters(callable, args, token, displayName, scope);

        // Set object context for methods
        if (instance != null)
        {
            frame.SetObjectContext(instance);
        }

        PushFrame(frame, callable is KLambda);
        if (callable is KLambda)
        {
            frame.SetFlag(FrameFlags.InLambda);
        }

        try
        {
            var body = callable switch
            {
                KFunction f => f.Decl.Body,
                KLambda l => l.Decl.Body,
                _ => throw new NotSupportedException()
            };

            var result = ExecuteBody(body, frame);

            // Validate return type
            if (!AssertTypeMatch(token, result, callable.ReturnTypeHint))
            {
                throw new TypeError(token,
                    $"Expected `{string.Join("|", callable.ReturnTypeHint.Select(TypeRegistry.GetTypeName))}` " +
                    $"from `{displayName}` but got `{TypeRegistry.GetTypeName(result)}`.");
            }

            return result;
        }
        finally
        {
            PopFrame();
            // Return the call scope to the pool if no nested lambda captured it.
            // Cap the pool at 64 to avoid unbounded growth in deeply-recursive code.
            if (scope.CanPool && _scopePool.Count < 64)
                _scopePool.Push(scope);
        }
    }

    public Value InvokeEvent(Token token, LambdaRef lambda, List<Value> args)
    {
        var doPop = false;

        try
        {
            var lambdaName = lambda.Identifier;
            var scope = CallStack.Count > 0 ? new Scope(CallStack.Peek().Scope) : new Scope();
            var lambdaFrame = PushFrame(lambdaName, token, scope, true);
            var result = Value.Default;
            var targetLambda = lambdaName;

            if (!Context.HasLambda(targetLambda))
            {
                if (!Context.HasMappedLambda(targetLambda))
                {
                    throw new CallableError(token, $"Could not find target lambda `{targetLambda}`");
                }

                targetLambda = Context.LambdaTable[targetLambda];
            }

            var func = Context.Lambdas[targetLambda];

            // If the lambda was compiled to VM bytecode, delegate to a fresh per-task VM.
            // InvokeEvent is called from TaskManager.Spawn on a thread-pool thread.  Thread-pool
            // threads are reused, so KiwiVM.Current may still hold a stale VM from a previous
            // task on this thread - always create a new one here.
            // Use func.CapturedScope (= main interpreter's global scope) so the task can see
            // globals like DB_PATH, HOST, etc. that were set at program startup.
            if (func.VMChunk != null)
            {
                PopFrame(); // undo the lambdaFrame pushed above
                var vm = new VM.KiwiVM(this, func.CapturedScope ?? GetGlobalScope());
                return vm.InvokeVMCallable(func, args, token, null);
            }

            var typeHints = func.TypeHints;
            var returnTypeHint = func.ReturnTypeHint;
            var defaultParameters = func.DefaultParameters;

            PrepareLambdaCall(func, args, defaultParameters, token, targetLambda, typeHints, lambdaName, scope);

            lambdaFrame.SetFlag(FrameFlags.InLambda);
            doPop = true;

            var decl = func.Decl.Body;
            foreach (var stmt in decl)
            {
                result = Interpret(stmt);
                if (lambdaFrame.IsFlagSet(FrameFlags.Return))
                {
                    result = lambdaFrame.ReturnValue ?? result;
                    break;
                }
            }

            if (!AssertTypeMatch(token, result, returnTypeHint))
            {
                throw new TypeError(token, $"Expected type `{string.Join("|", returnTypeHint.Select(TypeRegistry.GetTypeName))}` for return type of `{lambdaName}` but received `{TypeRegistry.GetTypeName(result)}`.");
            }

            return result;
        }
        catch
        {
            throw; // Let TaskManager.Spawn catch and record it; don't call DumpCrashLog
        }
        finally
        {
            if (doPop)
            {
                PopFrame();
            }
        }
    }

    private string GetCurrentStructName(Token token)
    {
        var frame = CallStack.Peek();

        if (!string.IsNullOrEmpty(frame.StructName))
        {
            return frame.StructName;
        }

        if (frame.InObjectContext())
        {
            var sn = frame.GetObjectContext()?.StructName ?? string.Empty;
            if (!string.IsNullOrEmpty(sn))
            {
                return sn;
            }
        }

        throw new InvalidContextError(token, "Cannot access static variable outside of a struct method.");
    }

    /// <summary>
    /// Public entry point used by the VM's Include opcode.
    /// Loads and interprets a Kiwi file by path (include-once semantics).
    /// Ensures the interpreter call stack has a frame for the duration of the call,
    /// since code inside the included file may define functions that call CaptureCurrentScope().
    /// </summary>
    public Value IncludeFile(Token token, string filePath)
    {
        var fullPath = ResolveIncludePath(token, filePath);

        if (Context.Includes.Contains(fullPath))
            return Value.Default;

        Context.Includes.Add(fullPath);

        if (!FileUtil.FileExists(token, fullPath))
            throw new InvalidOperationError(token, $"File not found: {filePath}");

        var oldExecutionPath = ExecutionPath;
        var savedVM = VM.KiwiVM.Current;
        ExecutionPath = fullPath;
        try
        {
            using var lexer = new Lexer(fullPath);
            var ast = new Parser(true).ParseTokenStream(lexer.GetTokenStream(), true);
            var chunk = VM.Compiler.CompileProgram((ProgramNode)ast);
            new VM.KiwiVM(this).Execute(chunk);
        }
        finally
        {
            VM.KiwiVM.Current = savedVM;
            ExecutionPath = oldExecutionPath;
        }

        return Value.Default;
    }

    /// <summary>
    /// Resolves an include path using two-phase lookup:
    ///   1. Explicit-relative paths (starting with ./ or ../) always resolve
    ///      relative to the currently executing file — same as before.
    ///   2. All other (bare) paths are tried relative to the entry-script's
    ///      directory (the project root) first, then fall back to the current
    ///      file's directory.
    /// This matches the behaviour developers expect from languages like Python,
    /// Ruby, and Node.js, where bare names resolve from the project root while
    /// explicit relative paths resolve from the caller.
    /// </summary>
    private string ResolveIncludePath(Token token, string filePath)
    {
        var currentDir = FileUtil.GetParentPath(token, ExecutionPath);

        bool isExplicitRelative = filePath.StartsWith("./",  StringComparison.Ordinal)
                               || filePath.StartsWith("../", StringComparison.Ordinal);

        if (isExplicitRelative)
        {
            // Explicit relative — always resolve from the including file's directory.
            return FindWithExtensions(Path.GetFullPath(Path.Combine(currentDir, filePath)));
        }

        // Bare path — try the project root (CWD at startup) first.
        var fromRoot = FindWithExtensions(Path.GetFullPath(Path.Combine(ProjectRoot, filePath)));
        if (File.Exists(fromRoot))
        {
            return fromRoot;
        }

        // Fall back to the including file's directory (preserves prior behaviour).
        return FindWithExtensions(Path.GetFullPath(Path.Combine(currentDir, filePath)));
    }

    // Try the path as-is, then with each recognised Kiwi extension in order.
    private static string FindWithExtensions(string p)
    {
        if (Path.HasExtension(p))
        {
            return p;
        }

        foreach (var ext in Kiwi.Settings.Extensions.Recognized)
        {
            if (File.Exists(p + ext))
            {
                return p + ext;
            }
        }

        // Return with the primary extension so the error message is readable.
        return p + Kiwi.Settings.Extensions.Primary;
    }

    private bool AssertTypeMatch(Token t, Value v, List<int> types) =>
        types.Any(type => AssertSingleTypeMatch(t, v, type));

    private bool AssertSingleTypeMatch(Token t, Value v, int type)
    {
        string typeName = TypeRegistry.GetTypeName(type);

        var isTypeMatch = typeName switch
        {
            "any" => true,
            "boolean" => v.IsBoolean(),
            "float" => v.IsFloat(),
            "hashmap" => v.IsHashmap(),
            "integer" => v.IsInteger(),
            "date" => v.IsDate(),
            "generator" => v.IsGenerator(),
            "lambda" => v.IsLambda(),
            "list" => v.IsList(),
            "none" => v.IsNull(),
            "object" => v.IsObject(),
            "string" => v.IsString(),
            "pointer" => v.IsPointer(),
            "bytes" => v.IsBytes(),
            _ => false,
        };

        if (!isTypeMatch && v.IsObject())
        {
            var structName = v.GetObject().StructName;
            isTypeMatch = TypeRegistry.GetType(t, structName) == type;

            if (!isTypeMatch)
            {
                var struc = Context.Structs[structName];
                var baseStruct = struc.BaseStruct;

                isTypeMatch = TypeRegistry.GetType(t, baseStruct) == type;
            }
        }

        return isTypeMatch;
    }

    private void BindParameters(Callable callable, List<Value> args, Token token, string name, Scope scope)
    {
        for (int i = 0; i < callable.Parameters.Count; i++)
        {
            var param = callable.Parameters[i];
            var argValue = i < args.Count ? args[i] :
                        callable.DefaultParameters.Contains(param.Key) ? param.Value :
                        throw new ParameterCountMismatchError(token, name, callable.Parameters.Count, args.Count);

            // Type hint check
            if (callable.TypeHints.TryGetValue(param.Key, out var hints) &&
                !AssertTypeMatch(token, argValue, hints))
            {
                throw new TypeError(token,
                    $"Parameter {i + 1} of `{name}` expected `{string.Join("|", hints.Select(TypeRegistry.GetTypeName))}`, " +
                    $"got `{TypeRegistry.GetTypeName(argValue)}`.");
            }

            scope.Declare(param.Key, argValue);
        }

        // Collect overflow args into variadic list
        if (!string.IsNullOrEmpty(callable.VariadicParamName))
        {
            var varargs = args.Skip(callable.Parameters.Count).ToList();
            scope.Declare(callable.VariadicParamName, Value.CreateList(varargs));
        }
    }

    private Value ExecuteBody(List<ASTNode?> body, StackFrame frame)
    {
        var result = Value.Default;
        foreach (var stmt in body)
        {
            if (stmt == null)
            {
                continue;
            }

            if (stmt.Type == ASTNodeType.Next || stmt.Type == ASTNodeType.Break)
            {
                continue;
            }

            result = Interpret(stmt);

            if (frame.IsFlagSet(FrameFlags.Return))
            {
                return frame.ReturnValue ?? result;
            }
        }

        return result;
    }

    private string PrintObject(Token token, Value value)
    {
        if (!value.IsObject())
        {
            return Serializer.Serialize(value);
        }

        var instance = value.GetObject();
        var method = GetObjectMethod(instance, CoreBuiltin.ToS);
        if (method == null)
        {
            return Serializer.Serialize(value);
        }

        var frame = CallStack.Peek();
        var oldContext = frame.GetObjectContext();
        var hadContext = frame.InObjectContext();

        frame.SetObjectContext(instance);

        try
        {
            var result = InvokeCallable(method, new List<Value>(), token, $"{instance.StructName}#{CoreBuiltin.ToS}", instance);
            return Serializer.Serialize(result);
        }
        finally
        {
            if (hadContext)
            {
                frame.SetObjectContext(oldContext);
            }
            else
            {
                frame.ClearFlag(FrameFlags.InObject);
            }
        }
    }

    private bool HasOperatorOverload(InstanceRef inst, TokenName op, out KFunction? func)
    {
        var struc = Context.Structs[inst.StructName];
        var opString = Serializer.GetOperatorString(op);
        func = null;

        if (string.IsNullOrEmpty(opString))
        {
            return false;
        }

        if (struc.Methods.TryGetValue(opString, out KFunction? structFunc))
        {
            func = structFunc;
            return true;
        }

        // check base struct
        if (!string.IsNullOrEmpty(struc.BaseStruct))
        {
            var baseStruct = Context.Structs[struc.BaseStruct];
            var baseStructMethods = baseStruct.Methods;

            if (baseStructMethods.TryGetValue(opString, out KFunction? baseStructFunc))
            {
                func = baseStructFunc;
                return true;
            }
        }

        return false;
    }

    /// <summary>
    /// Create a generator from pre-evaluated arguments. Used by the VM when a generator
    /// function is called via bytecode (arguments are already on the value stack).
    /// </summary>
    public Value CreateGeneratorFromValues(KFunction func, List<Value> args, Token token)
    {
        var generatorRef = new GeneratorRef();
        var capturedGlobal = _globalScope;
        var capturedContext = Context;
        var capturedExecPath = ExecutionPath;
        var capturedEntryPath = EntryPath;
        var capturedFunc = func;
        var capturedToken = token;

        generatorRef.Start(() =>
        {
            var genInterp = new Interpreter(capturedGlobal, capturedContext)
            {
                ExecutionPath = capturedExecPath,
                EntryPath = capturedEntryPath,
                GeneratorCancellationToken = generatorRef.CancellationToken
            };
            genInterp.RunGeneratorBody(capturedFunc, args, capturedToken, generatorRef);
        });

        return Value.CreateGenerator(generatorRef);
    }

    internal void RunGeneratorBody(KFunction func, List<Value> args, Token token, GeneratorRef generatorRef)
    {
        _activeGenerator = generatorRef;

        // For VM-compiled generators, run the bytecode body in a fresh per-thread VM.
        // Yield opcodes in the bytecode call _interp.YieldFromVM which uses _activeGenerator
        // (set above on this thread's interpreter instance).
        if (func.VMChunk != null)
        {
            var vm = new VM.KiwiVM(this, func.CapturedScope ?? _globalScope);
            vm.InvokeVMCallable(func, args, token, null);
            return;
        }

        // Tree-walk path for interpreter-defined generators.
        // Push a root frame so the call stack is not empty
        PushFrame("<generator>", token, new Scope(_globalScope));

        var functionScope = new Scope(func.CapturedScope ?? _globalScope);
        var functionFrame = new StackFrame(func.Name, functionScope, token);

        BindParameters(func, args, token, func.Name, functionScope);
        PushFrame(functionFrame);

        try
        {
            foreach (var stmt in func.Decl.Body)
            {
                if (stmt == null) continue;
                if (GeneratorCancellationToken.IsCancellationRequested)
                    break;
                Interpret(stmt);
                if (functionFrame.IsFlagSet(FrameFlags.Return))
                    break;
            }
        }
        finally
        {
            PopFrame(); // function frame
            PopFrame(); // root frame
        }
    }

    /// <summary>
    /// Public entry point used by the VM's Eval opcode.
    /// Lexes, parses, and interprets a Kiwi source string; returns the result.
    /// Ensures a call-stack frame is present (required if code defines functions).
    /// </summary>
    public Value EvalCode(Token token, string code)
    {
        using Lexer lexer = new(token.Span.File, code);
        var ast = new Parser(true).ParseTokenStream(lexer.GetTokenStream(), true);
        var chunk = VM.Compiler.CompileExpression((ProgramNode)ast);
        var savedVM = VM.KiwiVM.Current;
        try   { return new VM.KiwiVM(this).Execute(chunk); }
        finally { VM.KiwiVM.Current = savedVM; }
    }

    private KFunction? GetObjectMethod(InstanceRef obj, string name)
    {
        var struc = Context.Structs[obj.StructName];
        var strucMethods = struc.Methods;

        if (strucMethods.TryGetValue(name, out KFunction? func))
        {
            return func;
        }

        // check the base
        if (!string.IsNullOrEmpty(struc.BaseStruct))
        {
            var baseStruct = Context.Structs[struc.BaseStruct];
            var baseStructMethods = baseStruct.Methods;

            if (baseStructMethods.TryGetValue(name, out KFunction? baseStructFunc))
            {
                return baseStructFunc;
            }
        }

        return null;
    }

    private CallableType GetCallable(Token token, string name)
    {
        if (Context.HasFunction(name))
        {
            return CallableType.Function;
        }
        else if (Context.HasLambda(name))
        {
            return CallableType.Lambda;
        }
        else if (CoreBuiltin.IsBuiltinMethod(name))
        {
            return CallableType.Builtin;
        }

        if (Context.HasMappedLambda(name))
        {
            return CallableType.Lambda;
        }

        // Check scope chain for a lambda value bound to this name (e.g., lambda parameters).
        if (CallStack.Count > 0 && CallStack.Peek().Scope.TryGet(name, out Value scopeVal) && scopeVal.IsLambda())
        {
            return CallableType.Lambda;
        }

        var frame = CallStack.Peek();

        if (!frame.InObjectContext())
        {
            throw new FunctionUndefinedError(token, name);
        }

        var obj = frame.GetObjectContext() ?? throw new NullObjectError(token);
        var struc = Context.Structs[obj.StructName];
        var strucMethods = struc.Methods;

        if (strucMethods.ContainsKey(name))
        {
            return CallableType.Method;
        }

        // check the base
        if (!string.IsNullOrEmpty(struc.BaseStruct))
        {
            var baseStruct = Context.Structs[struc.BaseStruct];
            var baseStructMethods = baseStruct.Methods;

            if (baseStructMethods.ContainsKey(name))
            {
                return CallableType.Method;
            }
        }

        throw new UnimplementedMethodError(token, struc.Name, name);
    }

    /// <summary>
    /// Execute a C# native builtin call with pre-evaluated arguments.
    /// Called from the VM's CallBuiltin opcode handler.
    /// </summary>
    public Value ExecuteBuiltin(Token token, TokenName op, List<Value> args)
    {
        if (KiwiBuiltin.IsBuiltin(op))
        {
            return KiwiBuiltinHandler.Execute(token, op, args, ExecutionPath, EntryPath);
        }
        else if (ReflectorBuiltin.IsBuiltin(op))
        {
            return ReflectorBuiltinHandler.Execute(this, token, op, args, Context, CallStack, FuncStack);
        }
        else if (TaskBuiltin.IsBuiltin(op))
        {
            return TaskBuiltinHandler.Execute(token, op, args, Context);
        }
        else if (ChannelBuiltin.IsBuiltin(op))
        {
            return ChannelBuiltinHandler.Execute(token, op, args);
        }
        else if (SocketBuiltin.IsBuiltin(op))
        {
            return SocketBuiltinHandler.Execute(token, op, args);
        }
        else if (TlsSocketBuiltin.IsBuiltin(op))
        {
            return TlsSocketBuiltinHandler.Execute(token, op, args);
        }

        return BuiltinDispatch.Execute(token, op, args, CliArgs);
    }

    private Value CallPackageMethod(MethodCallNode node, PackageRef pkg)
    {
        var qualifiedName = pkg.Identifier + "::" + node.MethodName;

        if (Context.Functions.TryGetValue(qualifiedName, out KFunction? func))
        {
            // Arguments arrive as pre-evaluated LiteralNode wrappers (from DispatchMethod).
            var args = node.Arguments
                .Select(a => a is LiteralNode ln ? ln.Value : Value.Default)
                .ToList();

            if (func.IsGenerator)
            {
                return CreateGeneratorFromValues(func, args, node.Token);
            }

            // CallFunction(KFunction, ...) manages push/pop internally.
            return InvokeCallable(func, args, node.Token, qualifiedName);
        }

        throw new FunctionUndefinedError(node.Token, node.MethodName);
    }

    private void PrepareLambdaCall(KLambda func, List<Value> args, HashSet<string> defaultParameters, Token token, string targetLambda, Dictionary<string, List<int>> typeHints, string lambdaName, Scope scope)
    {
        var parms = func.Parameters;
        for (var i = 0; i < parms.Count; ++i)
        {
            var param = parms[i];
            var argValue = Value.Default;

            if (i < args.Count)
            {
                argValue = args[i];
            }
            else if (defaultParameters.Contains(param.Key))
            {
                argValue = param.Value;
            }
            else
            {
                throw new ParameterCountMismatchError(token, targetLambda, parms.Count, args.Count);
            }

            PrepareLambdaVariables(typeHints, param, ref argValue, token, i, lambdaName, scope);
        }

        // Collect overflow args into variadic list
        if (!string.IsNullOrEmpty(func.VariadicParamName))
        {
            var varargs = args.Skip(parms.Count).ToList();
            scope.Declare(func.VariadicParamName, Value.CreateList(varargs));
        }
    }

    private void PrepareLambdaVariables(Dictionary<string, List<int>> typeHints, KeyValuePair<string, Value> param, ref Value argValue, Token token, int i, string lambdaName, Scope scope)
    {
        if (typeHints.TryGetValue(param.Key, out List<int>? expectedTypes) && !AssertTypeMatch(token, argValue, expectedTypes))
        {
            throw new TypeError(token, $"Expected type `{string.Join("|", expectedTypes.Select(TypeRegistry.GetTypeName))}` for parameter {1 + i} of `{lambdaName}` but received `{TypeRegistry.GetTypeName(argValue)}`.");
        }

        scope.Declare(param.Key, argValue);
    }

    internal void RegisterTypeBuiltins(string packageName)
    {
        if (TypeRegistry.TryGetPrimitiveType(packageName, out int type))
        {
            var prefix = packageName + "::";
            var funcs = Context.Functions.Where(x => x.Key.StartsWith(prefix)).ToList();
            foreach (var func in funcs)
            {
                TypeBuiltins.Register(type, func.Key.Replace(prefix, string.Empty), func.Value);
            }
        }
    }

    public void ImportPackage(Token token, Value packageName)
    {
        if (!packageName.IsString())
            throw new InvalidOperationError(token, "Expected the name of a package to import.");

        var packageNameValue = packageName.GetString();

        if (Context.ImportedPackages.Contains(packageNameValue))
            return;

        if (!Context.HasPackage(packageNameValue))
        {
            // Check if external package.
            var packagePath = FileUtil.TryGetExtensionless(token, packageNameValue, ExecutionPath);
            if (FileUtil.IsScript(token, packagePath))
            {
                ImportExternal(token, packagePath);
                return;
            }

            throw new PackageUndefinedError(token, packageNameValue);
        }

        PackageStack.Push(packageNameValue);
        var decl = Context.Packages[packageNameValue].Decl;

        // Execute the package body via the VM so function definitions are compiled.
        // If the body is non-empty (interpreted/dynamic package), compile and run it.
        if (decl.Body.Count > 0)
        {
            // Wrap the PackageNode in a ProgramNode so the compiler can handle it.
            var prog = new ProgramNode([decl]);
            var chunk = VM.Compiler.CompileProgram(prog);
            var savedVM = VM.KiwiVM.Current;
            try { new VM.KiwiVM(this).Execute(chunk); }
            finally { VM.KiwiVM.Current = savedVM; }
        }

        if (PackageStack.Count > 0)
            PackageStack.Pop();

        RegisterTypeBuiltins(packageNameValue);
        Context.ImportedPackages.Add(packageNameValue);
    }

    private void ImportExternal(Token token, string packageName)
    {
        var packagePath = FileUtil.TryGetExtensionless(token, packageName);
        var content = FileUtil.ReadFile(token, packagePath);
        if (string.IsNullOrEmpty(content))
        {
            return;
        }

        var path = FileUtil.GetAbsolutePath(token, packagePath);

        if (Context.Includes.Contains(path))
        {
            return;
        }

        Context.Includes.Add(path);

        using Lexer lexer = new(path);
        var tokenStream = lexer.GetTokenStream();
        var ast = (ProgramNode)new Parser(true).ParseTokenStream(tokenStream, true);
        var chunk = VM.Compiler.CompileProgram(ast);
        var savedVM = VM.KiwiVM.Current;
        try { new VM.KiwiVM(this).Execute(chunk); }
        finally { VM.KiwiVM.Current = savedVM; }
    }
    private StackFrame PushFrame(StackFrame frame, bool inLambda = false) => _frames.Push(frame, inLambda);
    private StackFrame PushFrame(string name, Token token, Scope scope, bool inLambda = false) => _frames.Push(name, token, scope, inLambda);
    private Value PopFrame() => _frames.Pop();
    private bool PushFrame(StackFrame frame) => _frames.PushRaw(frame);
    private Scope EnterBlockScope(StackFrame frame) => _frames.EnterBlock(frame);
    private void ExitBlockScope(StackFrame frame, Scope parent) => _frames.ExitBlock(frame, parent);
    private Scope CaptureCurrentScope() => _frames.CaptureCurrentScope();
    private bool InTry() => _frames.InTry();
    private StackFrame CreateFrame(string name, Token token, Scope? parentScope = null) => _frames.CreateFrame(name, token, parentScope);

    private static string Id(ASTNode node) => ((IdentifierNode)node).Name;

    // ═══════════════════════════════════════════════════════════════════════════
    // VM Bridge - public surface used by KiwiVM to delegate work back to the
    // tree-walking interpreter for operations not yet compiled natively.
    // ═══════════════════════════════════════════════════════════════════════════

    /// <summary>
    /// Returns the interpreter's global scope (shared with the VM).
    /// </summary>
    public Scope GetGlobalScope() => _globalScope;

    /// <summary>
    /// Push a synthetic call-stack frame so builtins can safely call CallStack.Peek().
    /// Used by the VM's InterpFallback handler before delegating to the tree-walker.
    /// </summary>
    public void PushVMDispatchFrame(Scope globals, InstanceRef? self = null)
    {
        var frame = new StackFrame("<vm-dispatch>", globals);
        if (self != null) frame.SetObjectContext(self);
        CallStack.Push(frame);
    }

    /// <summary>Pop the synthetic frame pushed by <see cref="PushVMDispatchFrame"/>.</summary>
    public void PopVMDispatchFrame()
        => CallStack.Pop();

    /// <summary>
    /// Execute a single AST node and return its value (used by InterpFallback).
    /// </summary>

    /// <summary>
    /// Execute a single AST node with VM-local variables injected into a temporary
    /// interpreter scope so that expressions inside the node (e.g. argument sub-expressions
    /// in named-arg calls) can resolve VM locals by name.
    /// After execution, any variables that were modified in the interpreter scope are
    /// written back into <paramref name="locals"/> so the VM can update its stack.
    /// </summary>

    /// <summary>
    /// Dispatch a method call given a pre-evaluated receiver object and argument list.
    /// Used by the VM's CallMethod opcode.
    /// </summary>
    public Value DispatchMethod(Value obj, string methodName, List<Value> args, Token token)
    {
        // Build a synthetic MethodCallNode so existing dispatch logic can be reused.
        // The arguments are already evaluated, so we wrap each in a LiteralNode.
        var argNodes = args.Select(a => (ASTNode?)new LiteralNode(a) { Token = token }).ToList();
        var node = new MethodCallNode(null, methodName, TokenName.Ops_Assign, argNodes) { Token = token };

        // Pre-supply the already-evaluated object
        if (obj.IsPackage())  return CallPackageMethod(node, obj.GetPackage());
        if (obj.IsObject())   return CallObjectMethodDirect(token, methodName, obj.GetObject(), args);
        if (obj.IsStruct())   return CallStructMethodDirect(token, methodName, obj.GetStruct(), args);

        // Builtin method dispatch - resolve string name → TokenName
        if (ListBuiltin.Map.TryGetValue(methodName, out TokenName listOp))
        {
            // HandleListBuiltin peeks the call stack to detect early returns from lambdas.
            // Push a synthetic frame when the VM calls us with an empty stack.
            bool pushedFrame = CallStack.Count == 0;
            if (pushedFrame) CallStack.Push(new StackFrame("<vm-dispatch>", _globalScope));
            try   { return ListBuiltinHandler.HandleListBuiltin(this, token, ref obj, listOp, args); }
            finally { if (pushedFrame) CallStack.Pop(); }
        }
        if (CallableBuiltin.Map.TryGetValue(methodName, out _))
            return HandleCallableBuiltinDirect(token, obj, methodName, args);
        if (CoreBuiltin.Map.TryGetValue(methodName, out TokenName coreOp))
            return BuiltinDispatch.Execute(token, coreOp, obj, args);

        var typeId = TypeRegistry.GetType(token, obj);
        if (TypeBuiltins.TryGetBuiltin(typeId, methodName, out KFunction? typeBuiltin) && typeBuiltin != null)
        {
            var tArgs = new List<Value>(args.Count + 1) { obj };
            tArgs.AddRange(args);
            return InvokeCallable(typeBuiltin, tArgs, token, methodName);
        }

        throw new FunctionUndefinedError(token, methodName);
    }

    private static readonly HashSet<TokenName> _objectSpecificBuiltins =
    [
        TokenName.Builtin_Core_Clone,
        TokenName.Builtin_Core_Get,
        TokenName.Builtin_Core_HasKey,
        TokenName.Builtin_Core_IsA,
        TokenName.Builtin_Core_Keys,
        TokenName.Builtin_Core_Set,
        TokenName.Builtin_Core_Values,
    ];

    private Value CallObjectMethodDirect(Token token, string methodName, InstanceRef obj, List<Value> args)
    {
        if (!Context.HasStruct(obj.StructName))
            throw new StructUndefinedError(token, obj.StructName);

        var struc = Context.Structs[obj.StructName];
        // Try the struct method chain first; fall through to builtins if not found.
        if (!TryResolveStructMethod(struc, obj, methodName, out KFunction? fn))
        {
            if (CoreBuiltin.Map.TryGetValue(methodName, out TokenName coreOp))
            {
                // Object-specific builtins (is_a, clone, has_key, etc.) need struct context.
                if (_objectSpecificBuiltins.Contains(coreOp))
                    return ObjectBuiltinHandler.Handle(this, token, coreOp, obj, struc.BaseStruct, args);
                return BuiltinDispatch.Execute(token, coreOp, Value.CreateObject(obj), args);
            }
            throw new UnimplementedMethodError(token, obj.StructName, methodName);
        }

        bool isCtor = methodName == "new";
        if (isCtor)
        {
            var inst = new InstanceRef { StructName = obj.StructName, Identifier = obj.StructName };
            return InvokeCallable(fn!, args, token, methodName, inst);
        }
        return InvokeCallable(fn!, args, token, methodName, obj);
    }

    private bool TryResolveStructMethod(KStruct struc, InstanceRef obj, string methodName, out KFunction? fn)
    {
        if (struc.Methods.TryGetValue(methodName, out fn))
            return true;
        if (!string.IsNullOrEmpty(struc.BaseStruct) && Context.HasStruct(struc.BaseStruct))
            return TryResolveStructMethod(Context.Structs[struc.BaseStruct], obj, methodName, out fn);
        fn = null;
        return false;
    }

    private Value ResolveAndCallMethod(Token token, KStruct struc, InstanceRef obj, string methodName, List<Value> args)
    {
        if (!struc.Methods.TryGetValue(methodName, out KFunction? fn))
        {
            if (!string.IsNullOrEmpty(struc.BaseStruct) && Context.HasStruct(struc.BaseStruct))
                return ResolveAndCallMethod(token, Context.Structs[struc.BaseStruct], obj, methodName, args);
            throw new UnimplementedMethodError(token, obj.StructName, methodName);
        }

        bool isCtor = methodName == "new";
        if (isCtor)
        {
            var inst = new InstanceRef { StructName = obj.StructName, Identifier = obj.StructName };
            return InvokeCallable(fn, args, token, methodName, inst);
        }
        return InvokeCallable(fn, args, token, methodName, obj);
    }

    private Value CallStructMethodDirect(Token token, string methodName, StructRef sref, List<Value> args)
    {
        var structName = sref.Identifier;
        if (!Context.HasStruct(structName))
            throw new StructUndefinedError(token, structName);
        var struc = Context.Structs[structName];

        // Abstract structs cannot be instantiated directly.
        if (methodName == "new" && struc.IsAbstract)
            throw new AbstractInstantiationError(token, structName);

        // Walk the inheritance chain to find the method (handles inherited fn new, etc.).
        KFunction? fn = null;
        var search = struc;
        while (search != null)
        {
            if (search.Methods.TryGetValue(methodName, out fn)) break;
            fn = null;
            search = !string.IsNullOrEmpty(search.BaseStruct) && Context.HasStruct(search.BaseStruct)
                ? Context.Structs[search.BaseStruct]
                : null;
        }

        if (fn == null)
        {
            // Default constructor: no fn new() found anywhere in hierarchy → empty instance.
            if (methodName == "new")
            {
                var inst = new InstanceRef { StructName = structName, Identifier = structName };
                return Value.CreateObject(inst);
            }
            throw new UnimplementedMethodError(token, structName, methodName);
        }

        // Constructor: use the concrete type for StructName so the instance has the right type.
        if (methodName == "new")
        {
            var inst = new InstanceRef { StructName = structName, Identifier = structName };
            InvokeCallable(fn, args, token, methodName, inst);
            return Value.CreateObject(inst);
        }
        // Static method: pass a struct-name carrier so the VM frame can resolve @@ variables.
        var structSelf = new InstanceRef { StructName = structName, Identifier = structName };
        return InvokeCallable(fn, args, token, methodName, structSelf);
    }

    private Value HandleCallableBuiltinDirect(Token token, Value obj, string methodName, List<Value> args)
    {
        if (!CallableBuiltin.Map.TryGetValue(methodName, out var op))
            throw new FunctionUndefinedError(token, methodName);

        Callable? callable = null;
        string? callableName = null;

        if (obj.IsLambda())
        {
            callableName = obj.GetLambda().Identifier;
            if (Context.HasLambda(callableName)) callable = Context.Lambdas[callableName];
            else if (Context.HasMappedLambda(callableName)) callable = Context.Lambdas[Context.LambdaTable[callableName]];
            else if (Context.HasFunction(callableName)) callable = Context.Functions[callableName];
        }

        if (callableName == null)
            throw new InvalidOperationError(token, $"Expected a callable for function `{CallableBuiltin.MapName(op)}`.");
        if (callable == null)
            throw new InvalidOperationError(token, $"Expected a callable for function `{CallableBuiltin.MapName(op)}` on `{callableName}`.");
        if (callable is not KFunction && callable is not KLambda)
            throw new InvalidOperationError(token, $"Expected a function or lambda for function `{CallableBuiltin.MapName(op)}` on `{callableName}`.");

        return CallableBuiltinHandler.Execute(this, token, op, callable, callableName, args);
    }

    /// <summary>
    /// Get the value at obj[key].  Used by the VM's IndexGet opcode.
    /// </summary>
    public Value GetIndex(Value obj, Value key, Token token)
    {
        if (obj.IsList())
        {
            var idx  = Builtin.Operation.ConversionOp.GetInteger(token, key);
            var list = obj.GetList();
            if (idx < 0 || idx >= list.Count)
                throw new IndexError(token, "List index out of bounds.");
            return list[(int)idx];
        }
        if (obj.IsHashmap())
        {
            obj.GetHashmap().TryGetValue(key, out var v);
            return v ?? Value.CreateNull();
        }
        if (obj.IsString())
        {
            var s   = obj.GetString();
            var idx = Builtin.Operation.ConversionOp.GetInteger(token, key);
            if (idx < 0 || idx >= s.Length)
                throw new IndexError(token, "String index out of bounds.");
            return Value.CreateString(s[(int)idx].ToString());
        }
        if (obj.IsBytes())
        {
            var bytes = obj.GetBytes();
            var idx   = Builtin.Operation.ConversionOp.GetInteger(token, key);
            if (idx < 0 || idx >= bytes.Length)
                throw new IndexError(token, "Bytes index out of bounds.");
            return Value.CreateInteger(bytes[(int)idx]);
        }
        throw new InvalidOperationError(token, "Cannot index this type.");
    }

    /// <summary>
    /// Set obj[key] = value.  Used by the VM's IndexSet opcode.
    /// </summary>
    public void SetIndex(Value obj, Value key, Value value, Token token)
    {
        if (obj.IsList())
        {
            var idx  = Builtin.Operation.ConversionOp.GetInteger(token, key);
            var list = obj.GetList();
            if (idx < 0 || idx >= list.Count)
                throw new IndexError(token, "List index out of bounds.");
            list[(int)idx] = value;
            return;
        }
        if (obj.IsHashmap())
        {
            obj.GetHashmap()[key] = value;
            return;
        }
        throw new InvalidOperationError(token, "Cannot index-assign this type.");
    }

    /// <summary>
    /// Perform a slice on obj.  Used by the VM's SliceGet opcode.
    /// </summary>
    public Value GetSlice(Value obj, Value? startV, Value? stopV, Value? stepV, Token token)
    {
        var startVal = startV ?? Value.CreateInteger(0L);
        var stopVal  = stopV  ?? (obj.IsList()   ? Value.CreateInteger(obj.GetList().Count) :
                                  obj.IsString() ? Value.CreateInteger(obj.GetString().Length) :
                                  obj.IsBytes()  ? Value.CreateInteger(obj.GetBytes().Length) :
                                                   Value.CreateInteger(0L));
        var stepVal  = stepV  ?? Value.CreateInteger(1L);
        var slice    = new SliceIndex(startVal, stopVal, stepVal) { IsSlice = true };

        if (obj.IsString()) return Builtin.Util.SliceUtil.StringSlice(token, slice, obj.GetString());
        if (obj.IsList())   return Builtin.Util.SliceUtil.ListSlice(token, slice, obj.GetList());
        if (obj.IsBytes())  return Builtin.Util.SliceUtil.BytesSlice(token, slice, obj.GetBytes());
        throw new InvalidOperationError(token, "Cannot slice this type.");
    }

    /// <summary>
    /// Assign rhs into a slice range of obj in-place.
    /// Called from the VM's SliceSet opcode handler.
    /// </summary>
    public void SetSlice(Value obj, Value? startV, Value? stopV, Value? stepV, Value rhs, Token token)
    {
        var startVal = startV ?? Value.CreateInteger(0L);
        var stopVal  = stopV  ?? (obj.IsList()  ? Value.CreateInteger(obj.GetList().Count) :
                                  obj.IsBytes() ? Value.CreateInteger(obj.GetBytes().Length) :
                                                  Value.CreateInteger(0L));
        var stepVal  = stepV  ?? Value.CreateInteger(1L);
        var slice    = new SliceIndex(startVal, stopVal, stepVal) { IsSlice = true };

        if (obj.IsList() && rhs.IsList())
        {
            var targetList = obj.GetList();
            var rhsValues  = rhs.GetList();
            Builtin.Util.SliceUtil.UpdateListSlice(token, false, ref targetList, slice, rhsValues);
        }
    }

    /// <summary>
    /// Get obj.memberName (package/struct instance variable).
    /// Used by the VM's GetMember opcode.
    /// </summary>
    public Value GetMember(Value obj, string memberName, Token token)
    {
        if (obj.IsObject())
        {
            var inst = obj.GetObject();
            // Instance variables are stored with "@" prefix (e.g. "@tag"); try that first,
            // then fall back to the bare name for dynamically-set fields.
            if (!inst.InstanceVariables.TryGetValue("@" + memberName, out var v))
                inst.InstanceVariables.TryGetValue(memberName, out v);
            return v ?? Value.Default;
        }
        if (obj.IsPackage())
        {
            var pkgName = obj.GetPackage().Identifier;
            var qualifiedName = pkgName + "::" + memberName;
            if (Context.Constants.TryGetValue(qualifiedName, out Value? constVal))
                return constVal;
            if (Context.PackageVariables.TryGetValue(qualifiedName, out Value? pkgVar))
                return pkgVar;
            if (Context.HasFunction(qualifiedName))
                return Value.CreateLambda(new LambdaRef { Identifier = qualifiedName });
            return Value.Default;
        }
        if (obj.IsHashmap())
        {
            obj.GetHashmap().TryGetValue(Value.CreateString(memberName), out var v);
            return v ?? Value.Default;
        }
        if (obj.IsStruct())
        {
            var kstruct = Context.Structs[obj.GetStruct().Identifier];
            if (kstruct.StaticVariables.TryGetValue("@@" + memberName, out var sv))
                return sv;
            return Value.Default;
        }
        throw new InvalidOperationError(token, $"Cannot access member '{memberName}' on this type.");
    }

    /// <summary>
    /// Set obj.memberName = value.
    /// Used by the VM's SetMember opcode.
    /// </summary>
    public void SetMember(Value obj, string memberName, Value value, Token token)
    {
        if (obj.IsObject())
        {
            // Instance variables use "@" prefix; write to "@name" if that key exists,
            // otherwise use the bare name (for dynamically-added fields).
            var iv = obj.GetObject().InstanceVariables;
            var key = iv.ContainsKey("@" + memberName) ? "@" + memberName : memberName;
            iv[key] = value;
            return;
        }
        if (obj.IsHashmap())
        {
            obj.GetHashmap()[Value.CreateString(memberName)] = value;
            return;
        }
        if (obj.IsStruct())
        {
            var sName = obj.GetStruct().Identifier;
            if (Context.HasStruct(sName))
                Context.Structs[sName].StaticVariables["@@" + memberName] = value;
            return;
        }
        if (obj.IsPackage())
        {
            // Package members (constants/functions) are not directly settable via member assignment;
            // fall through to instance variable assignment if obj is also an object, otherwise no-op.
            return;
        }
        throw new InvalidOperationError(token, $"Cannot set member '{memberName}' on this type.");
    }

    /// <summary>
    /// Create a new struct instance with the given args.
    /// Used by the VM's NewObject opcode.
    /// </summary>
    public Value CreateObject(string structName, List<Value> args, Token token)
    {
        if (!Context.HasStruct(structName))
            throw new StructUndefinedError(token, structName);
        var struc = Context.Structs[structName];
        var inst  = new InstanceRef { StructName = structName, Identifier = structName };
        if (struc.Methods.TryGetValue("new", out KFunction? ctor))
            InvokeCallable(ctor, args, token, "new", inst);
        return Value.CreateObject(inst);
    }

    /// <summary>
    /// Serialize a value to a string (for interpolation / print).
    /// </summary>
    public string Serialize(Value v) => Serializer.Serialize(v);

    /// <summary>
    /// Yield a value from inside a VM-executed generator body.
    /// Delegates to the active GeneratorRef.
    /// </summary>
    public void YieldFromVM(Value v)
    {
        if (_activeGenerator == null)
            throw new RuntimeError(Token.Eof, "'yield' used outside of a generator.", []);
        _activeGenerator.Yield(v);
    }

    /// <summary>
    /// Returns the value itself for primitive/immutable types (int, float, bool, string, null)
    /// where sharing the Value wrapper is safe. Clones only mutable collections.
    /// </summary>
    [System.Runtime.CompilerServices.MethodImpl(System.Runtime.CompilerServices.MethodImplOptions.AggressiveInlining)]
    private static Value CloneIfCollection(Value v) =>
        (v.IsList() || v.IsHashmap()) ? v.Clone() : v;
}