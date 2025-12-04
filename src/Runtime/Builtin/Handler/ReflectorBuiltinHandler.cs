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
            TokenName.Builtin_Reflector_RList    => RList(token, context, callStack),
            TokenName.Builtin_Reflector_RObject  => RObject(token, callStack),
            TokenName.Builtin_Reflector_RStack   => RStack(token, funcStack),
            TokenName.Builtin_Reflector_RFFlags  => RFFlags(token, args, callStack),
            TokenName.Builtin_Reflector_RRetVal  => RRetVal(token, callStack),
            _ => throw new InvalidOperationError(token, "Unknown reflector builtin.")
        };
    }

    private static Value RFFlags(Token token, List<Value> args, Stack<StackFrame> callStack)
    {
        if (args.Count > 1)
            throw new ParameterCountMismatchError(token, "rfflags", 1, args.Count);

        int fromTop = 0;
        if (args.Count == 1)
        {
            fromTop = (int)ConversionOp.GetInteger(token, args[0]);
            if (fromTop < 0)
                throw new InvalidOperationError(token, "rfflags: frame index must be non-negative");
        }

        var frames = callStack.ToArray(); // Top is last
        if (fromTop >= frames.Length)
            throw new InvalidOperationError(token, $"rfflags: only {frames.Length} frames exist");

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

    private static Value RRetVal(Token token, Stack<StackFrame> callStack)
    {
        ParameterCountMismatchError.Check(token, ReflectorBuiltin.RRetVal, 0, 0);
        return callStack.Count > 0 ? callStack.Peek().ReturnValue ?? Value.Default : Value.Default;
    }

    private static Value RList(Token token, KContext ctx, Stack<StackFrame> callStack)
    {
        ParameterCountMismatchError.Check(token, ReflectorBuiltin.RList, 0, 0);

        var result = new Dictionary<Value, Value>();
        var packages = ctx.Packages.Keys.Select(Value.CreateString).OrderBy(v => v.GetString()).ToList();
        var structs  = ctx.Structs.Keys.Select(Value.CreateString).OrderBy(v => v.GetString()).ToList();
        var functions = ctx.Functions.Keys.Select(Value.CreateString).OrderBy(v => v.GetString()).ToList();

        var stackFrames = new List<Value>();
        foreach (var frame in callStack.Reverse()) // Bottom → Top
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

    private static Value RObject(Token token, Stack<StackFrame> callStack)
    {
        ParameterCountMismatchError.Check(token, ReflectorBuiltin.RObject, 0, 0);

        if (callStack.Count == 0 || !callStack.Peek().InObjectContext())
            return Value.EmptyString;

        var obj = callStack.Peek().GetObjectContext() ?? throw new NullObjectError(token);
        var serialized = Serializer.BasicSerializeObject(obj);
        return Value.CreateString(serialized);
    }

    private static Value RStack(Token token, Stack<string> funcStack)
    {
        ParameterCountMismatchError.Check(token, ReflectorBuiltin.RStack, 0, 0);

        var names = funcStack.Reverse().Select(Value.CreateString).ToList();
        return Value.CreateList(names);
    }
}