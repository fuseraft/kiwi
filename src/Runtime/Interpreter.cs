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
    const int SafemodeMaxIterations = 1000000;
    private readonly Scope _globalScope = new();

    public Interpreter()
    {
        Current = this;
    }

    public static Interpreter? Current { get; private set; }
    public Dictionary<string, string> CliArgs { get; set; } = [];
    public KContext Context { get; private set; } = new();
    public string ExecutionPath { get; set; } = string.Empty;
    private Stack<StackFrame> CallStack { get; set; } = [];
    private Stack<string> PackageStack { get; set; } = [];
    private Stack<string> StructStack { get; set; } = [];
    private Stack<string> FuncStack { get; set; } = [];
    public long CurrentTaskId { get; set; } = 0; // 0 = main thread
    public void SetContext(KContext context) => Context = context;

    public Value Interpret(ASTNode? node)
    {
        if (node == null)
        {
            return Value.Default;
        }

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
            ASTNodeType.Import => Visit((ImportNode)node),
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
            ASTNodeType.Return => Visit((ReturnNode)node),
            ASTNodeType.Self => Visit((SelfNode)node),
            ASTNodeType.Slice => Visit((SliceNode)node),
            ASTNodeType.Struct => Visit((StructNode)node),
            ASTNodeType.TernaryOperation => Visit((TernaryOperationNode)node),
            ASTNodeType.Throw => Visit((ThrowNode)node),
            ASTNodeType.Try => Visit((TryNode)node),
            ASTNodeType.UnaryOperation => Visit((UnaryOperationNode)node),
            ASTNodeType.Variable => Visit((VariableNode)node),
            ASTNodeType.WhileLoop => Visit((WhileLoopNode)node),
            _ => PrintNode(node),
        };

        return result;
    }

    public Value FuncToLambda(KFunction func)
    {
        return Visit(func.Decl.ToLambda(), false);
    }

    public Value InvokeCallable(Callable callable, List<Value> args, Token token, string displayName, InstanceRef? instance = null)
    {
        var scope = new Scope(callable.CapturedScope ?? _globalScope);
        var frame = new StackFrame(displayName, scope);

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
                    $"Expected `{TypeRegistry.GetTypeName(callable.ReturnTypeHint)}` " +
                    $"from `{displayName}` but got `{TypeRegistry.GetTypeName(result)}`.");
            }

            return result;
        }
        finally
        {
            PopFrame();
        }
    }

    public Value InvokeEvent(Token token, LambdaRef lambda, List<Value> args)
    {
        var doPop = false;

        try
        {
            var lambdaName = lambda.Identifier;
            var scope = CallStack.Count > 0 ? new Scope(CallStack.Peek().Scope) : new Scope();
            var lambdaFrame = PushFrame(lambdaName, scope, true);
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
                throw new TypeError(token, $"Expected type `{TypeRegistry.GetTypeName(returnTypeHint)}` for return type of `{lambdaName}` but received `{TypeRegistry.GetTypeName(result)}`.");
            }

            return result;
        }
        catch (Exception ex)
        {
            ErrorHandler.DumpCrashLog(ex);
            throw;
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

        Context.Events.On(eventName.GetString(), callback);

        return Value.Default;
    }

    private Value Visit(OnceNode node)
    {
        var eventName = Interpret(node.EventName);
        var callback = Interpret(node.Callback);

        Context.Events.Once(eventName.GetString(), callback);

        return Value.Default;
    }

    private Value Visit(EmitNode node)
    {
        var eventName = Interpret(node.EventName);
        var args = node.EventArgs.Select(Interpret).ToList();

        Context.Events.Emit(node.Token, eventName.GetString(), args);

        return Value.Default;
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
            PushFrame("kiwi", _globalScope);

            // Add the "global" hashmap.
            _globalScope.Declare("global", Value.CreateHashmap());
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

    private Value Visit(StructNode node)
    {
        var structName = node.Name;
        KStruct struc = new()
        {
            Name = structName
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

        foreach (var method in node.Methods)
        {
            if (method == null)
            {
                continue;
            }

            var funcDecl = (FunctionNode)method;
            var methodName = funcDecl.Name;

            Visit(funcDecl);

            if (methodName == "new")
            {
                struc.Methods["new"] = Context.Methods[methodName];
            }
            else
            {
                struc.Methods[methodName] = Context.Methods[methodName];
            }
        }

        Context.Structs[structName] = struc;
        StructStack.Pop();
        Context.Methods.Clear();

        return Value.Default;
    }

    private Value Visit(PackageNode node)
    {
        var packageName = Id(node.PackageName ?? throw new PackageUndefinedError(node.Token, string.Empty));
        Context.Packages[packageName] = new KPackage(node.Clone());

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
        var parentPath = FileUtil.GetParentPath(node.Token, ExecutionPath);
        var fullPath = Path.GetFullPath(Path.Combine(parentPath, filePath));

        if (!Path.HasExtension(fullPath))
        {
            fullPath = FileUtil.TryGetExtensionless(node.Token, fullPath);
        }

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
        var result = Interpret(ast);

        ExecutionPath = oldExecutionPath;

        return Value.Default;
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

            throw new KiwiError(node.Token, errorType, errorMessage);
        }

        return Value.Default;
    }

    private Value Visit(AssignmentNode node)
    {
        string Global = "global";

        var frame = CallStack.Peek();
        var scope = frame.Scope;
        var value = Interpret(node.Initializer).Clone();
        var type = node.Op;
        var name = node.Name;

        if (node.Condition != null)
        {
            var eval = Interpret(node.Condition);

            if (!BooleanOp.IsTruthy(eval))
            {
                return Value.Default;
            }
        }

        if (type == TokenName.Ops_Assign)
        {
            if (Global.Equals(name) || Context.HasConstant(name))
            {
                throw new IllegalNameError(node.Token, name);
            }

            if (value.IsLambda())
            {
                // WIP: need to work on this.
                var lambdaId = value.GetLambda().Identifier;
                Context.Lambdas.Add(name, Context.Lambdas[lambdaId]);
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

        return scope.GetBinding(name);
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

            if (indexExpr.IndexedObject.Type == ASTNodeType.Identifier)
            {
                var identifierName = Id(indexExpr.IndexedObject);
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
                throw new VariableUndefinedError(node.Token, varName);
            }

            ApplyMemberAssignment(node.Token, ref container, key, node.Op, ref newValue);
            scope.Assign(varName, container);
        }
        else
        {
            var container = Interpret(target);
            ApplyMemberAssignment(node.Token, ref container, key, node.Op, ref newValue);
        }

        return Value.Default;
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

        if (obj.IsHashmap())
        {
            var hash = obj.GetHashmap();
            var memberKey = Value.CreateString(memberName);

            if (!hash.TryGetValue(memberKey, out Value? memberValue))
            {
                return Value.CreateNull();
            }

            return memberValue;
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
        else if (Context.HasStruct(name))
        {
            return Value.CreateStruct(new StructRef { Identifier = name });
        }
        else if (Context.HasLambda(name))
        {
            return Value.CreateLambda(new LambdaRef { Identifier = name });
        }
        else if (Context.HasMappedLambda(name))
        {
            var id = Context.LambdaTable[name];

            if (Context.HasLambda(id))
            {
                return Value.CreateLambda(new LambdaRef { Identifier = id });
            }
        }
        else if (Context.HasConstant(name))
        {
            return Context.Constants[name];
        }
        else if (_globalScope.TryGet(node.Name, out val))
        {
            return val;
        }

        return Value.CreateNull();
    }

    private bool AssertTypeMatch(Token t, Value v, int type)
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
            if (callable.TypeHints.TryGetValue(param.Key, out var hint) &&
                !AssertTypeMatch(token, argValue, hint))
            {
                throw new TypeError(token,
                    $"Parameter {i + 1} of `{name}` expected `{TypeRegistry.GetTypeName(hint)}`, " +
                    $"got `{TypeRegistry.GetTypeName(argValue)}`.");
            }

            // Lambda mapping
            if (argValue.IsLambda())
            {
                Context.AddMappedLambda(param.Key, argValue.GetLambda().Identifier);
            }

            scope.Declare(param.Key, argValue);
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

    private Value Visit(PrintXyNode node) => throw new NotImplementedException();

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
            foreach (var stmt in node.Body)
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

    private Value Visit(IfNode node)
    {
        var conditionValue = Interpret(node.Condition);
        var frame = CallStack.Peek();
        var result = Value.Default;

        if (BooleanOp.IsTruthy(conditionValue))
        {
            foreach (var stmt in node.Body)
            {
                result = Interpret(stmt);
                if (frame.IsFlagSet(FrameFlags.Return))
                {
                    return frame.ReturnValue ?? result;
                }
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

                    foreach (var stmt in elsifNode.Body)
                    {
                        result = Interpret(stmt);
                        if (frame.IsFlagSet(FrameFlags.Return))
                        {
                            return frame.ReturnValue ?? result;
                        }
                    }
                    executed = true;
                    break;
                }
            }

            if (!executed && node.ElseBody.Count > 0)
            {
                foreach (var stmt in node.ElseBody)
                {
                    result = Interpret(stmt);
                    if (frame.IsFlagSet(FrameFlags.Return))
                    {
                        return frame.ReturnValue ?? result;
                    }
                }
            }
        }

        return result;
    }

    private Value Visit(CaseWhenNode node)
    {
        var testValue = Interpret(node.Condition);
        return testValue;
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
            var whenCondition = Interpret(whenNode);

            if ((isSwitch && BooleanOp.IsTruthy(whenCondition))
                || (!isSwitch && ComparisonOp.Equal(ref testValue, ref whenCondition)))
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

        throw new InvalidOperationError(node.Token, "Expected a list value in for-loop.");
    }

    private Value Visit(WhileLoopNode node)
    {
        var result = Value.Default;
        var frame = CallStack.Peek();
        frame.SetFlag(FrameFlags.InLoop);

        var fallOut = false;
        var iterations = 0;

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

        frame.ClearFlag(FrameFlags.InLoop);

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
                    scope.Assign(aliasName, Value.CreateInteger(i));
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
            if (aliasName != null)
            {
                scope.Assign(aliasName, Value.Default); // Optional: clean up
            }
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

        try
        {
            var scope = new Scope(CallStack.Peek().Scope);
            var tryFrame = PushFrame(tryName, scope);
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
            // Ensure try frame is cleaned up
            if (CallStack.Count > 0 && CallStack.Peek().Name == tryName)
            {
                PopFrame();
            }

            if (node.CatchBody.Count > 0)
            {
                var catchScope = new Scope(CallStack.Peek().Scope);
                var catchFrame = PushFrame("catch", catchScope);

                if (node.ErrorType != null)
                {
                    var typeName = Id(node.ErrorType);
                    catchScope.Declare(typeName, Value.CreateString(e.Type));
                }
                if (node.ErrorMessage != null)
                {
                    var msgName = Id(node.ErrorMessage);
                    catchScope.Declare(msgName, Value.CreateString(e.Message));
                }

                result = ExecuteBody(node.CatchBody, catchFrame);
                if (catchFrame.IsFlagSet(FrameFlags.Return))
                {
                    result = catchFrame.ReturnValue ?? result;
                    setReturnValue = true;
                }
                PopFrame();
            }
            else if (node.FinallyBody.Count == 0)
            {
                throw; // Re-throw if no finally-block
            }
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
        Context.Lambdas[internalName] = new KLambda(node)
        {
            Parameters = parameters,
            DefaultParameters = defaultParameters,
            TypeHints = node.TypeHints,
            ReturnTypeHint = node.ReturnTypeHint,
            CapturedScope = CallStack.Peek().Scope
        };

        if (map)
        {
            Context.AddMappedLambda(node.Token.Text, internalName);
        }

        return Value.CreateLambda(new LambdaRef { Identifier = internalName });
    }

    private Value Visit(FunctionNode node)
    {
        var name = node.Name;

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

        if (StructStack.Count > 0)
        {
            Context.Methods[name] = CreateFunction(node, name);
        }
        else
        {
            Context.Functions[name] = CreateFunction(node, name);
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
                value = Interpret(pair.Value);

                // if the value is a lambda, register to the lambda map
                if (value.IsLambda())
                {
                    var lambdaName = value.GetLambda().Identifier;
                    Context.AddMappedLambda(name, lambdaName);
                }
            }

            // check for a type-hint
            if (typeHints.TryGetValue(name, out int expectedType))
            {
                // if a default value was supplied, expect it to match the type
                if (hasDefaultValue && !AssertTypeMatch(node.Token, value, expectedType))
                {
                    throw new TypeError(node.Token, $"Expected type `{TypeRegistry.GetTypeName(expectedType)}` for variable  `{ASTTracer.Unmangle(name)}` but received `{TypeRegistry.GetTypeName(value)}`.");
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
        var lambdaName = nodeValue.GetLambda().Identifier;
        var result = Value.Default;
        var doPop = false;

        try
        {
            result = CallLambda(node.Token, lambdaName, node.Arguments, ref doPop);
            PopFrame();
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
                    result = CallFunction(node);
                    doPop = true;
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

        if (obj.IsObject())
        {
            return CallObjectMethod(node, obj.GetObject());
        }
        else if (obj.IsStruct())
        {
            return CallStructMethod(node, obj.GetStruct());
        }
        else if (ListBuiltin.IsBuiltin(node.Op))
        {
            return InterpretListBuiltin(node.Token, ref obj, node.Op, GetMethodCallArguments(node.Arguments));
        }
        else if (CallableBuiltin.IsBuiltin(node.Op))
        {
            return InterpretCallableBuiltin(node, GetMethodCallArguments(node.Arguments));
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

    private Value Visit(IndexingNode node)
    {
        if (node.IndexedObject == null)
        {
            throw new InvalidOperationError(node.Token, "Nothing to index.");
        }

        var obj = Interpret(node.IndexedObject);
        var indexValue = Interpret(node.IndexExpression);

        if (node.IndexExpression.Type == ASTNodeType.Index)
        {
            var indexExpr = (IndexingNode)node.IndexExpression;
            return HandleNestedIndexing(indexExpr, obj, TokenName.Ops_Assign, Value.CreateInteger(0L));
        }
        else
        {
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

                return Value.CreateString(str[(int)index].ToString());
            }

            throw new IndexError(node.Token, "Invalid indexing operation.");
        }
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

                if (typeHints.TryGetValue(paramName, out int expectedType))
                {
                    if (!AssertTypeMatch(node.Token, paramValue, expectedType))
                    {
                        throw new TypeError(node.Token, $"Expected type `{TypeRegistry.GetTypeName(expectedType)}` for parameter {paramCount} of `{name}` but received `{TypeRegistry.GetTypeName(paramValue)}`.");
                    }
                }

                defaultParameters.Add(paramName);
            }

            parameters.Add(new(paramName, paramValue));
        }

        return new KFunction(node)
        {
            Name = node.Name,
            Parameters = parameters,
            DefaultParameters = defaultParameters,
            IsPrivate = node.IsPrivate,
            IsStatic = node.IsStatic,
            IsCtor = node.Name == "new",
            TypeHints = node.TypeHints,
            ReturnTypeHint = node.ReturnTypeHint,
            CapturedScope = CallStack.Peek().Scope
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
        else if (indexExpr.IndexExpression.Type == ASTNodeType.Identifier && baseObj.IsHashmap())
        {
            var key = Id(indexExpr.IndexExpression);
            var keyString = Value.CreateString(key);
            var hashObj = baseObj.GetHashmap();

            // if (!hashObj.TryGetValue(keyString, out Value? nestedValue))
            // {
            //     throw new HashKeyError(indexExpr.Token, key);
            // }

            if (op == TokenName.Ops_Assign)
            {
                hashObj[keyString] = newValue;
            }
            else
            {
                var oldValue = hashObj[keyString];
                hashObj[keyString] = OpDispatch.DoBinary(indexExpr.Token, op, ref oldValue, ref newValue);
            }

            return Value.CreateHashmap(hashObj);
        }
        else if (indexExpr.IndexExpression.Type == ASTNodeType.Identifier && baseObj.IsList())
        {
            var identifier = Interpret(indexExpr.IndexExpression);
            var list = baseObj.GetList();
            var listIndex = (int)ConversionOp.GetInteger(indexExpr.Token, identifier);

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
        else if (indexExpr.IndexExpression.Type == ASTNodeType.Literal)
        {
            var literal = Interpret(indexExpr.IndexExpression);

            if (baseObj.IsList() && literal.IsInteger())
            {
                var list = baseObj.GetList();
                var listIndex = (int)ConversionOp.GetInteger(indexExpr.Token, literal);

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
                    hash[literal] = newValue;
                }
                else
                {
                    var oldValue = hash[literal];
                    hash[literal] = OpDispatch.DoBinary(indexExpr.Token, op, ref oldValue, ref newValue);
                }

                return Value.CreateHashmap(hash);
            }
        }

        throw new IndexError(indexExpr.Token, "Invalid index expression.");
    }

    private SliceIndex GetSlice(SliceNode node, Value obj)
    {
        var isSlice = true;
        var indexOrStart = Value.CreateInteger(0L);
        var stopIndex = Value.CreateInteger(0L);
        var stepValue = Value.CreateInteger(0L);

        if (obj.IsList())
        {
            stopIndex.SetValue(obj.GetList().Count);
        }
        else if (obj.IsString())
        {
            stopIndex.SetValue(obj.GetString().Length);
        }
        else if (obj.IsBytes())
        {
            stopIndex.SetValue(obj.GetBytes().Length);
        }

        stepValue.SetValue(1);

        if (node.StartExpression != null)
        {
            indexOrStart.SetValue(Interpret(node.StartExpression));
        }

        if (node.StopExpression != null)
        {
            stopIndex.SetValue(Interpret(node.StopExpression));
        }

        if (node.StepExpression != null)
        {
            stepValue.SetValue(Interpret(node.StepExpression));
        }

        return new(indexOrStart, stopIndex, stepValue)
        {
            IsSlice = isSlice
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
            arguments.Add(Interpret(arg));
        }

        return arguments;
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

        if (function.IsPrivate)
        {
            throw new InvalidContextError(node.Token, "Cannot invoke private method outside of struct.");
        }

        var result = CallFunction(function, node.Arguments, node.Token, methodName);

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

        // if the struct does not have the method, check the base struct
        if (!methods.ContainsKey(methodName))
        {
            // if there is no base, throw if not a constructor
            if (string.IsNullOrEmpty(kstruct.BaseStruct) && !isCtor)
            {
                throw new UnimplementedMethodError(node.Token, struc.Identifier, methodName);
            }

            if (!isCtor)
            {
                var baseStruct = Context.Structs[kstruct.BaseStruct];
                var baseStructMethods = baseStruct.Methods;

                if (!baseStructMethods.ContainsKey(methodName))
                {
                    throw new UnimplementedMethodError(node.Token, struc.Identifier, methodName);
                }

                return ExecuteStructMethod(baseStructMethods, methodName, frame, node, struc);
            }
        }

        return ExecuteStructMethod(methods, methodName, frame, node, struc);
    }

    private Value CallBuiltin(FunctionCallNode node)
    {
        var args = GetMethodCallArguments(node.Arguments);
        var op = node.Op;

        if (KiwiBuiltin.IsBuiltin(op))
        {
            return InterpretKiwiBuiltin(node.Token, op, args);
        }
        else if (ReflectorBuiltin.IsBuiltin(op))
        {
            return ReflectorBuiltinHandler.Execute(node.Token, op, args, Context, CallStack, FuncStack);
        }
        else if (TaskBuiltin.IsBuiltin(op))
        {
            return TaskBuiltinHandler.Execute(node.Token, op, args, Context);
        }
        else if (ChannelBuiltin.IsBuiltin(op))
        {
            return ChannelBuiltinHandler.Execute(node.Token, op, args);
        }
        else if (SocketBuiltin.IsBuiltin(op))
        {
            return SocketBuiltinHandler.Execute(node.Token, op, args);
        }
        
        // TODO: need to create issues for these in GitHub.
        /*
        else if (SignalBuiltin.IsBuiltin(op))
        {
            // return InterpretSignalBuiltin(node.Token, op, args);
        }
        else if (FFIBuiltin.IsBuiltin(op))
        {
            // return BuiltinDispatch.Execute(ffimgr, node.Token, op, args);
        }
        */

        return BuiltinDispatch.Execute(node.Token, op, args, CliArgs);
    }

    private Value CallFunction(FunctionCallNode node)
    {
        var functionName = node.FunctionName;
        var func = Context.Functions[functionName];
        var typeHints = func.TypeHints;
        var returnTypeHint = func.ReturnTypeHint;
        var defaultParameters = func.DefaultParameters;
        var functionScope = new Scope(CallStack.Peek().Scope);
        var functionFrame = PushFrame(functionName, functionScope);
        var result = Value.Default;

        PrepareFunctionCall(func, node, defaultParameters, typeHints, functionFrame);

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
            throw new TypeError(node.Token, $"Expected type `{TypeRegistry.GetTypeName(returnTypeHint)}` for return type of `{functionName}` but received `{TypeRegistry.GetTypeName(result)}`.");
        }

        return result;
    }

    private Value CallFunction(KFunction function, List<ASTNode?> args, Token token, string functionName)
    {
        var defaultParameters = function.DefaultParameters;
        var functionFrame = CreateFrame(functionName);
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
            throw new TypeError(token, $"Expected type `{TypeRegistry.GetTypeName(returnTypeHint)}` for return type of `{functionName}` but received `{TypeRegistry.GetTypeName(result)}`.");
        }

        return result;
    }

    private void ProcessFunctionParameters(KFunction function, List<ASTNode?> args, Token token, string functionName, HashSet<string> defaultParameters, Scope scope, Dictionary<string, int> typeHints)
    {
        for (var i = 0; i < function.Parameters.Count; ++i)
        {
            var param = function.Parameters[i];
            var argValue = Value.Default;

            if (i < args.Count)
            {
                argValue = Interpret(args[i]);
            }
            else if (defaultParameters.Contains(param.Key))
            {
                argValue = param.Value;
            }
            else
            {
                throw new ParameterCountMismatchError(token, functionName, function.Parameters.Count, args.Count);
            }

            PrepareFunctionVariables(typeHints, param, ref argValue, token, i, functionName, scope);
        }
    }

    private Value CallLambda(Token token, string lambdaName, List<ASTNode?> args, ref bool doPop)
    {
        var scope = new Scope(CallStack.Peek().Scope);
        var lambdaFrame = PushFrame(lambdaName, scope, true);
        var targetLambda = lambdaName;
        var result = Value.Default;

        if (!Context.HasLambda(targetLambda))
        {
            if (!Context.HasMappedLambda(targetLambda))
            {
                throw new CallableError(token, $"Could not find target lambda `{targetLambda}`");
            }

            targetLambda = Context.LambdaTable[targetLambda];
        }

        var func = Context.Lambdas[targetLambda];
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
            throw new TypeError(token, $"Expected type `{TypeRegistry.GetTypeName(returnTypeHint)}` for return type of `{lambdaName}` but received `{TypeRegistry.GetTypeName(result)}`.");
        }

        return result;
    }

    private Value CallObjectBaseMethod(MethodCallNode node, InstanceRef obj, string baseStruct, string methodName)
    {
        var struc = Context.Structs[baseStruct];
        var function = struc.Methods[methodName];
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
            throw new UnimplementedMethodError(node.Token, obj.StructName, methodName);
        }

        if (function.IsPrivate)
        {
            throw new InvalidContextError(node.Token, "Cannot invoke private method outside of struct.");
        }

        var result = CallFunction(function, node.Arguments, node.Token, methodName);

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

    private void PrepareFunctionCall(KFunction func, FunctionCallNode node, HashSet<string> defaultParameters, Dictionary<string, int> typeHints, StackFrame functionFrame)
    {
        var parms = func.Parameters;
        var nodeArguments = node.Arguments;
        var scope = functionFrame.Scope;

        for (var i = 0; i < parms.Count; ++i)
        {
            var param = parms[i];
            var argValue = Value.Default;

            if (i < nodeArguments.Count)
            {
                var arg = nodeArguments[i];
                argValue = Interpret(arg);
            }
            else if (defaultParameters.Contains(param.Key))
            {
                argValue = param.Value;
            }
            else
            {
                throw new ParameterCountMismatchError(node.Token, node.FunctionName, parms.Count, nodeArguments.Count);
            }

            if (typeHints.TryGetValue(param.Key, out int expectedType) && !AssertTypeMatch(node.Token, argValue, expectedType))
            {
                throw new TypeError(node.Token, $"Expected type `{TypeRegistry.GetTypeName(expectedType)}` for parameter {(1 + i)} of `{node.FunctionName}` but received `{TypeRegistry.GetTypeName(argValue)}`.");
            }

            if (argValue.IsLambda())
            {
                Context.AddMappedLambda(param.Key, argValue.GetLambda().Identifier);
            }
            else
            {
                scope.Declare(param.Key, argValue);
            }
        }
    }

    private void PrepareFunctionVariables(Dictionary<string, int> typeHints, KeyValuePair<string, Value> param, ref Value argValue, Token token, int i, string functionName, Scope scope)
    {
        if (typeHints.TryGetValue(param.Key, out int expectedType) && !AssertTypeMatch(token, argValue, expectedType))
        {
            throw new TypeError(token, $"Expected type `{TypeRegistry.GetTypeName(expectedType)}` for parameter {(1 + i)} of `{functionName}` but received `{TypeRegistry.GetTypeName(argValue)}`.");
        }

        if (argValue.IsLambda())
        {
            var lambdaId = argValue.GetLambda().Identifier;
            Context.AddMappedLambda(param.Key, lambdaId);
        }
        else
        {
            scope.Declare(param.Key, argValue);
        }
    }

    private void PrepareLambdaCall(KLambda func, List<ASTNode?> args, HashSet<string> defaultParameters, Token token, string targetLambda, Dictionary<string, int> typeHints, string lambdaName, Scope scope)
    {
        var parms = func.Parameters;
        for (var i = 0; i < parms.Count; ++i)
        {
            var param = parms[i];
            var argValue = Value.Default;
            if (i < args.Count)
            {
                argValue = Interpret(args[i]);
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
    }

    private void PrepareLambdaCall(KLambda func, List<Value> args, HashSet<string> defaultParameters, Token token, string targetLambda, Dictionary<string, int> typeHints, string lambdaName, Scope scope)
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
    }

    private void PrepareLambdaVariables(Dictionary<string, int> typeHints, KeyValuePair<string, Value> param, ref Value argValue, Token token, int i, string lambdaName, Scope scope)
    {
        if (typeHints.TryGetValue(param.Key, out int expectedType) && !AssertTypeMatch(token, argValue, expectedType))
        {
            throw new TypeError(token, $"Expected type `{TypeRegistry.GetTypeName(expectedType)}` for parameter {(1 + i)} of `{lambdaName}` but received `{TypeRegistry.GetTypeName(argValue)}`.");
        }

        if (argValue.IsLambda())
        {
            var lambdaId = argValue.GetLambda().Identifier;
            Context.AddMappedLambda(param.Key, lambdaId);
        }
        else
        {
            scope.Declare(param.Key, argValue);
        }
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
        var functionFrame = CreateFrame(functionName);
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
            throw new TypeError(node.Token, $"Expected type `{TypeRegistry.GetTypeName(returnTypeHint)}` for return type of `{functionName}` but received `{TypeRegistry.GetTypeName(result)}`.");
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

        if (isCtor)
        {
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

        var result = CallFunction(function, node.Arguments, node.Token, methodName);

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

    private void ImportPackage(Token token, Value packageName)
    {
        if (!packageName.IsString())
        {
            throw new InvalidOperationError(token, "Expected the name of a package to import.");
        }

        var packageNameValue = packageName.GetString();

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
        var package = Context.Packages[packageNameValue];
        var decl = package.Decl;

        foreach (var stmt in decl.Body)
        {
            Interpret(stmt);
        }

        // detect type builtin definitions
        if (TypeRegistry.TryGetPrimitiveType(packageNameValue, out int type))
        {
            var prefix = packageNameValue + "::";
            var funcs = Context.Functions.Where(x => x.Key.StartsWith(prefix)).ToList();
            foreach (var func in funcs)
            {
                TypeBuiltins.Register(type, func.Key.Replace(prefix, string.Empty), func.Value);
            }
        }

        if (PackageStack.Count > 0)
        {
            PackageStack.Pop();
        }
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
        using Lexer lexer = new(path);

        Parser p = new(true);
        var tokenStream = lexer.GetTokenStream();
        var ast = p.ParseTokenStream(tokenStream, true);

        Interpret(ast);

        return;
    }
    private Value BytesLoop(ForLoopNode node, byte[] list)
    {
        var frame = CallStack.Peek();
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
                scope.Assign(valueName, Value.CreateInteger(list[i]));
                if (indexName != null)
                {
                    scope.Assign(indexName, Value.CreateInteger(i));
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
            scope.Remove(valueName);
            if (indexName != null)
            {
                scope.Remove(indexName);
            }

            frame.ClearFlag(FrameFlags.InLoop);
        }

        return result;
    }

    private Value ListLoop(ForLoopNode node, List<Value> list)
    {
        var frame = CallStack.Peek();
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
                scope.Assign(valueName, list[i]);
                if (indexName != null)
                {
                    scope.Assign(indexName, Value.CreateInteger(i));
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
            scope.Remove(valueName);
            if (indexName != null)
            {
                scope.Remove(indexName);
            }

            frame.ClearFlag(FrameFlags.InLoop);
        }

        return result;
    }

    private Value HashmapLoop(ForLoopNode node, Dictionary<Value, Value> hash)
    {
        var frame = CallStack.Peek();
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

                scope.Assign(keyName, kvp.Key);
                if (valueName != null)
                {
                    scope.Assign(valueName, kvp.Value);
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
            scope.Remove(keyName);
            if (valueName != null)
            {
                scope.Remove(valueName);
            }
            frame.ClearFlag(FrameFlags.InLoop);
        }

        return result;
    }

    private Value InterpretKiwiBuiltin(Token token, TokenName op, List<Value> args)
    {
        return op switch
        {
            TokenName.Builtin_Kiwi_TypeOf => TypeOf(token, args),
            _ => throw new InvalidOperationError(token, "Invalid builtin invocation."),
        };
    }

    private Value TypeOf(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, KiwiBuiltin.TypeOf, 1, args.Count);
        var target = args[0];

        return Value.CreateString(TypeRegistry.GetTypeName(target));
    }

    private Value InterpretCallableBuiltin(MethodCallNode node, List<Value> args)
    {
        var result = Value.Default;
        var obj = Interpret(node.Object);

        Callable? callable = null;
        string? callableName = null;
        if (obj.IsLambda())
        {
            callableName = obj.GetLambda().Identifier;
            
            if (Context.HasLambda(callableName))
            {
                callable = Context.Lambdas[callableName];
            }
            else if (Context.HasMappedLambda(callableName))
            {
                callable = Context.Lambdas[Context.LambdaTable[callableName]];
            }
        }
        else if (node.Object?.Type == ASTNodeType.Identifier)
        {
            callableName = Id(node.Object);

            if (Context.HasFunction(callableName))
            {
                callable = Context.Functions[callableName];
            }
        }

        if (callableName == null)
        {
            throw new InvalidOperationError(node.Token, $"Expected a callable for function `{CallableBuiltin.MapName(node.Op)}`.");
        }

        if (callable == null)
        {
            throw new InvalidOperationError(node.Token, $"Expected a callable for function `{CallableBuiltin.MapName(node.Op)}` on `{callableName}`.");
        }

        if (!(callable is KFunction || callable is KLambda))
        {
            throw new InvalidOperationError(node.Token, $"Expected a function or lambda for function `{CallableBuiltin.MapName(node.Op)}` on `{callableName}`.");
        }

        return CallableBuiltinHandler.Execute(this, node.Token, node.Op, callable, callableName, args);
    }

    private Value InterpretListBuiltin(Token token, ref Value obj, TokenName op, List<Value> args)
    {
        if (!obj.IsList())
        {
            throw new InvalidOperationError(token, "Expected a list for specialized list builtin.");
        }

        var list = obj.GetList();

        switch (op)
        {
            case TokenName.Builtin_List_Max:
                return ListMax(token, list);

            case TokenName.Builtin_List_Min:
                return ListMin(token, list);

            case TokenName.Builtin_List_Sort:
                // perform a simple sort, otherwise expect a lambda.
                if (args.Count == 0)
                {
                    return ListSort(list);
                }
                break;

            case TokenName.Builtin_List_Sum:
                return ListSum(list);

            default:
                break;
        }

        if (args.Count == 1 && args[0].IsInteger())
        {
            switch (op)
            {
                case TokenName.Builtin_List_Skip:
                    return ListSkip(ref obj, (int)args[0].GetInteger());
                case TokenName.Builtin_List_Take:
                    return ListTake(ref obj, (int)args[0].GetInteger());
            }
        }

        if (args.Count == 1 && args[0].IsLambda())
        {
            var arg = args[0];

            var lambdaRef = arg.GetLambda();

            if (!Context.HasLambda(lambdaRef.Identifier))
            {
                throw new InvalidOperationError(token, $"Unrecognized lambda '{lambdaRef.Identifier}'.");
            }

            var lambda = Context.Lambdas[lambdaRef.Identifier];
            var isReturnSet = CallStack.Peek().IsFlagSet(FrameFlags.Return);
            var result = Value.Default;

            switch (op)
            {
                case TokenName.Builtin_List_Sort:
                    result = LambdaSort(lambda, list);
                    break;

                case TokenName.Builtin_List_Each:
                    result = LambdaEach(lambda, list);
                    break;

                case TokenName.Builtin_List_Map:
                    result = LambdaMap(lambda, list);
                    break;

                case TokenName.Builtin_List_None:
                    result = LambdaNone(lambda, list);
                    break;

                case TokenName.Builtin_List_Filter:
                    result = LambdaFilter(lambda, list);
                    break;

                case TokenName.Builtin_List_All:
                    result = LambdaAll(lambda, list);
                    break;

                default:
                    break;
            }

            var frame = CallStack.Peek();
            if (!isReturnSet && frame.IsFlagSet(FrameFlags.Return) && frame.ReturnValue != null)
            {
                if (!BooleanOp.IsSame(frame.ReturnValue, result))
                {
                    frame.ReturnValue = result;
                }
            }

            return result;
        }
        
        if (args.Count == 2 && op == TokenName.Builtin_List_Reduce)
        {
            var arg = args[1];

            if (!arg.IsLambda())
            {
                throw new InvalidOperationError(token, "Expected a lambda in specialized list builtin.");
            }
            var lambdaRef = arg.GetLambda();

            if (!Context.HasLambda(lambdaRef.Identifier))
            {
                throw new InvalidOperationError(token, $"Unrecognized lambda '{lambdaRef.Identifier}'.");
            }

            var lambda = Context.Lambdas[lambdaRef.Identifier];

            return LambdaReduce(lambda, args[0], list);
        }

        throw new InvalidOperationError(token, "Invalid specialized list builtin invocation.");
    }

    private static Value ListSkip(ref Value obj, int count)
    {
        var lst = obj.GetList();

        try
        {
            return Value.CreateList([.. lst.Skip(count)]);
        }
        catch {}

        return Value.CreateList();
    }

    private static Value ListTake(ref Value obj, int count)
    {
        var lst = obj.GetList();

        try
        {
            return Value.CreateList([.. lst.Take(count)]);
        }
        catch {}

        return Value.CreateList();
    }

    private static Value ListSum(List<Value> list)
    {
        var sum = 0D;
        var isFloatResult = false;

        foreach (var val in list)
        {
            if (val.IsInteger())
            {
                sum += val.GetInteger();
            }
            else if (val.IsFloat())
            {
                sum += val.GetFloat();
                isFloatResult = true;
            }
        }

        if (isFloatResult)
        {
            return Value.CreateFloat(sum);
        }

        return Value.CreateInteger((long)sum);
    }

    private static Value ListMin(Token token, List<Value> list)
    {
        if (list.Count == 0)
        {
            return Value.CreateNull();
        }

        var minValue = list[0];

        for (var i = 0; i < list.Count; i++)
        {
            var val = list[i];

            if (ComparisonOp.GetLtResult(ref val, ref minValue))
            {
                minValue = val;
            }
        }

        return minValue;
    }

    private static Value ListMax(Token token, List<Value> list)
    {
        if (list.Count == 0)
        {
            return Value.CreateNull();
        }

        var maxValue = list[0];

        for (var i = 0; i < list.Count; i++)
        {
            var val = list[i];

            if (ComparisonOp.GetGtResult(ref val, ref maxValue))
            {
                maxValue = val;
            }
        }

        return maxValue;
    }

    private static Value ListSort(List<Value> list)
    {
        list.Sort();
        return Value.CreateList(list);
    }

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

    private Value LambdaSort(KLambda lambda, List<Value> list)
    {
        var frame = CallStack.Peek();
        var scope = frame.Scope;

        if (lambda.Parameters.Count != 2)
        {
            return Value.CreateList(list);
        }

        var lhsVar = lambda.Parameters[0].Key;
        var rhsVar = lambda.Parameters[1].Key;

        scope.Declare(lhsVar, Value.Default);
        scope.Declare(rhsVar, Value.Default);

        var decl = lambda.Decl;

        list.Sort((a, b) =>
        {
            scope.Assign(lhsVar, a);
            scope.Assign(rhsVar, b);

            Value result = Value.Default;

            foreach (var stmt in decl.Body)
            {
                result = Interpret(stmt);
                if (frame.IsFlagSet(FrameFlags.Return))
                {
                    frame.ClearFlag(FrameFlags.Return);
                }
            }

            bool isLess = BooleanOp.IsTruthy(result);

            if (isLess) return -1;

            scope.Assign(lhsVar, b);
            scope.Assign(rhsVar, a);

            result = Value.Default;

            foreach (var stmt in decl.Body)
            {
                result = Interpret(stmt);
                if (frame.IsFlagSet(FrameFlags.Return))
                {
                    frame.ClearFlag(FrameFlags.Return);
                }
            }

            bool isGreater = BooleanOp.IsTruthy(result);

            if (isGreater)
            {
                return 1;
            }

            return 0;
        });

        scope.Remove(lhsVar);
        scope.Remove(rhsVar);

        return Value.CreateList(list);
    }

    private Value LambdaEach(KLambda lambda, List<Value> list)
    {
        var defaultParameters = lambda.DefaultParameters;
        var scope = CallStack.Peek().Scope;

        var valueVariable = string.Empty;
        var indexVariable = string.Empty;
        var hasIndexVariable = false;

        if (lambda.Parameters.Count == 0)
        {
            return Value.Default;
        }

        for (var i = 0; i < lambda.Parameters.Count; ++i)
        {
            var param = lambda.Parameters[i];
            if (i == 0)
            {
                valueVariable = param.Key;
                scope.Assign(valueVariable, Value.Default);
            }
            else if (i == 1)
            {
                indexVariable = param.Key;
                hasIndexVariable = true;
                scope.Assign(indexVariable, Value.Default);
            }
        }

        var result = Value.Default;
        var indexValue = Value.Default;
        var decl = lambda.Decl;

        for (var i = 0; i < list.Count; ++i)
        {
            scope.Assign(valueVariable, list[i]);

            if (hasIndexVariable)
            {
                indexValue.SetValue(i);
                scope.Assign(indexVariable, indexValue);
            }

            foreach (var stmt in decl.Body)
            {
                result = Interpret(stmt);
            }
        }

        scope.Remove(valueVariable);
        if (hasIndexVariable)
        {
            scope.Remove(indexVariable);
        }

        return result;
    }

    private Value LambdaNone(KLambda lambda, List<Value> list)
    {
        var filtered = LambdaFilter(lambda, list);
        var noneFound = Value.False;

        if (filtered.IsList())
        {
            var isEmpty = filtered.GetList().Count == 0;
            noneFound.SetValue(isEmpty);
        }

        return noneFound;
    }

    private Value LambdaMap(KLambda lambda, List<Value> list)
    {
        var defaultParameters = lambda.DefaultParameters;
        var frame = CallStack.Peek();
        var scope = frame.Scope;

        var mapVariable = string.Empty;

        if (lambda.Parameters.Count == 0)
        {
            return Value.CreateList(list);
        }

        for (var i = 0; i < lambda.Parameters.Count; ++i)
        {
            var param = lambda.Parameters[i];
            if (i == 0)
            {
                mapVariable = param.Key;
                scope.Assign(mapVariable, Value.Default);
            }
        }

        var decl = lambda.Decl;
        List<Value> resultList = [];
        Value result = Value.Default;

        for (var i = 0; i < list.Count; ++i)
        {
            scope.Assign(mapVariable, list[i]);

            foreach (var stmt in decl.Body)
            {
                result = Interpret(stmt);
                if (frame.IsFlagSet(FrameFlags.Return))
                {
                    frame.ClearFlag(FrameFlags.Return);
                }
                resultList.Add(result);
            }
        }

        scope.Remove(mapVariable);

        return Value.CreateList(resultList);
    }

    private Value LambdaReduce(KLambda lambda, Value accumulator, List<Value> list)
    {
        var defaultParameters = lambda.DefaultParameters;
        var scope = CallStack.Peek().Scope;

        var accumVariable = string.Empty;
        var valueVariable = string.Empty;

        if (lambda.Parameters.Count != 2)
        {
            return accumulator;
        }

        for (var i = 0; i < lambda.Parameters.Count; ++i)
        {
            var param = lambda.Parameters[i];
            if (i == 0)
            {
                accumVariable = param.Key;
                scope.Assign(accumVariable, accumulator);
            }
            else if (i == 1)
            {
                valueVariable = param.Key;
                scope.Assign(valueVariable, Value.Default);
            }
        }

        var decl = lambda.Decl;
        Value result;

        for (var i = 0; i < list.Count; ++i)
        {
            scope.Assign(valueVariable, list[i]);

            foreach (var stmt in decl.Body)
            {
                result = Interpret(stmt);
            }
        }

        result = scope.GetBinding(accumVariable);

        scope.Remove(accumVariable);
        scope.Remove(valueVariable);

        return result;
    }

    private Value LambdaAll(KLambda lambda, List<Value> list)
    {
        var defaultParameters = lambda.DefaultParameters;
        var scope = CallStack.Peek().Scope;

        var valueVariable = string.Empty;
        var indexVariable = string.Empty;
        var hasIndexVariable = false;

        var listSize = list.Count;
        var newListSize = 0;

        for (var i = 0; i < lambda.Parameters.Count; ++i)
        {
            var param = lambda.Parameters[i];
            if (i == 0)
            {
                valueVariable = param.Key;
                scope.Assign(valueVariable, Value.Default);
            }
            else if (i == 1)
            {
                indexVariable = param.Key;
                hasIndexVariable = true;
                scope.Assign(indexVariable, Value.Default);
            }
        }

        var result = Value.Default;
        var indexValue = Value.Default;
        var decl = lambda.Decl;

        for (var i = 0; i < list.Count; ++i)
        {
            scope.Assign(valueVariable, list[i]);

            if (hasIndexVariable)
            {
                indexValue.SetValue(i);
                scope.Assign(indexVariable, indexValue);
            }

            foreach (var stmt in decl.Body)
            {
                result = Interpret(stmt);

                if (BooleanOp.IsTruthy(result))
                {
                    ++newListSize;
                }
            }
        }

        scope.Remove(valueVariable);
        if (hasIndexVariable)
        {
            scope.Remove(indexVariable);
        }

        return Value.CreateBoolean(newListSize == listSize);
    }

    private Value LambdaFilter(KLambda lambda, List<Value> list)
    {
        var defaultParameters = lambda.DefaultParameters;
        var scope = CallStack.Peek().Scope;

        var valueVariable = string.Empty;
        var indexVariable = string.Empty;
        var hasIndexVariable = false;

        for (var i = 0; i < lambda.Parameters.Count; ++i)
        {
            var param = lambda.Parameters[i];
            if (i == 0)
            {
                valueVariable = param.Key;
                scope.Assign(valueVariable, Value.Default);
            }
            else if (i == 1)
            {
                indexVariable = param.Key;
                hasIndexVariable = true;
                scope.Assign(indexVariable, Value.Default);
            }
        }

        var result = Value.Default;
        var indexValue = Value.Default;
        var decl = lambda.Decl;
        List<Value> resultList = [];

        for (var i = 0; i < list.Count; ++i)
        {
            scope.Assign(valueVariable, list[i]);

            if (hasIndexVariable)
            {
                indexValue.SetValue(i);
                scope.Assign(indexVariable, indexValue);
            }

            foreach (var stmt in decl.Body)
            {
                result = Interpret(stmt);

                if (BooleanOp.IsTruthy(result))
                {
                    resultList.Add(list[i]);
                }
            }
        }

        scope.Remove(valueVariable);
        if (hasIndexVariable)
        {
            scope.Remove(indexVariable);
        }

        return Value.CreateList(resultList);
    }

    private StackFrame PushFrame(StackFrame frame, bool inLambda = false)
    {
        if (inLambda)
        {
            frame.SetFlag(FrameFlags.InLambda);
        }

        CallStack.Push(frame);
        FuncStack.Push(frame.Name);
        return frame;
    }

    private StackFrame PushFrame(string name, Scope scope, bool inLambda = false)
    {
        var frame = new StackFrame(name, scope);

        if (inLambda)
        {
            frame.SetFlag(FrameFlags.InLambda);
        }

        CallStack.Push(frame);
        FuncStack.Push(name);
        return frame;
    }

    private Value PopFrame()
    {
        if (CallStack.Count == 0)
        {
            return Value.Default;
        }

        var frame = CallStack.Pop();
        FuncStack.Pop();

        var ret = frame.ReturnValue ?? Value.Default;
        if (CallStack.Count > 0)
        {
            CallStack.Peek().ReturnValue = ret;
        }

        return ret;
    }

    private bool PushFrame(StackFrame frame)
    {
        CallStack.Push(frame);
        FuncStack.Push(frame.Name);
        return true;
    }

    private bool InTry()
    {
        if (CallStack.Count == 0)
        {
            return false;
        }

        return CallStack.Peek().IsFlagSet(FrameFlags.InTry);
    }

    private StackFrame CreateFrame(string name)
    {
        StackFrame frame = CallStack.Peek();
        Scope scope = new(frame.Scope);
        StackFrame subFrame = new(name, scope);

        if (frame.InObjectContext())
        {
            var objectContext = frame.GetObjectContext();
            subFrame.SetObjectContext(objectContext);
        }

        if (frame.IsFlagSet(FrameFlags.InTry))
        {
            subFrame.SetFlag(FrameFlags.InTry);
        }

        if (frame.IsFlagSet(FrameFlags.SubFrame))
        {
            subFrame.SetFlag(FrameFlags.SubFrame);
        }

        return subFrame;
    }

    private static string Id(ASTNode node) => ((IdentifierNode)node).Name;
}