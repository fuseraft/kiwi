using kiwi.Runtime.Builtin.Operation;
using kiwi.Parsing;
using kiwi.Parsing.AST;
using kiwi.Typing;
using kiwi.Tracing;
using kiwi.Tracing.Error;
using kiwi.Parsing.Keyword;
using kiwi.Runtime.Builtin.Handler;
using kiwi.Settings;
using kiwi.Runtime.Builtin.Dispatcher;
using kiwi.Runtime.Builtin.Util;

namespace kiwi.Runtime;

public class Interpreter
{
    private const int SafemodeMaxIterations = 1000000;
    private Scope _globalScope = new();
    private readonly Stack<Scope> _scopePool = new(32);
    private FrameManager _frames = null!;
    private BuiltinMethodDispatcher _builtinDispatcher = null!;

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
        _builtinDispatcher = new BuiltinMethodDispatcher(this);
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
    public Action<ASTNode>? DebugHook { get; set; }

    public Value EntryPoint(ASTNode? node)
    {
        if (node == null) return Value.Default;

        var rootToken = node?.Token ?? Token.Eof;
        PushFrame("<entry>", rootToken, new Scope(_globalScope), inLambda: false);
        
        try
        {
            return Interpret(node);
        }
        catch (KiwiError e)
        {
            if (e is not RuntimeError)
            {
                throw new RuntimeError(e.Token, e.Message ?? "Unknown error", CaptureStackTrace());
            }
            throw;
        }
        finally
        {
            PopFrame();
        }
    }

    private List<string> CaptureStackTrace() => _frames.CaptureStackTrace();

    public Value Interpret(ASTNode? node)
    {
        if (node == null)
        {
            return Value.Default;
        }

        DebugHook?.Invoke(node);

        var result = node.Type switch
        {
            ASTNodeType.Assignment => Visit((AssignmentNode)node),
            ASTNodeType.BinaryOperation => Visit((BinaryOperationNode)node),
            ASTNodeType.Break => Visit((BreakNode)node),
            ASTNodeType.Case => Visit((CaseNode)node),
            ASTNodeType.CaseWhen => Visit((CaseWhenNode)node),
            ASTNodeType.ConstAssignment => Visit((ConstAssignmentNode)node),
            ASTNodeType.Do => Visit((DoNode)node),
            ASTNodeType.Emit => Visit((EmitNode)node),
            ASTNodeType.Exit => Visit((ExitNode)node),
            ASTNodeType.Export => Visit((ExportNode)node),
            ASTNodeType.ForLoop => Visit((ForLoopNode)node),
            ASTNodeType.Function => Visit((FunctionNode)node),
            ASTNodeType.FunctionCall => Visit((FunctionCallNode)node),
            ASTNodeType.HashLiteral => Visit((HashLiteralNode)node),
            ASTNodeType.Identifier => Visit((IdentifierNode)node),
            ASTNodeType.If => Visit((IfNode)node),
            ASTNodeType.DecoratedFunction => Visit((DecoratedFunctionNode)node),
            ASTNodeType.Import => Visit((ImportNode)node),
            ASTNodeType.Interpolation => Visit((InterpolationNode)node),
            ASTNodeType.Include => Visit((IncludeNode)node),
            ASTNodeType.Index => Visit((IndexingNode)node),
            ASTNodeType.IndexAssignment => Visit((IndexAssignmentNode)node),
            ASTNodeType.Lambda => Visit((LambdaNode)node),
            ASTNodeType.LambdaCall => Visit((LambdaCallNode)node),
            ASTNodeType.ListLiteral => Visit((ListLiteralNode)node),
            ASTNodeType.Literal => Visit((LiteralNode)node),
            ASTNodeType.MemberAccess => Visit((MemberAccessNode)node),
            ASTNodeType.MemberAssignment => Visit((MemberAssignmentNode)node),
            ASTNodeType.MethodCall => Visit((MethodCallNode)node),
            ASTNodeType.Next => Visit((NextNode)node),
            ASTNodeType.NoOp => Value.Default,
            ASTNodeType.Off => Visit((OffNode)node),
            ASTNodeType.On => Visit((OnNode)node),
            ASTNodeType.Once => Visit((OnceNode)node),
            ASTNodeType.Package => Visit((PackageNode)node),
            ASTNodeType.PackAssignment => Visit((PackAssignmentNode)node),
            ASTNodeType.Eval => Visit((EvalNode)node),
            ASTNodeType.Print => Visit((PrintNode)node),
            ASTNodeType.PrintXy => Visit((PrintXyNode)node),
            ASTNodeType.Program => Visit((ProgramNode)node),
            ASTNodeType.RangeLiteral => Visit((RangeLiteralNode)node),
            ASTNodeType.RepeatLoop => Visit((RepeatLoopNode)node),
            ASTNodeType.Require => Visit((RequireNode)node),
            ASTNodeType.Return => Visit((ReturnNode)node),
            ASTNodeType.Self => Visit((SelfNode)node),
            ASTNodeType.StaticSelf => Visit((StaticSelfNode)node),
            ASTNodeType.Slice => Visit((SliceNode)node),
            ASTNodeType.Enum => Visit((EnumNode)node),
            ASTNodeType.Struct => Visit((StructNode)node),
            ASTNodeType.TernaryOperation => Visit((TernaryOperationNode)node),
            ASTNodeType.Throw => Visit((ThrowNode)node),
            ASTNodeType.Try => Visit((TryNode)node),
            ASTNodeType.UnaryOperation => Visit((UnaryOperationNode)node),
            ASTNodeType.Variable => Visit((VariableNode)node),
            ASTNodeType.WhileLoop => Visit((WhileLoopNode)node),
            ASTNodeType.Yield => Visit((YieldNode)node),
            _ => PrintNode(node),
        };

        return result;
    }

