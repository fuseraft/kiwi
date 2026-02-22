using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Runtime.Builtin.Operation;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class ReflectorBuiltinHandler
{
    public static Value Execute(Token token, TokenName op, List<Value> args, KContext context, Stack<StackFrame> callStack, Stack<string> funcStack)
    {
        return op switch
        {
            TokenName.Builtin_Reflector_CallStack     => CallStack(token, funcStack, args),
            TokenName.Builtin_Reflector_FrameFlags    => FFlags(token, callStack, args),
            TokenName.Builtin_Reflector_GetFunc       => GetFunc(token, context, args),
            TokenName.Builtin_Reflector_ObjectContext => ObjectContext(token, callStack, args),
            TokenName.Builtin_Reflector_RetVal        => RetVal(token, callStack, args),
            TokenName.Builtin_Reflector_State         => State(token, context, callStack, args),
            _ => throw new InvalidOperationError(token, "Unknown reflector builtin.")
        };
    }

    private static Value CallStack(Token token, Stack<string> funcStack, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, ReflectorBuiltin.CallStack, 0, args.Count);

        var names = funcStack.Reverse().Select(Value.CreateString).ToList();
        return Value.CreateList(names);
    }

    private static Value FFlags(Token token, Stack<StackFrame> callStack, List<Value> args)
    {
        ParameterCountMismatchError.CheckRange(token, ReflectorBuiltin.FrameFlags, 0, 1, args.Count);

        int fromTop = 0;
        if (args.Count == 1)
        {
            fromTop = (int)ConversionOp.GetInteger(token, args[0]);
            if (fromTop < 0)
            {
                throw new InvalidOperationError(token, "Frame index must be non-negative");
            }
        }

        var frames = callStack.ToArray(); // Top is last
        if (fromTop >= frames.Length)
        {
            throw new InvalidOperationError(token, $"Only {frames.Length} frames exist");
        }

        var frame = frames[frames.Length - 1 - fromTop];
        var flags = new List<Value>();

        void AddIfSet(FrameFlags flag)
        {
            if (frame.IsFlagSet(flag))
            {
                flags.Add(Value.CreateInteger((long)flag));
            }
        }

        AddIfSet(FrameFlags.Break);
        AddIfSet(FrameFlags.InLambda);
        AddIfSet(FrameFlags.InLoop);
        AddIfSet(FrameFlags.InObject);
        AddIfSet(FrameFlags.InTry);
        AddIfSet(FrameFlags.Next);
        AddIfSet(FrameFlags.None);
        AddIfSet(FrameFlags.Return);
        AddIfSet(FrameFlags.SubFrame);

        return Value.CreateList(flags);
    }

    private static Value GetFunc(Token token, KContext context, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, ReflectorBuiltin.GetFunc, 1, args.Count);
        ParameterTypeMismatchError.ExpectString(token, ReflectorBuiltin.GetFunc, 0, args[0]);

        var callableName = args[0].GetString();
        if (!context.Functions.TryGetValue(callableName, out KFunction? func) || func == null)
        {
            return Value.CreateNull();
        }

        var interp = Interpreter.Current ?? throw new RuntimeError(token, ReflectorBuiltin.GetFunc, []);
        return CallableBuiltinHandler.ToLambda(interp, token, func, callableName, []);
    }

    private static Value ObjectContext(Token token, Stack<StackFrame> callStack, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, ReflectorBuiltin.ObjectContext, 0, args.Count);

        if (callStack.Count == 0 || !callStack.Peek().InObjectContext())
        {
            return Value.CreateNull();
        }

        var obj = callStack.Peek().GetObjectContext() ?? throw new NullObjectError(token);
        return Value.CreateObject(obj);
    }

    private static Value RetVal(Token token, Stack<StackFrame> callStack, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, ReflectorBuiltin.RetVal, 0, args.Count);
        return callStack.Count > 0 ? callStack.Peek().ReturnValue ?? Value.Default : Value.Default;
    }

    private static Value State(Token token, KContext ctx, Stack<StackFrame> callStack, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, ReflectorBuiltin.State, 0, args.Count);

        var result = new Dictionary<Value, Value>();
        var packages = ctx.Packages.Keys.Select(Value.CreateString).OrderBy(v => v.GetString()).ToList();
        var structs  = ctx.Structs.Keys.Select(Value.CreateString).OrderBy(v => v.GetString()).ToList();
        var functions = ctx.Functions.Keys.Select(Value.CreateString).OrderBy(v => v.GetString()).ToList();

        var stackFrames = new List<Value>();
        foreach (var frame in callStack.Reverse()) // Bottom to Top
        {
            var frameVars = new List<Value>();
            foreach (var kv in frame.Scope.GetAllBindings().OrderBy(kv => kv.Key))
            {
                var varMap = new Dictionary<Value, Value> { [Value.CreateString(kv.Key)] = kv.Value };
                frameVars.Add(Value.CreateHashmap(varMap));
            }
            var frameMap = new Dictionary<Value, Value> { [Value.CreateString("variables")] = Value.CreateList(frameVars) };
            stackFrames.Add(Value.CreateHashmap(frameMap));
        }

        result[Value.CreateString("packages")]  = Value.CreateList(packages);
        result[Value.CreateString("structs")]   = Value.CreateList(structs);
        result[Value.CreateString("functions")] = Value.CreateList(functions);
        result[Value.CreateString("stack")]     = Value.CreateList(stackFrames);

        return Value.CreateHashmap(result);
    }
}