    public Value FuncToLambda(KFunction func)
    {
        var lambdaValue = Visit(func.Decl.ToLambda(), false);

        // If the function was compiled to bytecode (VM mode), transfer the chunk
        // so the lambda executes via the VM rather than the (empty) AST body.
        if (func.VMChunk != null && lambdaValue.IsLambda())
        {
            var lref = lambdaValue.GetLambda();
            if (Context.Lambdas.TryGetValue(lref.Identifier, out var kl))
            {
                kl.VMChunk   = func.VMChunk;
                kl.VMUpvalues = func.VMUpvalues;
            }
        }

        return lambdaValue;
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

    private Value Visit(OnNode node)
    {
        var eventName = Interpret(node.EventName);
        var callback = Interpret(node.Callback);

        Context.Events.On(eventName.GetString(), callback, priority: node.Priority);

        return Value.Default;
    }

    private Value Visit(OnceNode node)
    {
        var eventName = Interpret(node.EventName);
        var callback = Interpret(node.Callback);

        Context.Events.Once(eventName.GetString(), callback, priority: node.Priority);

        return Value.Default;
    }

    private Value Visit(EmitNode node)
    {
        var eventName = Interpret(node.EventName);
        var args = node.EventArgs.Select(Interpret).ToList();

        var results = Context.Events.Emit(this, node.Token, eventName.GetString(), args);

        return Value.CreateList(results);
    }

    private Value Visit(OffNode node)
    {
        var eventName = Interpret(node.EventName);
        Value? callback = null;

        if (node.Callback != null)
        {
            callback = Interpret(node.Callback!);
        }

        Context.Events.Off(eventName.GetString(), callback);
        return Value.Default;
    }

    private bool Initialized = false;

    /// <summary>
    /// The entry point of the program.
    /// </summary>
    /// <param name="node">The root node.</param>
    /// <returns></returns>
    private Value Visit(ProgramNode node)
    {
        // This is the program root
        if (node.IsEntryPoint && !Initialized)
        {
            PushFrame("<kiwi>", node.Token, _globalScope);
            Initialized = true;
        }

        var result = Value.Default;

        foreach (var stmt in node.Statements)
        {
            result = Interpret(stmt);
        }

        return result;
    }

    private Value Visit(SelfNode node)
    {
        var frame = CallStack.Peek();

        if (!frame.InObjectContext())
        {
            throw new InvalidContextError(node.Token);
        }

        var obj = frame.GetObjectContext() ?? throw new NullObjectError(node.Token);

        if (!string.IsNullOrEmpty(node.Name))
        {
            var name = node.Name;

            if (!obj.HasVariable(name))
            {
                obj.InstanceVariables[name] = Value.Default;
            }

            return obj.InstanceVariables[name];
        }

        return Value.CreateObject(obj);
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

    private Value Visit(StaticSelfNode node)
    {
        var structName = GetCurrentStructName(node.Token);
        var kstruct = Context.Structs[structName];
        var varName = node.Name;

        if (!kstruct.StaticVariables.ContainsKey(varName))
        {
            kstruct.StaticVariables[varName] = Value.Default;
        }

        return kstruct.StaticVariables[varName];
    }

    private Value Visit(EnumNode node)
    {
        KStruct kenum = new()
        {
            Name = node.Name,
            IsEnum = true
        };

        long counter = 0;
        foreach (var (memberName, valueExpr) in node.Members)
        {
            Value memberValue;
            if (valueExpr != null)
            {
                memberValue = Interpret(valueExpr);
                if (memberValue.IsInteger())
                {
                    counter = memberValue.GetInteger();
                }
            }
            else
            {
                memberValue = Value.CreateInteger(counter);
            }

            kenum.StaticVariables["@@" + memberName] = memberValue;
            counter++;
        }

        Context.Structs[node.Name] = kenum;
        return Value.Default;
    }

    private Value Visit(StructNode node)
    {
        var structName = node.Name;
        KStruct struc = new()
        {
            Name = structName,
            IsAbstract = node.IsAbstract
        };

        if (!string.IsNullOrEmpty(node.BaseStruct))
        {
            struc.BaseStruct = node.BaseStruct;

            if (!Context.HasStruct(struc.BaseStruct))
            {
                throw new StructUndefinedError(node.Token, struc.BaseStruct);
            }
        }

        StructStack.Push(structName);

        try
        {
            foreach (var method in node.Methods)
            {
                if (method == null)
                {
                    continue;
                }

                var funcDecl = (FunctionNode)method;
                var methodName = funcDecl.Name;

                Visit(funcDecl, inStruct: true);

                if (methodName == "new")
                {
                    struc.Methods["new"] = Context.Methods[methodName];
                }
                else
                {
                    struc.Methods[methodName] = Context.Methods[methodName];
                }

                if (funcDecl.IsAbstract)
                {
                    struc.Methods[methodName].IsAbstract = true;
                    struc.AbstractMethods.Add(methodName);
                }
            }

            // If derived from an abstract base, verify all abstract methods are implemented (only for concrete structs)
            if (!node.IsAbstract && !string.IsNullOrEmpty(node.BaseStruct))
            {
                var baseKStruct = Context.Structs[node.BaseStruct];
                foreach (var abstractMethod in baseKStruct.AbstractMethods)
                {
                    if (!struc.Methods.ContainsKey(abstractMethod))
                    {
                        throw new AbstractMethodError(node.Token, structName, abstractMethod);
                    }
                }
            }

            Context.Structs[structName] = struc;

            // Initialize static variables
            foreach (var (varName, initializer) in node.StaticVars)
            {
                struc.StaticVariables[varName] = initializer != null ? Interpret(initializer) : Value.Default;
            }
        }
        finally
        {
            StructStack.Pop();
            Context.Methods.Clear();
        }

        return Value.Default;
    }

    private Value Visit(PackageNode node)
    {
        var localName = Id(node.PackageName ?? throw new PackageUndefinedError(node.Token, string.Empty));

        // Build the fully-qualified package name from the current package context.
        // PackageStack holds unqualified names (outermost at bottom, innermost at top).
        // Reversing gives outer-to-inner order so we can join with "::".
        var packageName = PackageStack.Count > 0
            ? string.Join("::", PackageStack.Reverse().Append(localName))
            : localName;

        if (Context.Packages.TryGetValue(packageName, out var existing))
        {
            foreach (var stmt in node.Body)
            {
                existing.Decl.Body.Add(stmt?.Clone());
            }
        }
        else
        {
            Context.Packages[packageName] = new KPackage(node.Clone());
        }

        PackageStack.Push(localName);
        bool activationSucceeded = true;
        try
        {
            foreach (var stmt in node.Body)
            {
                Interpret(stmt);
            }
        }
        catch (KiwiError)
        {
            activationSucceeded = false;
        }
        finally
        {
            if (PackageStack.Count > 0)
                PackageStack.Pop();
        }

        if (activationSucceeded)
        {
            RegisterTypeBuiltins(packageName);
            Context.ImportedPackages.Add(packageName);
        }

        return Value.Default;
    }

    private Value Visit(ExportNode node)
    {
        var packageName = Interpret(node.PackageName);
        ImportPackage(node.Token, packageName);

        return Value.Default;
    }

    private Value Visit(ImportNode node)
    {
        var packageName = Interpret(node.PackageName);
        ImportPackage(node.Token, packageName);

        if (packageName.IsString())
        {
            return Value.CreatePackage(packageName.GetString());
        }

        return Value.Default;
    }

    private Value Visit(RequireNode node)
    {
        var packageName = node.PackageName;

        if (Context.ImportedPackages.Contains(packageName))
            return Value.Default;

        if (Context.HasPackage(packageName))
        {
            ImportPackage(node.Token, Value.CreateString(packageName));
            return Value.Default;
        }

        var packagePath = FileUtil.TryGetExtensionless(node.Token, packageName, ExecutionPath);
        if (FileUtil.IsScript(node.Token, packagePath))
        {
            ImportExternal(node.Token, packagePath);
        }

        return Value.Default;
    }

    private Value Visit(IncludeNode node)
    {
        var pathValue = Interpret(node.Path);
        if (!pathValue.IsString())
        {
            throw new InvalidOperationError(node.Token, "Include path must be a string.");
        }

        var filePath = pathValue.GetString();
        var fullPath = ResolveIncludePath(node.Token, filePath);

        if (Context.Includes.Contains(fullPath))
        {
            return Value.Default;
        }

        Context.Includes.Add(fullPath);

        if (!FileUtil.FileExists(node.Token, fullPath))
        {
            throw new InvalidOperationError(node.Token, $"File not found: {filePath}");
        }

        var oldExecutionPath = ExecutionPath;
        ExecutionPath = fullPath;

        using var lexer = new Lexer(fullPath);
        var ast = new Parser(true).ParseTokenStream(lexer.GetTokenStream(), true);
        Interpret(ast);

        ExecutionPath = oldExecutionPath;

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

    /// <summary>
    /// Handle the exit node.
    /// </summary>
    /// <param name="node">The node.</param>
    private Value Visit(ExitNode node)
    {
        if (node.Condition == null || BooleanOp.IsTruthy(Interpret(node.Condition)))
        {
            var exitValue = Interpret(node.ExitValue);
            long exitCode = exitValue.IsInteger() ? exitValue.GetInteger() : 1L;
            Environment.Exit(Convert.ToInt32(exitCode));
        }

        return Value.Default;
    }

    private Value Visit(ThrowNode node)
    {
        string DefaultErrorType = "KiwiError";

        if (node.Condition == null || BooleanOp.IsTruthy(Interpret(node.Condition)))
        {
            string errorType = DefaultErrorType;
            string errorMessage = string.Empty;

            if (node.ErrorValue != null)
            {
                var errorValue = Interpret(node.ErrorValue);

                if (errorValue.IsHashmap())
                {
                    var errorHash = errorValue.GetHashmap();
                    var errorKey = Value.CreateString("error");
                    var messageKey = Value.CreateString("message");

                    if (errorHash.TryGetValue(errorKey, out Value? errorTypeValue) && errorTypeValue.IsString())
                    {
                        errorType = errorTypeValue.GetString();
                    }
                    if (errorHash.TryGetValue(messageKey, out Value? messageValue) && messageValue.IsString())
                    {
                        errorMessage = messageValue.GetString();
                    }
                }
                else if (errorValue.IsString())
                {
                    errorMessage = errorValue.GetString();
                }
            }

            throw new RuntimeError(node.Token, errorType, errorMessage, CaptureStackTrace());
        }

        return Value.Default;
    }

    private Value Visit(AssignmentNode node)
    {
        string Global = "global";

        var frame = CallStack.Peek();
        var scope = frame.Scope;
        var type = node.Op;
        var name = node.Name;

        // Evaluate the when-guard condition BEFORE the RHS initializer so that
        // `x += expensive() when cond` only calls expensive() when cond is true.
        // `x = a when cond else b` assigns b when condition is falsy.
        if (node.Condition != null)
        {
            var eval = Interpret(node.Condition);

            if (!BooleanOp.IsTruthy(eval))
            {
                if (node.ElseInitializer == null)
                {
                    return Value.Default;
                }

                // Condition is false and an else-initializer exists: assign it instead.
                node = new AssignmentNode(node.Left, node.Name, node.Op, node.ElseInitializer)
                {
                    Token = node.Token
                };
            }
        }

        var value = CloneIfCollection(Interpret(node.Initializer));

        // Static variable assignment: @@varname = expr or @@varname += expr etc.
        if (node.Left?.Type == ASTNodeType.StaticSelf)
        {
            var varName = name;
            var structName = GetCurrentStructName(node.Token);
            var kstruct = Context.Structs[structName];

            if (type == TokenName.Ops_Assign)
            {
                kstruct.StaticVariables[varName] = value;
            }
            else
            {
                if (!kstruct.StaticVariables.TryGetValue(varName, out var oldVal))
                {
                    kstruct.StaticVariables[varName] = Value.Default;
                    oldVal = Value.Default;
                }

                kstruct.StaticVariables[varName] = type == TokenName.Ops_BitwiseNotAssign
                    ? BitwiseOp.Not(node.Token, ref oldVal)
                    : OpDispatch.DoBinary(node.Token, type, ref oldVal, ref value);
            }

            return kstruct.StaticVariables[varName];
        }

        if (type == TokenName.Ops_Assign)
        {
            if (Global.Equals(name) || Context.HasConstant(name))
            {
                throw new IllegalNameError(node.Token, name);
            }

            if (value.IsLambda())
            {
                if (frame.InObjectContext() && (node.Left?.Type == ASTNodeType.Self || name[0] == '@'))
                {
                    var obj = frame.GetObjectContext();
                    if (obj != null)
                    {
                        obj.InstanceVariables[name] = value;
                        return obj.InstanceVariables[name];
                    }
                }

                var lambdaId = value.GetLambda().Identifier;
                Context.Lambdas[name] = Context.Lambdas[lambdaId];

                var enclosingForLambda = CallStack.FirstOrDefault(f => f.IsFunction);
                enclosingForLambda?.LocalLambdas.Add(name);

                return value;
            }
            else
            {
                if (frame.InObjectContext() && (node.Left?.Type == ASTNodeType.Self || name[0] == '@'))
                {
                    var obj = frame.GetObjectContext();
                    if (obj != null)
                    {
                        obj.InstanceVariables[name] = value;
                        return obj.InstanceVariables[name];
                    }
                }

                if (value.IsObject())
                {
                    var obj = value.GetObject();
                    obj.Identifier = name;
                }

                scope.Assign(name, value);
            }
        }
        else
        {
            if (Context.HasConstant(name))
            {
                throw new IllegalNameError(node.Token, name);
            }

            if (scope.TryGet(name, out Value oldValue))
            {
                if (type == TokenName.Ops_BitwiseNotAssign)
                {
                    scope.Assign(name, BitwiseOp.Not(node.Token, ref oldValue));
                }
                else
                {
                    scope.Assign(name, OpDispatch.DoBinary(node.Token, type, ref oldValue, ref value));
                }

                return scope.GetBinding(name);
            }
            else if (frame.InObjectContext())
            {
                var obj = frame.GetObjectContext();

                if (obj == null)
                {
                    return Value.Default;
                }

                if (!obj.HasVariable(name))
                {
                    throw new VariableUndefinedError(node.Token, name);
                }

                var varVal = obj.InstanceVariables[name];

                if (type == TokenName.Ops_BitwiseNotAssign)
                {
                    obj.InstanceVariables[name] = BitwiseOp.Not(node.Token, ref varVal);
                }
                else
                {
                    obj.InstanceVariables[name] = OpDispatch.DoBinary(node.Token, type, ref varVal, ref value);
                }

                return obj.InstanceVariables[name];
            }

            throw new VariableUndefinedError(node.Token, name);
        }

        var result = scope.GetBinding(name);

        // If we're at package top-level (not inside a function body), mirror the variable
        // into Context.PackageVariables so it's reachable as package::varname.
        if (PackageStack.Count > 0 && !frame.IsFunction && !frame.InObjectContext())
        {
            Stack<string> tmpStack = new([.. PackageStack]);
            var prefix = string.Empty;
            while (tmpStack.Count > 0) { prefix += tmpStack.Peek() + "::"; tmpStack.Pop(); }
            Context.PackageVariables[prefix + name] = result;
        }

        return result;
    }

    private Value Visit(ConstAssignmentNode node)
    {
        var name = node.Name;
        var value = Interpret(node.Initializer);

        if (PackageStack.Count > 0)
        {
            Stack<string> tmpStack = new([.. PackageStack]);
            var prefix = string.Empty;
            while (tmpStack.Count > 0)
            {
                prefix += tmpStack.Peek() + "::";
                tmpStack.Pop();
            }
            name = prefix + name;
        }

        if (Context.HasConstant(name))
        {
            throw new IllegalNameError(node.Token, name);
        }

        Context.Constants.Add(name, value);

        return Value.Default;
    }

    private Value Visit(IndexAssignmentNode node)
    {
        var frame = CallStack.Peek();
        var scope = frame.Scope;
        var op = node.Op;
        var newValue = Interpret(node.Initializer);

        if (node.Object != null && node.Object.Type == ASTNodeType.Slice)
        {
            var sliceExpr = (SliceNode)node.Object;

            if (sliceExpr.SlicedObject != null && sliceExpr.SlicedObject.Type == ASTNodeType.Identifier)
            {
                var identifierName = Id(sliceExpr.SlicedObject);
                var slicedObj = Value.CreateInteger(0L);
                var objContext = frame.GetObjectContext();

                // This is an instance variable.
                if (frame.InObjectContext() && !string.IsNullOrEmpty(identifierName) && identifierName[0] == '@' && objContext != null && objContext.HasVariable(identifierName))
                {
                    slicedObj = objContext.InstanceVariables[identifierName];
                }
                else if (scope.TryGet(identifierName, out Value val))
                {
                    slicedObj = val;
                }
                else
                {
                    throw new VariableUndefinedError(node.Token, identifierName);
                }

                var slice = GetSlice(sliceExpr, slicedObj);

                DoSliceAssignment(node.Token, ref slicedObj, slice, ref newValue);
                scope.Assign(identifierName, slicedObj);
            }
        }
        else if (node.Object != null && node.Object.Type == ASTNodeType.Index)
        {
            var indexExpr = (IndexingNode)node.Object;

            if (indexExpr.IndexedObject == null)
            {
                throw new IndexError(indexExpr.Token, "Invalid indexing expression.");
            }

            if (indexExpr.IndexedObject.Type is ASTNodeType.Identifier or ASTNodeType.Self)
            {
                string identifierName = indexExpr.IndexedObject.Type == ASTNodeType.Self
                    ? ((SelfNode)indexExpr.IndexedObject).Name
                    : Id(indexExpr.IndexedObject);
                var indexedObj = Value.CreateInteger(0L);
                var objContext = frame.GetObjectContext();

                // This is an instance variable.
                if (frame.InObjectContext() && !string.IsNullOrEmpty(identifierName) && identifierName[0] == '@' && objContext != null && objContext.HasVariable(identifierName))
                {
                    indexedObj = objContext.InstanceVariables[identifierName];
                }
                else if (scope.TryGet(identifierName, out Value val))
                {
                    indexedObj = val;
                }
                else
                {
                    throw new VariableUndefinedError(node.Token, identifierName);
                }

                var index = Interpret(indexExpr.IndexExpression);

                if (indexedObj.IsList() && index.IsInteger())
                {
                    var listObj = indexedObj.GetList();
                    var indexValue = (int)ConversionOp.GetInteger(node.Token, index);

                    if (indexValue < 0 || indexValue >= listObj.Count)
                    {
                        throw new IndexError(node.Token, "The index was outside the bounds of the list.");
                    }

                    // Handle nested indexing
                    if (indexExpr.IndexExpression.Type == ASTNodeType.Index)
                    {
                        listObj[indexValue] = HandleNestedIndexing(indexExpr, listObj[indexValue], op, newValue);
                    }
                    else
                    {
                        if (op == TokenName.Ops_Assign)
                        {
                            listObj[indexValue] = newValue;
                        }
                        else
                        {
                            var oldValue = listObj[indexValue];
                            listObj[indexValue] = OpDispatch.DoBinary(node.Token, op, ref oldValue, ref newValue);
                        }
                    }

                    if (identifierName[0] == '@' && objContext != null)
                        objContext.InstanceVariables[identifierName] = indexedObj;
                    else
                        scope.Assign(identifierName, Value.CreateList(listObj));
                }
                else if (indexedObj.IsHashmap())
                {
                    var hashObj = indexedObj.GetHashmap();

                    if (op == TokenName.Ops_Assign)
                    {
                        hashObj[index] = newValue;
                    }
                    else
                    {
                        if (!hashObj.TryGetValue(index, out Value? oldValue))
                        {
                            throw new HashKeyError(node.Token, Serializer.Serialize(index));
                        }

                        hashObj[index] = OpDispatch.DoBinary(node.Token, op, ref oldValue, ref newValue);
                    }
                }
                else if (indexedObj.IsBytes())
                {
                    var bytesObj = indexedObj.GetBytes().ToList();
                    var indexValue = (int)ConversionOp.GetInteger(node.Token, index);

                    if (indexValue < 0 || indexValue >= bytesObj.Count)
                    {
                        throw new IndexError(node.Token, "The index was outside the bounds of the bytes.");
                    }

                    TypeError.ExpectInteger(node.Token, newValue);

                    var nValue = newValue.GetInteger();
                    TypeError.ByteCheck(node.Token, nValue);

                    if (op == TokenName.Ops_Assign)
                    {
                        bytesObj[indexValue] = (byte)nValue;
                    }
                    else
                    {
                        var oldValue = Value.CreateInteger(bytesObj[indexValue]);
                        var assignValue = OpDispatch.DoBinary(node.Token, op, ref oldValue, ref newValue);
                        TypeError.ByteCheck(node.Token, assignValue.GetInteger());
                        bytesObj[indexValue] = (byte)assignValue.GetInteger();
                    }

                    if (identifierName[0] == '@' && objContext != null)
                        objContext.InstanceVariables[identifierName] = Value.CreateBytes([.. bytesObj]);
                    else
                        scope.Assign(identifierName, Value.CreateBytes([.. bytesObj]));
                }
            }
            else if (indexExpr.IndexedObject.Type == ASTNodeType.Index)
            {
                var indexedObj = (IndexingNode)indexExpr.IndexedObject;

                if (indexedObj.IndexedObject == null || indexedObj.IndexedObject.Type != ASTNodeType.Identifier)
                {
                    throw new IndexError(indexExpr.Token, "Invalid nested indexing expression.");
                }

                var baseObj = Interpret(indexExpr.IndexedObject);
                _ = HandleNestedIndexing(indexExpr, baseObj, op, newValue);
            }
        }

        return Value.Default;
    }

    private Value Visit(MemberAssignmentNode node)
    {
        var target = node.Object;
        var memberName = node.MemberName;
        var key = Value.CreateString(memberName);
        var newValue = Interpret(node.Initializer);

        if (target?.Type == ASTNodeType.Identifier)
        {
            var varName = Id(target);
            var frame = CallStack.Peek();
            var scope = frame.Scope;

            if (!scope.TryGet(varName, out var container))
            {
                // Check if it's a struct name (static member assignment)
                if (Context.HasStruct(varName))
                {
                    var kstruct = Context.Structs[varName];
                    ApplyStaticVarAssignment(node.Token, kstruct, memberName, node.Op, ref newValue);
                    return Value.Default;
                }

                throw new VariableUndefinedError(node.Token, varName);
            }

            if (container.IsStruct())
            {
                var kstruct = Context.Structs[container.GetStruct().Identifier];
                ApplyStaticVarAssignment(node.Token, kstruct, memberName, node.Op, ref newValue);
                return Value.Default;
            }

            ApplyMemberAssignment(node.Token, ref container, key, node.Op, ref newValue);
            scope.Assign(varName, container);
        }
        else
        {
            var container = Interpret(target);

            if (container.IsStruct())
            {
                var kstruct = Context.Structs[container.GetStruct().Identifier];
                ApplyStaticVarAssignment(node.Token, kstruct, memberName, node.Op, ref newValue);
                return Value.Default;
            }

            ApplyMemberAssignment(node.Token, ref container, key, node.Op, ref newValue);
        }

        return Value.Default;
    }

    private void ApplyStaticVarAssignment(Token token, KStruct kstruct, string memberName, TokenName op, ref Value newValue)
    {
        var varKey = "@@" + memberName;

        if (op == TokenName.Ops_Assign)
        {
            kstruct.StaticVariables[varKey] = newValue;
        }
        else
        {
            if (!kstruct.StaticVariables.TryGetValue(varKey, out var oldVal))
            {
                throw new VariableUndefinedError(token, memberName);
            }

            kstruct.StaticVariables[varKey] = op == TokenName.Ops_BitwiseNotAssign
                ? BitwiseOp.Not(token, ref oldVal)
                : OpDispatch.DoBinary(token, op, ref oldVal, ref newValue);
        }
    }

    private void ApplyMemberAssignment(Token token, ref Value container, Value key, TokenName op, ref Value newValue)
    {
        if (!container.IsHashmap())
        {
            throw new TypeError(token, $"Cannot assign to member of type {container.Type}");
        }

        var dict = container.GetHashmap();

        if (op == TokenName.Ops_Assign)
        {
            dict[key] = newValue;
        }
        else
        {
            if (!dict.TryGetValue(key, out var oldValue))
            {
                throw new HashKeyError(token, Serializer.Serialize(key));
            }

            dict[key] = OpDispatch.DoBinary(token, op, ref oldValue, ref newValue);
        }

        // Update container (in case of future write-back)
        container = Value.CreateHashmap(dict);
    }

    private Value Visit(PackAssignmentNode node)
    {
        var frame = CallStack.Peek();
        var scope = frame.Scope;

        // Evaluate RHS
        var rhsValues = node.Right.Select(Interpret).ToList();

        // Unpack if RHS is a single list
        if (rhsValues.Count == 1 && rhsValues[0].IsList())
        {
            rhsValues = rhsValues[0].GetList();
        }

        var lhsCount = node.Left.Count;
        var rhsCount = rhsValues.Count;

        // Assign to each LHS identifier
        for (int i = 0; i < lhsCount; i++)
        {
            var lhs = node.Left[i];
            if (lhs == null) continue;

            if (lhs.Type != ASTNodeType.Identifier)
            {
                throw new SyntaxError(node.Token, "Left side of pack assignment must be identifier");
            }

            var name = Id(lhs);
            var value = i < rhsCount ? rhsValues[i] : Value.CreateNull();

            scope.Assign(name, value);
        }

        return Value.Default;
    }

    private Value Visit(MemberAccessNode node)
    {
        var obj = Interpret(node.Object);
        var memberName = node.MemberName;

        if (obj.IsPackage())
        {
            var pkgName = obj.GetPackage().Identifier;
            var qualifiedName = pkgName + "::" + memberName;

            if (Context.Constants.TryGetValue(qualifiedName, out Value? constVal))
            {
                return constVal;
            }

            if (Context.PackageVariables.TryGetValue(qualifiedName, out Value? pkgVar))
            {
                return pkgVar;
            }

            if (Context.HasFunction(qualifiedName))
            {
                return Value.CreateLambda(new LambdaRef { Identifier = qualifiedName });
            }

            throw new RuntimeError(node.Token, $"Package '{pkgName}' has no member '{memberName}'.", CaptureStackTrace());
        }
        else if (obj.IsHashmap())
        {
            var hash = obj.GetHashmap();
            var memberKey = Value.CreateString(memberName);

            if (!hash.TryGetValue(memberKey, out Value? memberValue))
            {
                return Value.CreateNull();
            }

            return memberValue;
        }
        else if (obj.IsObject())
        {
            var o = obj.GetObject();

            if (o.InstanceVariables.TryGetValue("@" + memberName, out Value? v))
            {
                if (v != null)
                {
                    return v;
                }
            }
        }
        else if (obj.IsStruct())
        {
            var struc = obj.GetStruct();
            var kstruct = Context.Structs[struc.Identifier];
            var varKey = "@@" + memberName;

            if (kstruct.StaticVariables.TryGetValue(varKey, out Value? sv))
            {
                return sv;
            }
        }

        return Value.Default;
    }

    private Value Visit(RangeLiteralNode node)
    {
        var startValue = Interpret(node.RangeStart);
        var stopValue = Interpret(node.RangeEnd);

        if (!startValue.IsInteger() || !stopValue.IsInteger())
        {
            throw new RangeError(node.Token, "Range value must be an integer.");
        }

        var start = startValue.GetInteger();
        var stop = stopValue.GetInteger();
        var step = (stop < start) ? -1 : 1;

        List<Value> list = [];

        for (var i = start; i != stop; i += step)
        {
            list.Add(Value.CreateInteger(i));
        }

        list.Add(Value.CreateInteger(stop));

        return Value.CreateList(list);
    }

    private Value Visit(ListLiteralNode node)
    {
        List<Value> list = [];

        foreach (var element in node.Elements)
        {
            var value = Interpret(element);
            list.Add(value);
        }

        return Value.CreateList(list);
    }

    private Value Visit(HashLiteralNode node)
    {
        Dictionary<Value, Value> hash = [];

        foreach (var pair in node.Elements)
        {
            var key = Interpret(pair.Key);
            var value = Interpret(pair.Value);
            hash[key] = value;
        }

        return Value.CreateHashmap(hash);
    }

    private Value Visit(IdentifierNode node)
    {
        var frame = CallStack.Peek();
        var name = node.Name;

        if (frame.InObjectContext() && name[0] == '@')
        {
            return frame.GetObjectContext()?.InstanceVariables[name] ?? Value.CreateNull();
        }

        if (frame.Scope.TryGet(name, out Value val))
        {
            return val;
        }
        else if (Context.Structs.TryGetValue(name, out var kstruct))
        {
            return kstruct.AsValue;
        }
        else if (Context.HasLambda(name))
        {
            // Use the stored Ref so the returned value carries the original internal identifier
            // (e.g. "<lambda_uuid>").  If we return a new LambdaRef with the user-defined name,
            // and the user-defined name is later cleaned up (local lambda in a returned function),
            // callers that try to re-register the lambda via the name will hit a KeyNotFoundException.
            return Value.CreateLambda(Context.Lambdas[name].Ref);
        }
        else if (Context.LambdaTable.TryGetValue(name, out var mappedId) &&
                 Context.Lambdas.ContainsKey(mappedId))
        {
            return Value.CreateLambda(new LambdaRef { Identifier = mappedId });
        }
        else if (Context.Constants.TryGetValue(name, out var constant))
        {
            return constant;
        }
        else if (Context.PackageVariables.TryGetValue(name, out var pkgVar))
        {
            return pkgVar;
        }
        else if (_globalScope.TryGet(node.Name, out val))
        {
            return val;
        }

        return Value.CreateNull();
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

    private Value InvokeCallable(Callable callable, List<ASTNode?> argNodes, Token token, string displayName, InstanceRef instance)
    {
        var args = argNodes.Select(Interpret).ToList();
        return InvokeCallable(callable, args, token, displayName, instance);
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

    private Value Visit(PrintNode node)
    {
        var value = Interpret(node.Expression);
        var text = value.IsObject() ? PrintObject(node.Token, value) : Serializer.Serialize(value);

        var writer = node.PrintStdError ? Console.Error : Console.Out;
        if (node.PrintNewline)
        {
            writer.WriteLine(text);
        }
        else
        {
            writer.Write(text);
        }

        return Value.Default;
    }

    private Value Visit(PrintXyNode node)
    {
        var text = node.Expression != null ? Interpret(node.Expression) : Value.Default;
        var x    = node.X          != null ? Interpret(node.X)          : Value.CreateInteger(1);
        var y    = node.Y          != null ? Interpret(node.Y)          : Value.CreateInteger(1);
        var col  = x.IsInteger() ? (int)x.GetInteger() : x.IsFloat() ? (int)x.GetFloat() : 1;
        var row  = y.IsInteger() ? (int)y.GetInteger() : y.IsFloat() ? (int)y.GetFloat() : 1;
        Console.SetCursorPosition(col - 1, row - 1);
        Console.Write(Serializer.Serialize(text));
        return Value.Default;
    }

    private Value Visit(TernaryOperationNode node)
    {
        var eval = Interpret(node.EvalExpression);

        if (BooleanOp.IsTruthy(eval))
        {
            return Interpret(node.TrueExpression);
        }

        return Interpret(node.FalseExpression);
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

    private Value Visit(BinaryOperationNode node)
    {
        var left = Interpret(node.Left);

        if (node.Op == TokenName.Ops_And && !BooleanOp.IsTruthy(left))
        {
            return Value.False;
        }
        else if (node.Op == TokenName.Ops_Or && BooleanOp.IsTruthy(left))
        {
            return Value.True;
        }
        else if (node.Op == TokenName.Ops_NullCoalesce && !left.IsNull())
        {
            return left;
        }

        if (left.IsObject())
        {
            var inst = left.GetObject();

            if (HasOperatorOverload(inst, node.Op, out KFunction? func))
            {
                if (func == null)
                {
                    throw new FunctionUndefinedError(node.Token, Serializer.GetOperatorString(node.Op));
                }

                return InvokeCallable(func, [node.Right], node.Token, $"{inst.StructName}#{func.Name}", inst);
            }
        }

        var right = Interpret(node.Right);
        return OpDispatch.DoBinary(node.Token, node.Op, ref left, ref right);
    }

    private Value Visit(UnaryOperationNode node)
    {
        var right = Interpret(node.Operand);
        return OpDispatch.DoUnary(node.Token, node.Op, ref right);
    }

    private Value Visit(DoNode node)
    {
        var frame = CallStack.Peek();
        var condition = node.Condition;
        var result = Value.Default;

        if (condition == null || BooleanOp.IsTruthy(Interpret(condition)))
        {
            var parent = EnterBlockScope(frame);
            try
            {
                foreach (var stmt in node.Body)
                {
                    result = Interpret(stmt);
                    if (frame.IsFlagSet(FrameFlags.Return))
                        return frame.ReturnValue ?? result;
                }
            }
            finally
            {
                ExitBlockScope(frame, parent);
            }
        }

        return result;
    }

    private Value Visit(IfNode node)
    {
        var conditionValue = Interpret(node.Condition);
        var frame = CallStack.Peek();
        var result = Value.Default;

        if (BooleanOp.IsTruthy(conditionValue))
        {
            var parent = EnterBlockScope(frame);
            try
            {
                foreach (var stmt in node.Body)
                {
                    result = Interpret(stmt);
                    if (frame.IsFlagSet(FrameFlags.Return))
                        return frame.ReturnValue ?? result;
                }
            }
            finally
            {
                ExitBlockScope(frame, parent);
            }
        }
        else
        {
            var executed = false;
            foreach (var elsifNode in node.ElsifNodes)
            {
                var elsifCondition = Interpret(elsifNode?.Condition);

                if (BooleanOp.IsTruthy(elsifCondition))
                {
                    if (elsifNode == null)
                    {
                        continue;
                    }

                    var parent = EnterBlockScope(frame);
                    try
                    {
                        foreach (var stmt in elsifNode.Body)
                        {
                            result = Interpret(stmt);
                            if (frame.IsFlagSet(FrameFlags.Return))
                                return frame.ReturnValue ?? result;
                        }
                    }
                    finally
                    {
                        ExitBlockScope(frame, parent);
                    }
                    executed = true;
                    break;
                }
            }

            if (!executed && node.ElseBody.Count > 0)
            {
                var parent = EnterBlockScope(frame);
                try
                {
                    foreach (var stmt in node.ElseBody)
                    {
                        result = Interpret(stmt);
                        if (frame.IsFlagSet(FrameFlags.Return))
                            return frame.ReturnValue ?? result;
                    }
                }
                finally
                {
                    ExitBlockScope(frame, parent);
                }
            }
        }

        return result;
    }

    private Value Visit(CaseWhenNode node)
    {
        return node.Conditions.Count > 0 ? Interpret(node.Conditions[0]) : Value.Default;
    }

    private Value Visit(CaseNode node)
    {
        var testValue = Value.True;
        var isSwitch = false;
        var result = Value.Default;

        if (node.TestValue != null)
        {
            testValue = Interpret(node.TestValue);
            isSwitch = true;

            if (node.TestValueAlias != null)
            {
                var id = Id(node.TestValueAlias);
                CallStack.Peek().Scope.Assign(id, testValue);
            }
        }

        foreach (var whenNode in node.WhenNodes)
        {
            var matched = false;
            foreach (var condNode in whenNode.Conditions)
            {
                if (isSwitch && condNode?.Type == ASTNodeType.RangeLiteral)
                {
                    var rangeNode = (RangeLiteralNode)condNode;
                    var start = Interpret(rangeNode.RangeStart);
                    var end = Interpret(rangeNode.RangeEnd);
                    if ((start.IsInteger() || start.IsFloat()) && (end.IsInteger() || end.IsFloat()))
                    {
                        var tv = testValue.IsFloat() ? testValue.GetFloat() : (double)(testValue.IsInteger() ? testValue.GetInteger() : 0L);
                        var sv = start.IsFloat() ? start.GetFloat() : (double)start.GetInteger();
                        var ev = end.IsFloat() ? end.GetFloat() : (double)end.GetInteger();
                        matched = sv <= ev ? (tv >= sv && tv <= ev) : (tv >= ev && tv <= sv);
                    }
                }
                else
                {
                    var condValue = Interpret(condNode);
                    if ((!isSwitch && BooleanOp.IsTruthy(condValue))
                        || (isSwitch && ComparisonOp.Equal(ref testValue, ref condValue)))
                    {
                        matched = true;
                    }
                }

                if (matched) break;
            }

            if (matched)
            {
                var frame = CallStack.Peek();
                foreach (var stmt in whenNode.Body)
                {
                    result = Interpret(stmt);
                    if (frame.IsFlagSet(FrameFlags.Return))
                    {
                        result = frame.ReturnValue ?? result;
                        break;
                    }
                }

                return result;
            }
        }

        if (node.ElseBody.Count > 0)
        {
            var frame = CallStack.Peek();

            foreach (var stmt in node.ElseBody)
            {
                result = Interpret(stmt);

                if (frame.IsFlagSet(FrameFlags.Return))
                {
                    return frame.ReturnValue ?? result;
                }
            }
        }

        return result;
    }

    private Value Visit(ForLoopNode node)
    {
        var dataSetValue = Interpret(node.DataSet);

        if (dataSetValue.IsList())
        {
            return ListLoop(node, dataSetValue.GetList());
        }
        else if (dataSetValue.IsHashmap())
        {
            return HashmapLoop(node, dataSetValue.GetHashmap());
        }
        else if (dataSetValue.IsBytes())
        {
            return BytesLoop(node, dataSetValue.GetBytes());
        }
        else if (dataSetValue.IsGenerator())
        {
            return GeneratorLoop(node, dataSetValue.GetGenerator());
        }

        throw new InvalidOperationError(node.Token, "Expected a list value in for-loop.");
    }

    private Value Visit(WhileLoopNode node)
    {
        var result = Value.Default;
        var frame = CallStack.Peek();
        var parent = EnterBlockScope(frame);
        frame.SetFlag(FrameFlags.InLoop);

        var fallOut = false;
        var iterations = 0;

        try
        {
        while (BooleanOp.IsTruthy(Interpret(node.Condition)))
        {
            if (Kiwi.Settings.SafeMode)
            {
                ++iterations;

                if (iterations == SafemodeMaxIterations)
                {
                    throw new InfiniteLoopError(node.Token, "Detected an infinite loop in safemode.");
                }
            }

            var skip = false;

            foreach (var stmt in node.Body)
            {
                if (skip)
                {
                    break;
                }

                if (stmt != null && stmt.Type != ASTNodeType.Next && stmt.Type != ASTNodeType.Break)
                {
                    result = Interpret(stmt);

                    if (frame.IsFlagSet(FrameFlags.Break))
                    {
                        frame.ClearFlag(FrameFlags.Break);
                        fallOut = true;
                        break;
                    }

                    if (frame.IsFlagSet(FrameFlags.Next))
                    {
                        frame.ClearFlag(FrameFlags.Next);
                        skip = true;
                        break;
                    }
                }

                if (frame.IsFlagSet(FrameFlags.Return))
                {
                    fallOut = true;
                    break;
                }

                if (stmt != null && stmt.Type == ASTNodeType.Next)
                {
                    var condition = ((NextNode)stmt).Condition;
                    if (condition == null || BooleanOp.IsTruthy(Interpret(condition)))
                    {
                        skip = true;
                        break;
                    }
                }
                else if (stmt != null && stmt.Type == ASTNodeType.Break)
                {
                    var condition = ((BreakNode)stmt).Condition;
                    if (condition == null || BooleanOp.IsTruthy(Interpret(condition)))
                    {
                        fallOut = true;
                        break;
                    }
                }
            }

            if (fallOut)
            {
                break;
            }
        }
        } // end try
        finally
        {
            frame.ClearFlag(FrameFlags.InLoop);
            ExitBlockScope(frame, parent);
        }

        return frame.ReturnValue ?? result;
    }

    private Value Visit(RepeatLoopNode node)
    {
        var countValue = Interpret(node.Count);
        if (!countValue.IsInteger())
        {
            throw new InvalidOperationError(node.Token, "Repeat loop count must be an integer.");
        }

        var count = (int)countValue.GetInteger();
        var frame = CallStack.Peek();
        var parent = EnterBlockScope(frame);
        var scope = frame.Scope;
        var result = Value.Default;

        string? aliasName = null;
        if (node.Alias != null)
        {
            aliasName = Id(node.Alias);
            scope.Declare(aliasName, Value.Default); // Will be updated each iteration
        }

        frame.SetFlag(FrameFlags.InLoop);

        try
        {
            for (int i = 1; i <= count; i++)
            {
                // Update loop alias (e.g., `i` in `repeat 5 as i`)
                if (aliasName != null)
                {
                    scope.SetLocal(aliasName, Value.CreateInteger(i));
                }

                result = ExecuteLoopBody(node.Body, frame);
                if (frame.IsFlagSet(FrameFlags.Return))
                {
                    return frame.ReturnValue ?? result;
                }
            }
        }
        finally
        {
            frame.ClearFlag(FrameFlags.InLoop);
            ExitBlockScope(frame, parent);
        }

        return result;
    }

    private Value ExecuteLoopBody(List<ASTNode?> body, StackFrame frame)
    {
        var result = Value.Default;

        foreach (var stmt in body)
        {
            if (stmt == null) continue;

            if (stmt.Type == ASTNodeType.Next)
            {
                var next = (NextNode)stmt;
                if (next.Condition == null || BooleanOp.IsTruthy(Interpret(next.Condition)))
                {
                    frame.SetFlag(FrameFlags.Next);
                    break;
                }
            }
            else if (stmt.Type == ASTNodeType.Break)
            {
                var brk = (BreakNode)stmt;
                if (brk.Condition == null || BooleanOp.IsTruthy(Interpret(brk.Condition)))
                {
                    frame.SetFlag(FrameFlags.Break);
                    break;
                }
            }
            else
            {
                result = Interpret(stmt);
            }

            if (frame.IsFlagSet(FrameFlags.Return))
            {
                return frame.ReturnValue ?? result;
            }

            if (frame.IsFlagSet(FrameFlags.Break) ||
                frame.IsFlagSet(FrameFlags.Next))
            {
                break;
            }
        }

        // Clear Next flag for next iteration
        if (frame.IsFlagSet(FrameFlags.Next))
        {
            frame.ClearFlag(FrameFlags.Next);
        }

        return result;
    }

    private Value Visit(BreakNode node)
    {
        var condition = node.Condition;
        if (condition == null || BooleanOp.IsTruthy(Interpret(condition)))
        {
            var frame = CallStack.Peek();
            frame.SetFlag(FrameFlags.Break);
        }

        return Value.Default;
    }

    private Value Visit(NextNode node)
    {
        var condition = node.Condition;
        if (condition == null || BooleanOp.IsTruthy(Interpret(condition)))
        {
            var frame = CallStack.Peek();
            frame.SetFlag(FrameFlags.Next);
        }

        return Value.Default;
    }

    private Value Visit(TryNode node)
    {
        Value result = Value.Default;
        var tryName = $"try-{Guid.NewGuid()}";
        var setReturnValue = false;
        var stackDepthBeforeTry = CallStack.Count;

        try
        {
            var scope = new Scope(CallStack.Peek().Scope);
            var tryFrame = PushFrame(tryName, node.Token, scope);
            tryFrame.SetFlag(FrameFlags.InTry);

            result = ExecuteBody(node.TryBody, tryFrame);
            if (tryFrame.IsFlagSet(FrameFlags.Return))
            {
                result = tryFrame.ReturnValue ?? result;
                setReturnValue = true;
            }

            PopFrame();
        }
        catch (KiwiError e)
        {
            // Pop ALL frames pushed during the try block (the try frame itself plus any
            // orphaned callee frames whose C# stack was unwound by the exception).
            while (CallStack.Count > stackDepthBeforeTry)
            {
                PopFrame();
            }

            if (node.CatchBody.Count > 0)
            {
                var catchScope = new Scope(CallStack.Peek().Scope);
                var catchFrame = PushFrame("catch", node.Token, catchScope);

                if (node.ErrorType != null)
                {
                    var typeName = Id(node.ErrorType);
                    catchScope.Declare(typeName, Value.CreateString(e.Type));
                }
                if (node.ErrorMessage != null)
                {
                    var msgName = Id(node.ErrorMessage);
                    if (node.ErrorType == null)
                    {
                        // Single-param catch: bind a structured hashmap { error, message }
                        var errorMap = new Dictionary<Value, Value>
                        {
                            [Value.CreateString("error")]   = Value.CreateString(e.Type),
                            [Value.CreateString("message")] = Value.CreateString(e.Message ?? string.Empty)
                        };
                        catchScope.Declare(msgName, Value.CreateHashmap(errorMap));
                    }
                    else
                    {
                        // Two-param catch: bind message string
                        catchScope.Declare(msgName, Value.CreateString(e.Message ?? string.Empty));
                    }
                }

                result = ExecuteBody(node.CatchBody, catchFrame);
                if (catchFrame.IsFlagSet(FrameFlags.Return))
                {
                    result = catchFrame.ReturnValue ?? result;
                    setReturnValue = true;
                }
                PopFrame();
            }
            // No catch and no finally: bare try...end silently swallows the error.
        }

        if (node.FinallyBody.Count > 0)
        {
            var finallyResult = ExecuteBody(node.FinallyBody, CallStack.Peek());
            if (CallStack.Peek().IsFlagSet(FrameFlags.Return))
            {
                result = CallStack.Peek().ReturnValue ?? finallyResult;
                setReturnValue = true;
            }
            else
            {
                result = finallyResult;
            }
        }

        if (setReturnValue)
        {
            var frame = CallStack.Peek();
            if (!frame.IsFlagSet(FrameFlags.InLambda))
            {
                frame.SetFlag(FrameFlags.Return);
                frame.ReturnValue = result;
            }
        }

        return result;
    }

    private Value Visit(LambdaNode node, bool map = true)
    {
        List<KeyValuePair<string, Value>> parameters = [];
        HashSet<string> defaultParameters = [];

        foreach (var pair in node.Parameters)
        {
            var paramName = pair.Key;
            var paramValue = Value.Default;

            if (pair.Value != null)
            {
                paramValue = Interpret(pair.Value);
                defaultParameters.Add(paramName);
            }

            parameters.Add(new(paramName, paramValue));
        }

        var internalName = $"<lambda_{Guid.NewGuid()}>";
        var lref = new LambdaRef { Identifier = internalName };
        Context.Lambdas[internalName] = new KLambda(node)
        {
            Parameters = parameters,
            DefaultParameters = defaultParameters,
            TypeHints = node.TypeHints,
            ReturnTypeHint = node.ReturnTypeHint,
            CapturedScope = CaptureCurrentScope(),
            VariadicParamName = node.VariadicParamName,
            Ref = lref
        };

        if (map)
        {
            Context.AddMappedLambda(node.Token.Text, internalName);
        }

        return Value.CreateLambda(lref);
    }

    private Value Visit(DecoratedFunctionNode node)
    {
        var inStruct = StructStack.Count > 0;

        // 1. Define the function normally.
        Visit(node.Function, inStruct);

        // 2. Compute the qualified registered name (mirrors Visit(FunctionNode) prefix logic).
        var funcName = node.Function.Name;
        var registeredName = funcName;
        if (PackageStack.Count > 0 && !inStruct)
        {
            Stack<string> tmpStack = new([.. PackageStack]);
            var prefix = string.Empty;
            while (tmpStack.Count > 0) { prefix += tmpStack.Peek() + "::"; tmpStack.Pop(); }
            registeredName = prefix + funcName;
        }

        if (!Context.HasFunction(registeredName))
            return Value.Default;

        // 3. Wrap the original function as a lambda so it can be passed as a value.
        var kfunc = Context.Functions[registeredName];
        var klambda = new KLambda(kfunc.Decl.ToLambda())
        {
            Parameters = kfunc.Parameters,
            DefaultParameters = kfunc.DefaultParameters,
            TypeHints = kfunc.TypeHints,
            ReturnTypeHint = kfunc.ReturnTypeHint,
            CapturedScope = kfunc.CapturedScope
        };
        var internalName = $"_deco_{Guid.NewGuid().ToString()[..8]}_";
        Context.Lambdas[internalName] = klambda;
        Value current = Value.CreateLambda(new LambdaRef { Identifier = internalName });

        // 4. Apply decorators bottom-to-top: @A @B fn f => f = A(B(f)).
        for (int i = node.Decorators.Count - 1; i >= 0; i--)
        {
            var (decoExpr, extraArgs) = node.Decorators[i];
            List<ASTNode?> argNodes = [new LiteralNode(current) { Token = node.Token }];
            argNodes.AddRange(extraArgs);
            current = CallDecoratorExpr(node.Token, decoExpr, argNodes);
        }

        // 5. Replace the function entry with the decorated result.
        Context.Functions.Remove(registeredName);
        if (current.IsLambda())
        {
            Context.AddMappedLambda(registeredName, current.GetLambda().Identifier);
        }
        else
        {
            _globalScope.Assign(registeredName, current);
        }

        return Value.Default;
    }

    private Value CallDecoratorExpr(Token token, ASTNode? expr, List<ASTNode?> argNodes)
    {
        // Fast path: bare identifier — look up directly by name.
        if (expr is IdentifierNode ident)
            return CallDecoratorByName(token, ident.Name, argNodes);

        // General path: evaluate the expression (e.g. x.decorate) to get a callable value.
        var callableVal = Interpret(expr);
        if (!callableVal.IsLambda())
            throw new FunctionUndefinedError(token, "<decorator expression>");

        return CallDecoratorByName(token, callableVal.GetLambda().Identifier, argNodes);
    }

    private Value CallDecoratorByName(Token token, string name, List<ASTNode?> argNodes)
    {
        if (Context.HasFunction(name))
            return CallFunction(Context.Functions[name], argNodes, token, name);

        if (Context.HasMappedLambda(name) || Context.HasLambda(name))
        {
            var lambda = Context.HasMappedLambda(name)
                ? Context.Lambdas[Context.LambdaTable[name]]
                : Context.Lambdas[name];

            // Use ResolveArguments so named/keyword args are matched by parameter name.
            var tmpScope = new Scope(_globalScope);
            var slots = ResolveArguments(lambda.Parameters, argNodes, lambda.DefaultParameters, token, name, lambda.VariadicParamName, tmpScope);
            var ordered = slots.Select(s => s!).ToList();
            // Append variadic overflow so BindParameters collects it correctly.
            if (!string.IsNullOrEmpty(lambda.VariadicParamName) && tmpScope.TryGet(lambda.VariadicParamName, out var varList) && varList.IsList())
                ordered.AddRange(varList.GetList());

            return InvokeCallable(lambda, ordered, token, name);
        }

        throw new FunctionUndefinedError(token, name);
    }

    private Value Visit(FunctionNode node, bool inStruct = false)
    {
        var name = node.Name;

        if (PackageStack.Count > 0 && !inStruct)
        {
            Stack<string> tmpStack = new([.. PackageStack]);
            var prefix = string.Empty;
            while (tmpStack.Count > 0)
            {
                prefix += tmpStack.Peek() + "::";
                tmpStack.Pop();
            }
            name = prefix + name;
        }

        if (StructStack.Count > 0)
        {
            Context.Methods[name] = CreateFunction(node, name);
        }
        else
        {
            Context.Functions[name] = CreateFunction(node, name);

            // If declared inside a user function, track it for cleanup on return.
            var enclosing = CallStack.FirstOrDefault(f => f.IsFunction);
            enclosing?.LocalFunctions.Add(name);
        }

        return Value.Default;
    }

    private Value Visit(VariableNode node)
    {
        var typeHints = node.TypeHints;

        // we're going to be injecting these into the stack frame
        var scope = CallStack.Peek().Scope;
        // for each declared variable
        foreach (var pair in node.Variables)
        {
            var value = Value.Default; // a variable value
            var name = pair.Key;  // grab the name
                                  // a flag to determine if a variable has an initializer
            var hasDefaultValue = pair.Value != null;

            // if there is a default value, grab it
            if (!hasDefaultValue)
            {
                // default to null
                value = Value.CreateNull();
            }
            else
            {
                value = Interpret(pair.Value).Clone();

            }

            // check for a type-hint
            if (typeHints.TryGetValue(name, out List<int>? expectedTypes))
            {
                var expectedType = expectedTypes[0];
                // if a default value was supplied, expect it to match the type
                if (hasDefaultValue && !AssertTypeMatch(node.Token, value, expectedTypes))
                {
                    throw new TypeError(node.Token, $"Expected type `{string.Join("|", expectedTypes.Select(TypeRegistry.GetTypeName))}` for variable `{name}` but received `{TypeRegistry.GetTypeName(value)}`.");
                }
                else if (!hasDefaultValue)
                {
                    // give it a default value based on the type-hint
                    if (TypeRegistry.IsPrimitive(expectedType))
                    {
                        var expectedValueType = TypeRegistry.GetValueType(expectedType);

                        switch (expectedValueType)
                        {
                            case Typing.ValueType.Boolean:
                                value.SetValue(false);
                                break;

                            case Typing.ValueType.Integer:
                                value.SetValue(0);
                                break;

                            case Typing.ValueType.Float:
                                value.SetValue(0.0);
                                break;

                            case Typing.ValueType.String:
                                value.SetValue(string.Empty);
                                break;

                            case Typing.ValueType.List:
                                value.SetValue(Value.CreateList());
                                break;

                            case Typing.ValueType.Hashmap:
                                value.SetValue(Value.CreateHashmap());
                                break;

                            default:
                                // to throw, or not to throw...
                                break;
                        }
                    }
                }
            }

            // inject the variable
            scope.Declare(name, value);
        }

        return Value.Default;
    }

    private Value Visit(LambdaCallNode node)
    {
        var nodeValue = Interpret(node.LambdaNode);
        if (!nodeValue.IsLambda())
            throw new InvalidOperationError(node.Token, $"Expected a callable but got `{TypeRegistry.GetTypeName(nodeValue)}`.");
        var lambdaName = nodeValue.GetLambda().Identifier;
        var result = Value.Default;
        var doPop = false;

        try
        {
            result = CallLambda(node.Token, lambdaName, node.Arguments, ref doPop);
            if (doPop) PopFrame();
        }
        catch (KiwiError)
        {
            if (doPop && InTry())
            {
                PopFrame();
            }
            throw;
        }

        return result;
    }

    private Value Visit(FunctionCallNode node)
    {
        var result = Value.Default;
        var callableType = GetCallable(node.Token, node.FunctionName);
        var doPop = false;

        try
        {
            switch (callableType)
            {
                case CallableType.Builtin:
                    result = CallBuiltin(node);
                    break;

                case CallableType.Method:
                    result = ExecuteInstanceMethod(node, ref doPop);
                    break;

                case CallableType.Function:
                    var func = Context.Functions[node.FunctionName];
                    if (func.IsGenerator)
                    {
                        result = CreateGenerator(func, node.Arguments, node.Token);
                    }
                    else
                    {
                        result = CallFunction(node);
                        doPop = true;
                    }
                    break;

                case CallableType.Lambda:
                    result = CallLambda(node.Token, node.FunctionName, node.Arguments, ref doPop);
                    break;
            }

            if (doPop)
            {
                PopFrame();
            }
        }
        catch (KiwiError)
        {
            if (doPop)
            {
                PopFrame();
            }

            throw;
        }

        return result;
    }

    private Value Visit(MethodCallNode node)
    {
        var obj = Interpret(node.Object);

        if (obj.IsPackage())
        {
            return CallPackageMethod(node, obj.GetPackage());
        }
        else if (obj.IsObject())
        {
            return CallObjectMethod(node, obj.GetObject());
        }
        else if (obj.IsStruct())
        {
            return CallStructMethod(node, obj.GetStruct());
        }
        else if (ListBuiltin.IsBuiltin(node.Op))
        {
            return ListBuiltinHandler.HandleListBuiltin(this, node.Token, ref obj, node.Op, GetMethodCallArguments(node.Arguments));
        }
        else if (CallableBuiltin.IsBuiltin(node.Op))
        {
            return HandleCallableBuiltin(node, GetMethodCallArguments(node.Arguments));
        }
        else if (CoreBuiltin.IsBuiltin(node.Op))
        {
            return BuiltinDispatch.Execute(node.Token, node.Op, obj, GetMethodCallArguments(node.Arguments));
        }
        else
        {
            var type = TypeRegistry.GetType(node.Token, obj);
            
            // check for type builtin
            if (TypeBuiltins.TryGetBuiltin(type, node.MethodName, out KFunction? typeBuiltin) && typeBuiltin != null)
            {
                List<Value> args = [];
                args.Add(obj);
                args.AddRange(GetMethodCallArguments(node.Arguments));
                return InvokeCallable(typeBuiltin, args, node.Token, node.MethodName);
            }
        }

        throw new FunctionUndefinedError(node.Token, node.MethodName);
    }

    private Value Visit(ReturnNode node)
    {
        var returnValue = Value.Default;

        if (node.Condition == null || BooleanOp.IsTruthy(Interpret(node.Condition)))
        {
            if (node.ReturnValue != null)
            {
                returnValue = Interpret(node.ReturnValue);
            }

            var frame = CallStack.Peek();
            frame.SetFlag(FrameFlags.Return);
            frame.ReturnValue = returnValue;
            return returnValue;
        }

        return returnValue;
    }

    private Value Visit(YieldNode node)
    {
        if (_activeGenerator == null)
        {
            throw new RuntimeError(node.Token, "'yield' used outside of a generator function.", CaptureStackTrace());
        }

        var value = node.YieldValue != null ? Interpret(node.YieldValue) : Value.Default;
        _activeGenerator.Yield(value);
        return Value.Default;
    }

    private Value CreateGenerator(KFunction func, List<ASTNode?> argNodes, Token token)
    {
        // Evaluate arguments in the caller's context before starting the generator thread
        var args = argNodes.Select(arg => arg != null ? Interpret(arg) : Value.Default).ToList();
        return CreateGeneratorFromValues(func, args, token);
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

    private Value GeneratorLoop(ForLoopNode node, GeneratorRef genRef)
    {
        var frame = CallStack.Peek();
        var parent = EnterBlockScope(frame);
        var scope = frame.Scope;
        frame.SetFlag(FrameFlags.InLoop);

        var valueName = Id(node.ValueIterator);
        string? indexName = null;
        if (node.IndexIterator != null)
            indexName = Id(node.IndexIterator);

        scope.Declare(valueName, Value.Default);
        if (indexName != null)
            scope.Declare(indexName, Value.Default);

        var result = Value.Default;
        var fallOut = false;
        long index = 0;

        try
        {
            while (true)
            {
                var (hasValue, value) = genRef.Next();
                if (!hasValue) break;

                scope.SetLocal(valueName, value);
                if (indexName != null)
                    scope.SetLocal(indexName, Value.CreateInteger(index));
                index++;

                var skip = false;

                foreach (var stmt in node.Body)
                {
                    if (skip || fallOut) break;
                    if (stmt == null) continue;

                    var stmtType = stmt.Type;
                    if (stmtType != ASTNodeType.Next && stmtType != ASTNodeType.Break)
                    {
                        result = Interpret(stmt);

                        if (frame.IsFlagSet(FrameFlags.Break))
                        {
                            frame.ClearFlag(FrameFlags.Break);
                            fallOut = true;
                            break;
                        }

                        if (frame.IsFlagSet(FrameFlags.Next))
                        {
                            frame.ClearFlag(FrameFlags.Next);
                            skip = true;
                            break;
                        }
                    }

                    if (frame.IsFlagSet(FrameFlags.Return))
                        return frame.ReturnValue ?? result;

                    if (stmtType == ASTNodeType.Next)
                    {
                        var condition = ((NextNode)stmt).Condition;
                        if (condition == null || BooleanOp.IsTruthy(Interpret(condition)))
                        {
                            skip = true;
                            break;
                        }
                    }
                    else if (stmtType == ASTNodeType.Break)
                    {
                        var condition = ((BreakNode)stmt).Condition;
                        if (condition == null || BooleanOp.IsTruthy(Interpret(condition)))
                        {
                            fallOut = true;
                            break;
                        }
                    }
                }

                if (fallOut) break;
            }
        }
        finally
        {
            frame.ClearFlag(FrameFlags.InLoop);
            genRef.Dispose();
            ExitBlockScope(frame, parent);
        }

        return result;
    }

    private Value Visit(IndexingNode node)
    {
        if (node.IndexedObject == null)
        {
            throw new InvalidOperationError(node.Token, "Nothing to index.");
        }

        var obj = Interpret(node.IndexedObject);
        var indexValue = Interpret(node.IndexExpression);

        if (obj.IsList())
        {
            var index = ConversionOp.GetInteger(node.Token, indexValue);
            var list = obj.GetList();

            if (index < 0 || index >= list.Count)
            {
                throw new IndexError(node.Token, "The index was outside the bounds of the list.");
            }

            return list[(int)index];
        }
        else if (obj.IsBytes())
        {
            var index = ConversionOp.GetInteger(node.Token, indexValue);
            var list = obj.GetBytes();

            if (index < 0 || index >= list.Length)
            {
                throw new IndexError(node.Token, "The index was outside the bounds of the bytes.");
            }

            return Value.CreateInteger(list[(int)index]);
        }
        else if (obj.IsHashmap())
        {
            var hash = obj.GetHashmap();

            if (!hash.TryGetValue(indexValue, out Value? value))
            {
                return Value.CreateNull();
            }

            return value;
        }
        else if (obj.IsString())
        {
            var str = obj.GetString();
            var index = ConversionOp.GetInteger(node.Token, indexValue);

            if (index < 0 || index >= str.Length)
            {
                throw new IndexError(node.Token, "The index was outside the bounds of the string.");
            }

            return Value.CreateChar(str[(int)index]);
        }

        throw new IndexError(node.Token, "Invalid indexing operation.");
    }

    private Value Visit(SliceNode node)
    {
        var obj = Interpret(node.SlicedObject);
        var slice = GetSlice(node, obj);

        if (obj.IsString())
        {
            return SliceUtil.StringSlice(node.Token, slice, obj.GetString());
        }
        else if (obj.IsList())
        {
            return SliceUtil.ListSlice(node.Token, slice, obj.GetList());
        }
        else if (obj.IsBytes())
        {
            return SliceUtil.BytesSlice(node.Token, slice, obj.GetBytes());
        }

        throw new InvalidOperationError(node.Token, $"Non-sliceable type: `{TypeRegistry.GetTypeName(obj)}`");
    }

    private Value Visit(EvalNode node)
    {
        var content = Interpret(node.ParseValue);

        if (!content.IsString())
        {
            throw new KiwiError(node.Token, "Invalid parse expression.");
        }

        using Lexer lexer = new(node.Token.Span.File, content.GetString());

        Parser p = new(true);
        var tokenStream = lexer.GetTokenStream();
        var ast = p.ParseTokenStream(tokenStream, true);

        var result = Interpret(ast);

        return result;
    }

    private KFunction CreateFunction(FunctionNode node, string name)
    {
        List<KeyValuePair<string, Value>> parameters = [];
        HashSet<string> defaultParameters = [];
        var typeHints = node.TypeHints;
        var paramCount = 0;

        foreach (var pair in node.Parameters)
        {
            var paramValue = Value.Default;
            var paramName = pair.Key;
            ++paramCount;

            if (pair.Value != null)
            {
                paramValue = Interpret(pair.Value);

                if (typeHints.TryGetValue(paramName, out List<int>? expectedTypes))
                {
                    if (!AssertTypeMatch(node.Token, paramValue, expectedTypes))
                    {
                        throw new TypeError(node.Token, $"Expected type `{string.Join("|", expectedTypes.Select(TypeRegistry.GetTypeName))}` for parameter {paramCount} of `{name}` but received `{TypeRegistry.GetTypeName(paramValue)}`.");
                    }
                }

                defaultParameters.Add(paramName);
            }

            parameters.Add(new(paramName, paramValue));
        }

        var isPackageFn = PackageStack.Count > 0;

        return new KFunction(node)
        {
            Name = node.Name,
            Parameters = parameters,
            DefaultParameters = defaultParameters,
            IsPrivate = node.IsPrivate,
            IsStatic = node.IsStatic,
            IsCtor = node.Name == "new",
            IsGenerator = node.IsGenerator,
            TypeHints = node.TypeHints,
            ReturnTypeHint = node.ReturnTypeHint,
            // Package functions capture the global scope rather than the caller's
            // local scope so their internal variables can't clobber caller variables.
            CapturedScope = CaptureCurrentScope(),
            IsPackageFunction = isPackageFn,
            VariadicParamName = node.VariadicParamName
        };
    }

    private Value HandleNestedIndexing(IndexingNode indexExpr, Value baseObj, TokenName op, Value newValue)
    {
        if (indexExpr.IndexExpression == null)
        {
            throw new IndexError(indexExpr.Token, "Invalid index expression.");
        }

        if (indexExpr.IndexExpression.Type == ASTNodeType.Index)
        {
            var nestedIndexExpr = (IndexingNode)indexExpr.IndexExpression;
            var nestedIndex = Interpret(nestedIndexExpr.IndexExpression);

            if (!baseObj.IsList() || !nestedIndex.IsInteger())
            {
                throw new IndexError(indexExpr.Token, "Nested index does not target a list.");
            }

            var listObj = baseObj.GetList();
            var indexValue = (int)ConversionOp.GetInteger(indexExpr.Token, nestedIndex);

            if (indexValue < 0 || indexValue >= listObj.Count)
            {
                throw new IndexError(indexExpr.Token, "The index was outside the bounds of the list.");
            }

            if (nestedIndexExpr.IndexExpression != null && nestedIndexExpr.IndexExpression.Type == ASTNodeType.Index)
            {
                listObj[indexValue] = HandleNestedIndexing(nestedIndexExpr, listObj[indexValue], op, newValue);
            }
            else if (op == TokenName.Ops_Assign)
            {
                listObj[indexValue] = newValue;
            }
            else
            {
                var oldValue = listObj[indexValue];
                listObj[indexValue] = OpDispatch.DoBinary(indexExpr.Token, op, ref oldValue, ref newValue);
            }

            return Value.CreateList(listObj);
        }
        else
        {
            // General case: evaluate the index expression (handles Identifier, Self,
            // Literal, BinaryOperation, FunctionCall, MethodCall, MemberAccess, etc.)
            var indexVal = Interpret(indexExpr.IndexExpression);

            if (baseObj.IsList())
            {
                var list = baseObj.GetList();
                var listIndex = (int)ConversionOp.GetInteger(indexExpr.Token, indexVal);

                if (listIndex < 0 || listIndex >= list.Count)
                {
                    throw new IndexError(indexExpr.Token, "The index was outside the bounds of the list.");
                }

                if (op == TokenName.Ops_Assign)
                {
                    list[listIndex] = newValue;
                }
                else
                {
                    var oldValue = list[listIndex];
                    list[listIndex] = OpDispatch.DoBinary(indexExpr.Token, op, ref oldValue, ref newValue);
                }

                return Value.CreateList(list);
            }
            else if (baseObj.IsHashmap())
            {
                var hash = baseObj.GetHashmap();

                if (op == TokenName.Ops_Assign)
                {
                    hash[indexVal] = newValue;
                }
                else
                {
                    if (!hash.TryGetValue(indexVal, out Value? oldValue))
                    {
                        throw new HashKeyError(indexExpr.Token, Serializer.Serialize(indexVal));
                    }
                    hash[indexVal] = OpDispatch.DoBinary(indexExpr.Token, op, ref oldValue, ref newValue);
                }

                return Value.CreateHashmap(hash);
            }

            throw new IndexError(indexExpr.Token, "Invalid index expression.");
        }
    }

    private SliceIndex GetSlice(SliceNode node, Value obj)
    {
        long start = 0L;
        long stop = obj.IsList() ? obj.GetList().Count
                  : obj.IsString() ? obj.GetString().Length
                  : obj.IsBytes() ? (long)obj.GetBytes().Length
                  : 0L;
        long step = 1L;

        if (node.StartExpression != null)
        {
            var v = Interpret(node.StartExpression);
            start = v.IsInteger() ? v.GetInteger() : (long)v.GetFloat();
        }

        if (node.StopExpression != null)
        {
            var v = Interpret(node.StopExpression);
            stop = v.IsInteger() ? v.GetInteger() : (long)v.GetFloat();
        }

        if (node.StepExpression != null)
        {
            var v = Interpret(node.StepExpression);
            step = v.IsInteger() ? v.GetInteger() : (long)v.GetFloat();
        }

        return new(Value.CreateInteger(start), Value.CreateInteger(stop), Value.CreateInteger(step))
        {
            IsSlice = true
        };
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

    private List<Value> GetMethodCallArguments(List<ASTNode?> args)
    {
        List<Value> arguments = [];

        foreach (var arg in args)
        {
            if (arg is SplatNode splat)
            {
                var listVal = Interpret(splat.Expression);
                if (listVal.IsList())
                    arguments.AddRange(listVal.GetList());
                else
                    arguments.Add(listVal);
            }
            else
            {
                arguments.Add(Interpret(arg));
            }
        }

        return arguments;
    }

    // Evaluate call arguments in the current scope, expanding SplatNodes.
    private List<Value> EvaluateCallArgs(List<ASTNode?> args)
    {
        List<Value> result = [];
        foreach (var arg in args)
        {
            if (arg is SplatNode splat)
            {
                var listVal = Interpret(splat.Expression);
                if (listVal.IsList())
                    result.AddRange(listVal.GetList());
                else
                    result.Add(listVal);
            }
            else
            {
                result.Add(Interpret(arg));
            }
        }
        return result;
    }

    private Value CallObjectMethod(MethodCallNode node, InstanceRef obj)
    {
        var struc = Context.Structs[obj.StructName];
        var methodName = node.MethodName;

        if (!struc.Methods.TryGetValue(methodName, out KFunction? function))
        {
            var baseStruct = struc.BaseStruct;

            if (string.IsNullOrEmpty(baseStruct))
            {
                throw new UnimplementedMethodError(node.Token, obj.StructName, methodName);
            }

            if (!Context.HasStruct(baseStruct))
            {
                throw new StructUndefinedError(node.Token, baseStruct);
            }

            return CallObjectBaseMethod(node, obj, baseStruct, methodName);
        }

        var isCtor = methodName == "new";
        var frame = CallStack.Peek();
        var oldObjContext = frame.GetObjectContext();
        var contextSwitch = false;

        if (frame.InObjectContext())
        {
            contextSwitch = true;
        }

        frame.SetObjectContext(obj);

        if (function == null)
        {
            throw new UnimplementedMethodError(node.Token, obj.StructName, methodName);
        }

        if (function.IsPrivate && !(contextSwitch && oldObjContext?.StructName == obj.StructName))
        {
            throw new InvalidContextError(node.Token, "Cannot invoke private method outside of struct.");
        }

        var result = CallFunction(function, node.Arguments, node.Token, methodName, instance: obj);

        if (contextSwitch)
        {
            frame.SetObjectContext(oldObjContext);
        }
        else
        {
            frame.ClearFlag(FrameFlags.InObject);
        }

        if (isCtor)
        {
            return Value.CreateObject(obj);
        }

        return result;
    }

    private Value CallStructMethod(MethodCallNode node, StructRef struc)
    {
        var methodName = node.MethodName;
        var frame = CallStack.Peek();
        var kstruct = Context.Structs[struc.Identifier];
        var methods = kstruct.Methods;
        var isCtor = methodName == "new";

        if (isCtor && kstruct.IsAbstract)
            throw new AbstractInstantiationError(node.Token, struc.Identifier);

        // if the struct does not have the method, check the base struct
        if (!methods.ContainsKey(methodName))
        {
            if (string.IsNullOrEmpty(kstruct.BaseStruct))
            {
                // No base struct: non-constructors are undefined; constructors use default.
                if (!isCtor)
                {
                    throw new UnimplementedMethodError(node.Token, struc.Identifier, methodName);
                }

                return ExecuteStructMethod(methods, methodName, frame, node, struc);
            }

            var baseStruct = Context.Structs[kstruct.BaseStruct];
            var baseStructMethods = baseStruct.Methods;

            if (!baseStructMethods.ContainsKey(methodName))
            {
                if (isCtor)
                    return ExecuteStructMethod(methods, methodName, frame, node, struc);
                throw new UnimplementedMethodError(node.Token, struc.Identifier, methodName);
            }

            return ExecuteStructMethod(baseStructMethods, methodName, frame, node, struc);
        }

        return ExecuteStructMethod(methods, methodName, frame, node, struc);
    }

    private Value CallBuiltin(FunctionCallNode node)
    {
        var args = GetMethodCallArguments(node.Arguments);
        return ExecuteBuiltin(node.Token, node.Op, args);
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

    private Value CallFunction(FunctionCallNode node)
    {
        var functionName = node.FunctionName;
        var func = Context.Functions[functionName];
        var typeHints = func.TypeHints;
        var returnTypeHint = func.ReturnTypeHint;
        var defaultParameters = func.DefaultParameters;
        var functionScope = new Scope(func.CapturedScope ?? _globalScope);
        var functionFrame = new StackFrame(functionName, functionScope, node.Token) { IsFunction = true };
        var result = Value.Default;

        // If the function was compiled as VM bytecode, evaluate args and delegate to the VM.
        // The frame is still pushed so the caller's doPop=true path pops it correctly.
        var vm = VM.KiwiVM.Current;
        if (vm != null && func.VMChunk != null)
        {
            // Resolve named/positional/splat args; for variadic functions, also collect
            // overflow varargs and append them so the VM receives the full flat arg list.
            Scope? varScope = string.IsNullOrEmpty(func.VariadicParamName) ? null : new Scope(null);
            var slots = ResolveArguments(func.Parameters, node.Arguments, defaultParameters, node.Token, functionName, func.VariadicParamName, varScope);
            var evaluatedArgs = slots.Select(v => v ?? Value.Default).ToList();
            if (varScope != null && varScope.TryGet(func.VariadicParamName, out var vl) && vl.IsList())
                evaluatedArgs.AddRange(vl.GetList());
            PushFrame(functionFrame); // placeholder so caller's PopFrame is balanced
            return vm.InvokeVMCallable(func, evaluatedArgs, node.Token);
        }

        // Evaluate arguments while caller's frame is still on top of the stack,
        // then push the callee's frame so argument expressions resolve correctly.
        PrepareFunctionCall(func, node, defaultParameters, typeHints, functionFrame);
        PushFrame(functionFrame);

        var decl = func.Decl.Body;
        foreach (var stmt in decl)
        {
            result = Interpret(stmt);

            if (functionFrame.IsFlagSet(FrameFlags.Return))
            {
                result = functionFrame.ReturnValue ?? result;
                break;
            }
        }

        if (!AssertTypeMatch(node.Token, result, returnTypeHint))
        {
            throw new TypeError(node.Token, $"Expected type `{string.Join("|", returnTypeHint.Select(TypeRegistry.GetTypeName))}` for return type of `{functionName}` but received `{TypeRegistry.GetTypeName(result)}`.");
        }

        return result;
    }

    private Value CallFunction(KFunction function, List<ASTNode?> args, Token token, string functionName, string structName = "", InstanceRef? instance = null)
    {
        // For VM-compiled functions the Decl.Body is an empty stub; delegate to the VM.
        // When KiwiVM.Current is null (e.g. a task thread running an AST lambda that calls
        // into a VM-compiled stdlib function such as Channel.send), create a thread-local VM
        // on demand.  The constructor sets KiwiVM.Current, so subsequent calls on the same
        // thread reuse the same VM without allocating again.
        if (function.VMChunk != null)
        {
            var vm = VM.KiwiVM.Current ?? new VM.KiwiVM(this, function.CapturedScope ?? _globalScope);
            var tmpScope = new Scope(_globalScope);
            var slots = ResolveArguments(function.Parameters, args, function.DefaultParameters, token, functionName, function.VariadicParamName, tmpScope);
            var values = slots.Select(s => s ?? Value.Default).ToList();
            return vm.InvokeVMCallable(function, values, token, instance);
        }

        var defaultParameters = function.DefaultParameters;
        // Package functions get an isolated scope (null parent) so their internal
        // local variables cannot walk up into the caller's scope via Assign().
        var capturedForFrame = function.IsPackageFunction ? null : function.CapturedScope;
        var functionFrame = CreateFrame(functionName, token, capturedForFrame);
        functionFrame.StructName = structName;
        functionFrame.IsFunction = true;
        var scope = functionFrame.Scope;
        var typeHints = function.TypeHints;
        var returnTypeHint = function.ReturnTypeHint;

        var result = Value.Default;
        var doPop = false;

        try
        {
            ProcessFunctionParameters(function, args, token, functionName, defaultParameters, scope, typeHints);

            doPop = PushFrame(functionFrame);
            result = ExecuteFunctionBody(function);

            PopFrame();
        }
        catch (KiwiError)
        {
            if (doPop)
            {
                PopFrame();
            }
            throw;
        }

        if (!AssertTypeMatch(token, result, returnTypeHint))
        {
            throw new TypeError(token, $"Expected type `{string.Join("|", returnTypeHint.Select(TypeRegistry.GetTypeName))}` for return type of `{functionName}` but received `{TypeRegistry.GetTypeName(result)}`.");
        }

        return result;
    }

    private void ProcessFunctionParameters(KFunction function, List<ASTNode?> args, Token token, string functionName, HashSet<string> defaultParameters, Scope scope, Dictionary<string, List<int>> typeHints)
    {
        var slots = ResolveArguments(function.Parameters, args, defaultParameters, token, functionName, function.VariadicParamName, scope);
        for (var i = 0; i < function.Parameters.Count; ++i)
        {
            var param = function.Parameters[i];
            var argValue = slots[i]!;
            PrepareFunctionVariables(typeHints, param, ref argValue, token, i, functionName, scope);
        }
    }

    private Value CallLambda(Token token, string lambdaName, List<ASTNode?> args, ref bool doPop)
    {
        var targetLambda = lambdaName;
        var result = Value.Default;

        if (!Context.HasLambda(targetLambda))
        {
            if (Context.HasMappedLambda(targetLambda))
            {
                targetLambda = Context.LambdaTable[targetLambda];
            }
            else if (CallStack.Count > 0 && CallStack.Peek().Scope.TryGet(targetLambda, out Value scopeLambda) && scopeLambda.IsLambda())
            {
                targetLambda = scopeLambda.GetLambda().Identifier;
            }
            else
            {
                throw new CallableError(token, $"Could not find target lambda `{targetLambda}`");
            }
        }

        var func = Context.Lambdas[targetLambda];

        // If this lambda was compiled to VM bytecode, delegate to the VM.
        // The synthetic LambdaNode built at compile time has an empty body,
        // so we must NOT fall through to the AST-walk path below.
        var vm = VM.KiwiVM.Current;
        if (vm != null && func.VMChunk != null)
        {
            doPop = false; // no interpreter frame was pushed; caller must NOT PopFrame()
            return vm.InvokeVMCallable(func, EvaluateCallArgs(args), token);
        }

        var typeHints = func.TypeHints;
        var returnTypeHint = func.ReturnTypeHint;
        var defaultParameters = func.DefaultParameters;

        // Evaluate args in CALLER's scope (before pushing the new frame), so
        // splat expressions and closured variables resolve correctly.
        List<Value> evaluatedArgs = EvaluateCallArgs(args);

        // Push new frame with captured scope as parent (enables closures).
        var scope = new Scope(func.CapturedScope ?? CallStack.Peek().Scope);
        var lambdaFrame = PushFrame(lambdaName, token, scope, true);
        lambdaFrame.IsFunction = true;

        PrepareLambdaCall(func, evaluatedArgs, defaultParameters, token, targetLambda, typeHints, lambdaName, scope);

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

    private Value CallPackageMethod(MethodCallNode node, PackageRef pkg)
    {
        var qualifiedName = pkg.Identifier + "::" + node.MethodName;

        if (Context.Functions.TryGetValue(qualifiedName, out KFunction? func))
        {
            if (func.IsGenerator)
            {
                return CreateGenerator(func, node.Arguments, node.Token);
            }

            // CallFunction(KFunction, ...) manages push/pop internally.
            return CallFunction(func, node.Arguments, node.Token, qualifiedName);
        }

        throw new FunctionUndefinedError(node.Token, node.MethodName);
    }

    private Value CallObjectBaseMethod(MethodCallNode node, InstanceRef obj, string baseStruct, string methodName)
    {
        var struc = Context.Structs[baseStruct];
        struc.Methods.TryGetValue(methodName, out KFunction? function);
        var isCtor = methodName == "new";

        var frame = CallStack.Peek();
        var objContext = obj;
        var contextSwitch = false;

        if (frame.InObjectContext())
        {
            objContext = frame.GetObjectContext();
            contextSwitch = true;
        }

        frame.SetObjectContext(obj);

        if (function == null)
        {
            // check if it's a builtin.
            if (CoreBuiltin.Map.TryGetValue(methodName, out TokenName builtin))
            {
                return HandleObjectBuiltin(node, obj, baseStruct, builtin);
            }

            throw new UnimplementedMethodError(node.Token, obj.StructName, methodName);
        }
        else
        {
            if (function.IsPrivate && !(contextSwitch && objContext?.StructName == obj.StructName))
            {
                throw new InvalidContextError(node.Token, "Cannot invoke private method outside of struct.");
            }

            var result = CallFunction(function, node.Arguments, node.Token, methodName, instance: obj);

            if (contextSwitch)
            {
                frame.SetObjectContext(objContext);
            }
            else
            {
                frame.ClearFlag(FrameFlags.InObject);
            }

            if (isCtor)
            {
                return Value.CreateObject(obj);
            }

            return result;
        }
    }

    private void PrepareFunctionCall(KFunction func, FunctionCallNode node, HashSet<string> defaultParameters, Dictionary<string, List<int>> typeHints, StackFrame functionFrame)
    {
        var parms = func.Parameters;
        var nodeArguments = node.Arguments;
        var scope = functionFrame.Scope;
        var slots = ResolveArguments(parms, nodeArguments, defaultParameters, node.Token, node.FunctionName, func.VariadicParamName, scope);

        for (var i = 0; i < parms.Count; ++i)
        {
            var param = parms[i];
            var argValue = slots[i]!;

            if (typeHints.TryGetValue(param.Key, out List<int>? expectedTypes) && !AssertTypeMatch(node.Token, argValue, expectedTypes))
            {
                throw new TypeError(node.Token, $"Expected type `{string.Join("|", expectedTypes.Select(TypeRegistry.GetTypeName))}` for parameter {1 + i} of `{node.FunctionName}` but received `{TypeRegistry.GetTypeName(argValue)}`.");
            }

            scope.Declare(param.Key, argValue);
        }
    }

    private static int FindParameterIndexByName(List<KeyValuePair<string, Value>> parms, string name)
    {
        for (int i = 0; i < parms.Count; i++)
            if (parms[i].Key == name) return i;
        return -1;
    }

    private Value[] ResolveArguments(
        List<KeyValuePair<string, Value>> parms,
        List<ASTNode?> nodeArguments,
        HashSet<string> defaultParameters,
        Token token, string callableName,
        string variadicParamName = "",
        Scope? variadicScope = null)
    {
        var slots = new Value?[parms.Count];
        List<Value>? varargs = string.IsNullOrEmpty(variadicParamName) ? null : [];

        // 1. Fill named args
        foreach (var arg in nodeArguments.OfType<NamedArgumentNode>())
        {
            int idx = FindParameterIndexByName(parms, arg.Name);
            if (idx < 0)
                throw new TypeError(token, $"Unknown parameter `{arg.Name}` in call to `{callableName}`.");
            if (slots[idx] != null)
                throw new TypeError(token, $"Parameter `{arg.Name}` specified more than once in call to `{callableName}`.");
            slots[idx] = Interpret(arg.Value);
        }

        // 2. Fill positional args into unfilled slots left-to-right; overflow into varargs
        int pos = 0;
        foreach (var arg in nodeArguments.Where(a => a is not NamedArgumentNode))
        {
            // Splat: expand a list into positional args
            IEnumerable<Value> expanded;
            if (arg is SplatNode splat)
            {
                var listVal = Interpret(splat.Expression);
                expanded = listVal.IsList() ? listVal.GetList() : [listVal];
            }
            else
            {
                expanded = [Interpret(arg)];
            }

            foreach (var v in expanded)
            {
                while (pos < slots.Length && slots[pos] != null) pos++;
                if (pos >= slots.Length)
                {
                    if (varargs == null)
                        throw new ParameterCountMismatchError(token, callableName, parms.Count, nodeArguments.Count);
                    varargs.Add(v);
                    continue;
                }
                slots[pos] = v;
                pos++;
            }
        }

        // 3. Fill remaining slots with defaults; error on missing
        for (int i = 0; i < parms.Count; i++)
        {
            if (slots[i] == null)
            {
                if (defaultParameters.Contains(parms[i].Key))
                    slots[i] = parms[i].Value;
                else
                    throw new ParameterCountMismatchError(token, callableName, parms.Count, nodeArguments.Count);
            }
        }

        // 4. Bind varargs list if variadic
        if (varargs != null && variadicScope != null)
        {
            variadicScope.Declare(variadicParamName, Value.CreateList(varargs));
        }

        return slots!;
    }

    private void PrepareFunctionVariables(Dictionary<string, List<int>> typeHints, KeyValuePair<string, Value> param, ref Value argValue, Token token, int i, string functionName, Scope scope)
    {
        if (typeHints.TryGetValue(param.Key, out List<int>? expectedTypes) && !AssertTypeMatch(token, argValue, expectedTypes))
        {
            throw new TypeError(token, $"Expected type `{string.Join("|", expectedTypes.Select(TypeRegistry.GetTypeName))}` for parameter {1 + i} of `{functionName}` but received `{TypeRegistry.GetTypeName(argValue)}`.");
        }

        scope.Declare(param.Key, argValue);
    }

    private void PrepareLambdaCall(KLambda func, List<ASTNode?> args, HashSet<string> defaultParameters, Token token, string targetLambda, Dictionary<string, List<int>> typeHints, string lambdaName, Scope scope)
    {
        var parms = func.Parameters;
        var slots = ResolveArguments(parms, args, defaultParameters, token, targetLambda, func.VariadicParamName, scope);
        for (var i = 0; i < parms.Count; ++i)
        {
            var param = parms[i];
            var argValue = slots[i]!;
            PrepareLambdaVariables(typeHints, param, ref argValue, token, i, lambdaName, scope);
        }
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

    private Value ExecuteFunctionBody(KFunction function)
    {
        var result = Value.Default;
        var decl = function.Decl;

        foreach (var stmt in decl.Body)
        {
            result = Interpret(stmt);

            if (CallStack.Peek().IsFlagSet(FrameFlags.Return))
            {
                result = CallStack.Peek().ReturnValue ?? result;
                break;
            }
        }

        return result;
    }

    private Value ExecuteInstanceMethod(FunctionCallNode node, ref bool doPop)
    {
        var frame = CallStack.Peek();
        if (!frame.InObjectContext())
        {
            throw new InvalidContextError(node.Token);
        }

        var obj = frame.GetObjectContext() ?? throw new NullObjectError(node.Token);
        var struc = Context.Structs[obj.StructName];
        var strucMethods = struc.Methods;
        var functionName = node.FunctionName;

        if (!strucMethods.ContainsKey(functionName))
        {
            // check the base
            if (string.IsNullOrEmpty(struc.BaseStruct))
            {
                throw new UnimplementedMethodError(node.Token, struc.Name, functionName);
            }

            var baseStruct = Context.Structs[struc.BaseStruct];
            var baseStructMethods = baseStruct.Methods;

            if (!baseStructMethods.ContainsKey(functionName))
            {
                throw new UnimplementedMethodError(node.Token, struc.Name, functionName);
            }

            return ExecuteInstanceMethodFunction(baseStructMethods, node, ref doPop);
        }

        return ExecuteInstanceMethodFunction(strucMethods, node, ref doPop);
    }

    private Value ExecuteInstanceMethodFunction(Dictionary<string, KFunction> strucMethods, FunctionCallNode node, ref bool doPop)
    {
        var functionName = node.FunctionName;
        var func = strucMethods[functionName];
        var defaultParameters = func.DefaultParameters;
        var functionFrame = CreateFrame(functionName, node.Token, func.CapturedScope);
        functionFrame.IsFunction = true;
        var result = Value.Default;

        var typeHints = func.TypeHints;
        var returnTypeHint = func.ReturnTypeHint;

        PrepareFunctionCall(func, node, defaultParameters, typeHints, functionFrame);

        doPop = PushFrame(functionFrame);

        var decl = func.Decl.Body;
        foreach (var stmt in decl)
        {
            result = Interpret(stmt);
            if (functionFrame.IsFlagSet(FrameFlags.Return))
            {
                result = functionFrame.ReturnValue ?? result;
                break;
            }
        }

        if (!AssertTypeMatch(node.Token, result, returnTypeHint))
        {
            throw new TypeError(node.Token, $"Expected type `{string.Join("|", returnTypeHint.Select(TypeRegistry.GetTypeName))}` for return type of `{functionName}` but received `{TypeRegistry.GetTypeName(result)}`.");
        }

        return result;
    }

    private Value ExecuteStructMethod(Dictionary<string, KFunction> methods, string methodName, StackFrame frame, MethodCallNode node, StructRef struc)
    {
        methods.TryGetValue(methodName, out KFunction? function);
        InstanceRef obj = new();
        var isCtor = methodName == "new";

        var oldObjectContext = frame.GetObjectContext();
        var contextSwitch = false;

        if (function == null && isCtor)
        {
            obj.StructName = struc.Identifier;
            // default constructor
            return Value.CreateObject(obj);
        }

        if (function != null && !function.IsStatic && !isCtor)
        {
            throw new InvalidContextError(node.Token, "Cannot invoke non-static method on struct.");
        }

        // Evaluate ctor arguments in the current context before switching object context,
        // so that @instanceVar references in ctor args resolve to the calling instance.
        List<ASTNode?> ctorArgs = node.Arguments;
        if (isCtor)
        {
            ctorArgs = [.. GetMethodCallArguments(node.Arguments).Select(v => (ASTNode?)new LiteralNode(v))];

            if (frame.InObjectContext())
            {
                contextSwitch = true;
            }

            obj.StructName = struc.Identifier;
            frame.SetObjectContext(obj);
        }

        if (function == null)
        {
            // TODO: Need a new error/message.
            throw new InvalidContextError(node.Token, "Invalid function context.");
        }

        var result = CallFunction(function, ctorArgs, node.Token, methodName, struc.Identifier, isCtor ? obj : null);

        if (isCtor)
        {
            if (contextSwitch)
            {
                frame.SetObjectContext(oldObjectContext);
            }
            else
            {
                frame.ClearFlag(FrameFlags.InObject);
            }
            return Value.CreateObject(obj);
        }

        return result;
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
        {
            throw new InvalidOperationError(token, "Expected the name of a package to import.");
        }

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

        foreach (var stmt in decl.Body)
        {
            Interpret(stmt);
        }

        if (PackageStack.Count > 0)
        {
            PackageStack.Pop();
        }

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

        Parser p = new(true);
        var tokenStream = lexer.GetTokenStream();
        var ast = p.ParseTokenStream(tokenStream, true);

        Interpret(ast);
    }
    private Value BytesLoop(ForLoopNode node, byte[] list)
    {
        var frame = CallStack.Peek();
        var parent = EnterBlockScope(frame);
        var scope = frame.Scope;
        frame.SetFlag(FrameFlags.InLoop);

        var valueName = Id(node.ValueIterator);
        string? indexName = null;
        if (node.IndexIterator != null)
        {
            indexName = Id(node.IndexIterator);
        }

        // Declare iterators in scope
        scope.Declare(valueName, Value.Default);
        if (indexName != null)
        {
            scope.Declare(indexName, Value.Default);
        }

        var result = Value.Default;
        var fallOut = false;

        try
        {
            for (int i = 0; i < list.Length; i++)
            {
                if (fallOut)
                {
                    break;
                }

                // Update iterators
                scope.SetLocal(valueName, Value.CreateInteger(list[i]));
                if (indexName != null)
                {
                    scope.SetLocal(indexName, Value.CreateInteger(i));
                }

                var skip = false;

                foreach (var stmt in node.Body)
                {
                    if (skip)
                    {
                        break;
                    }

                    if (stmt == null)
                    {
                        continue;
                    }

                    ASTNodeType statement = stmt.Type;
                    if (statement != ASTNodeType.Next && statement != ASTNodeType.Break)
                    {
                        result = Interpret(stmt);

                        if (frame.IsFlagSet(FrameFlags.Break))
                        {
                            frame.ClearFlag(FrameFlags.Break);
                            fallOut = true;
                            break;
                        }

                        if (frame.IsFlagSet(FrameFlags.Next))
                        {
                            frame.ClearFlag(FrameFlags.Next);
                            skip = true;
                            break;
                        }
                    }

                    if (frame.IsFlagSet(FrameFlags.Return))
                    {
                        return frame.ReturnValue ?? result;
                    }

                    if (statement == ASTNodeType.Next)
                    {
                        var condition = ((NextNode)stmt).Condition;
                        if (condition == null || BooleanOp.IsTruthy(Interpret(condition)))
                        {
                            skip = true;
                            break;
                        }
                    }
                    else if (statement == ASTNodeType.Break)
                    {
                        var condition = ((BreakNode)stmt).Condition;
                        if (condition == null || BooleanOp.IsTruthy(Interpret(condition)))
                        {
                            fallOut = true;
                            break;
                        }
                    }
                }
            }
        }
        finally
        {
            frame.ClearFlag(FrameFlags.InLoop);
            ExitBlockScope(frame, parent);
        }

        return result;
    }

    private Value ListLoop(ForLoopNode node, List<Value> list)
    {
        var frame = CallStack.Peek();
        var parent = EnterBlockScope(frame);
        var scope = frame.Scope;
        frame.SetFlag(FrameFlags.InLoop);

        var valueName = Id(node.ValueIterator);
        string? indexName = null;
        if (node.IndexIterator != null)
        {
            indexName = Id(node.IndexIterator);
        }

        // Declare iterators in scope
        scope.Declare(valueName, Value.Default);
        if (indexName != null)
        {
            scope.Declare(indexName, Value.Default);
        }

        var result = Value.Default;
        var fallOut = false;

        try
        {
            for (int i = 0; i < list.Count; i++)
            {
                if (fallOut)
                {
                    break;
                }

                // Update iterators
                scope.SetLocal(valueName, list[i]);
                if (indexName != null)
                {
                    scope.SetLocal(indexName, Value.CreateInteger(i));
                }

                var skip = false;

                foreach (var stmt in node.Body)
                {
                    if (skip)
                    {
                        break;
                    }

                    if (stmt == null)
                    {
                        continue;
                    }

                    ASTNodeType statement = stmt.Type;
                    if (statement != ASTNodeType.Next && statement != ASTNodeType.Break)
                    {
                        result = Interpret(stmt);

                        if (frame.IsFlagSet(FrameFlags.Break))
                        {
                            frame.ClearFlag(FrameFlags.Break);
                            fallOut = true;
                            break;
                        }

                        if (frame.IsFlagSet(FrameFlags.Next))
                        {
                            frame.ClearFlag(FrameFlags.Next);
                            skip = true;
                            break;
                        }
                    }

                    if (frame.IsFlagSet(FrameFlags.Return))
                    {
                        return frame.ReturnValue ?? result;
                    }

                    if (statement == ASTNodeType.Next)
                    {
                        var condition = ((NextNode)stmt).Condition;
                        if (condition == null || BooleanOp.IsTruthy(Interpret(condition)))
                        {
                            skip = true;
                            break;
                        }
                    }
                    else if (statement == ASTNodeType.Break)
                    {
                        var condition = ((BreakNode)stmt).Condition;
                        if (condition == null || BooleanOp.IsTruthy(Interpret(condition)))
                        {
                            fallOut = true;
                            break;
                        }
                    }
                }
            }
        }
        finally
        {
            frame.ClearFlag(FrameFlags.InLoop);
            ExitBlockScope(frame, parent);
        }

        return result;
    }

    private Value HashmapLoop(ForLoopNode node, Dictionary<Value, Value> hash)
    {
        var frame = CallStack.Peek();
        var parent = EnterBlockScope(frame);
        var scope = frame.Scope;
        frame.SetFlag(FrameFlags.InLoop);

        var keyName = Id(node.ValueIterator);
        string? valueName = null;
        if (node.IndexIterator != null)
        {
            valueName = Id(node.IndexIterator);
        }

        scope.Declare(keyName, Value.Default);
        if (valueName != null)
        {
            scope.Declare(valueName, Value.Default);
        }

        var result = Value.Default;
        var fallOut = false;

        try
        {
            foreach (var kvp in hash)
            {
                if (fallOut)
                {
                    break;
                }

                scope.SetLocal(keyName, kvp.Key);
                if (valueName != null)
                {
                    scope.SetLocal(valueName, kvp.Value);
                }

                var skip = false;

                foreach (var stmt in node.Body)
                {
                    if (skip)
                    {
                        break;
                    }

                    if (stmt == null)
                    {
                        continue;
                    }

                    if (stmt.Type != ASTNodeType.Next && stmt.Type != ASTNodeType.Break)
                    {
                        result = Interpret(stmt);

                        if (frame.IsFlagSet(FrameFlags.Break))
                        {
                            frame.ClearFlag(FrameFlags.Break);
                            fallOut = true;
                            break;
                        }

                        if (frame.IsFlagSet(FrameFlags.Next))
                        {
                            frame.ClearFlag(FrameFlags.Next);
                            skip = true;
                            break;
                        }
                    }

                    if (frame.IsFlagSet(FrameFlags.Return))
                    {
                        return frame.ReturnValue ?? result;
                    }

                    if (stmt.Type == ASTNodeType.Next)
                    {
                        var condition = ((NextNode)stmt).Condition;
                        if (condition == null || BooleanOp.IsTruthy(Interpret(condition)))
                        {
                            skip = true;
                            break;
                        }
                    }
                    else if (stmt.Type == ASTNodeType.Break)
                    {
                        var condition = ((BreakNode)stmt).Condition;
                        if (condition == null || BooleanOp.IsTruthy(Interpret(condition)))
                        {
                            fallOut = true;
                            break;
                        }
                    }
                }
            }
        }
        finally
        {
            frame.ClearFlag(FrameFlags.InLoop);
            ExitBlockScope(frame, parent);
        }

        return result;
    }

    private Value HandleObjectBuiltin(MethodCallNode node, InstanceRef obj, string baseStruct, TokenName builtin)
        => _builtinDispatcher.HandleObjectBuiltin(node, obj, baseStruct, builtin);

    private Value HandleCallableBuiltin(MethodCallNode node, List<Value> args)
        => _builtinDispatcher.HandleCallableBuiltin(node, args);

    private static bool RequiresSigCheck(ASTNodeType type)
    {
        return type is ASTNodeType.Program
            or ASTNodeType.Export
            or ASTNodeType.Try or ASTNodeType.If or ASTNodeType.Case or
            ASTNodeType.ForLoop or ASTNodeType.WhileLoop or ASTNodeType.RepeatLoop or
            ASTNodeType.LambdaCall or ASTNodeType.FunctionCall or ASTNodeType.MethodCall;
    }

    private static Value PrintNode(ASTNode node)
    {
        node.Print();
        return Value.Default;
    }

    private static Value DoSliceAssignment(Token token, ref Value slicedObj, SliceIndex slice, ref Value newValue)
    {
        if (slicedObj.IsList() && newValue.IsList())
        {
            var targetList = slicedObj.GetList();
            var rhsValues = newValue.GetList();
            SliceUtil.UpdateListSlice(token, false, ref targetList, slice, rhsValues);

            return Value.CreateList(targetList);
        }

        return slicedObj;
    }

    private static Value Visit(LiteralNode node) => node.Value;

    private Value Visit(InterpolationNode node)
    {
        var sb = new System.Text.StringBuilder();
        foreach (var part in node.Parts)
        {
            var value = Interpret(part);
            sb.Append(value.IsString() ? value.GetString() : ConversionOp.ToString(value));
        }
        return Value.CreateString(sb.ToString());
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
    public Value InterpretNode(ASTNode node)
    {
        // When called from the VM, the interpreter's CallStack may be empty.
        // EntryPoint sets up a proper global-scope frame for the execution.
        if (CallStack.Count == 0)
            return EntryPoint(node);
        return Interpret(node);
    }

    /// <summary>
    /// Execute a single AST node with VM-local variables injected into a temporary
    /// interpreter scope so that expressions inside the node (e.g. argument sub-expressions
    /// in named-arg calls) can resolve VM locals by name.
    /// After execution, any variables that were modified in the interpreter scope are
    /// written back into <paramref name="locals"/> so the VM can update its stack.
    /// </summary>
    public Value InterpretNodeWithLocals(ASTNode node, Dictionary<string, Value> locals, InstanceRef? self = null)
    {
        var scope = new Scope(_globalScope);
        foreach (var kv in locals)
            scope.Declare(kv.Key, kv.Value);

        var frame = new StackFrame("<vm-fallback>", scope);
        if (self != null) frame.SetObjectContext(self);
        CallStack.Push(frame);
        try
        {
            return Interpret(node);
        }
        finally
        {
            CallStack.Pop();
            // Sync any written-back values to the caller's dictionary.
            foreach (var kv in locals.Keys.ToList())
            {
                if (scope.TryGet(kv, out var updated))
                    locals[kv] = updated;
            }
        }
    }

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
        return InvokeCallable(fn, args, token, methodName);
    }

    private Value HandleCallableBuiltinDirect(Token token, Value obj, string methodName, List<Value> args)
        => _builtinDispatcher.HandleCallableBuiltinDirect(token, obj, methodName, args);

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