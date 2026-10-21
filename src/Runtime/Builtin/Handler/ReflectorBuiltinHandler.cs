using kiwi.Parsing;
using kiwi.VM;
using kiwi.Parsing.Keyword;
using kiwi.Runtime.Builtin.Operation;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class ReflectorBuiltinHandler
{
    public static Value Execute(KiwiVM vm, Token token, TokenName op, List<Value> args, KContext context, Stack<StackFrame> callStack)
    {
        return op switch
        {
            TokenName.Builtin_Reflector_CallStack     => CallStack(vm, token, args),
            TokenName.Builtin_Reflector_FrameFlags    => FFlags(token, callStack, args),
            TokenName.Builtin_Reflector_GetFunc       => GetFunc(vm, token, context, args),
            TokenName.Builtin_Reflector_ObjectContext => ObjectContext(token, callStack, args),
            TokenName.Builtin_Reflector_RetVal        => RetVal(token, callStack, args),
            TokenName.Builtin_Reflector_State         => State(vm, token, context, args),
            _ => throw new InvalidOperationError(token, "Unknown reflector builtin.")
        };
    }

    private static Value CallStack(KiwiVM vm, Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, ReflectorBuiltin.CallStack, 0, args.Count);

        var names = CallerFrames(vm)
            .Select(i => vm.GetFrame(i))
            .Select(f => Value.CreateString(f.Chunk.PackagePrefix + f.Name))
            .ToList();
        return Value.CreateList(names);
    }

    /// <summary>
    /// Indexes of the VM frames from the bottom (`<main>`) up to the caller of the reflector
    /// function, leaving out the reflector package's own frame.
    /// </summary>
    private static IEnumerable<int> CallerFrames(KiwiVM vm)
    {
        int top = vm.FrameCount;
        if (top > 0 && vm.GetFrame(top - 1).Chunk.PackagePrefix == "reflector::") top--;
        return Enumerable.Range(0, top);
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

    private static Value GetFunc(KiwiVM vm, Token token, KContext context, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, ReflectorBuiltin.GetFunc, 1, args.Count);
        ParameterTypeMismatchError.ExpectString(token, ReflectorBuiltin.GetFunc, 0, args[0]);

        var callableName = args[0].GetString();
        if (!context.Functions.TryGetValue(callableName, out KFunction? func) || func == null)
        {
            return Value.CreateNull();
        }

        return CallableBuiltinHandler.ToLambda(vm, token, func, callableName, []);
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

    private static Value State(KiwiVM vm, Token token, KContext ctx, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, ReflectorBuiltin.State, 0, args.Count);

        var result = new Dictionary<Value, Value>();
        var packages = ctx.Packages.Keys.Select(Value.CreateString).OrderBy(v => v.GetString()).ToList();
        var structs  = ctx.Structs.Keys.Select(Value.CreateString).OrderBy(v => v.GetString()).ToList();
        var functions = ctx.Functions.Keys.Select(Value.CreateString).OrderBy(v => v.GetString()).ToList();

        var stackFrames = new List<Value>();
        foreach (var i in CallerFrames(vm)) // Bottom to Top
        {
            var frame = vm.GetFrame(i);
            // A top-level frame's variables are the globals; a function's are its locals.
            var bindings = frame.IsGlobalScope
                ? vm.GetGlobalScope().GetAllBindings().Select(kv => (Name: kv.Key, Val: kv.Value))
                : vm.GetFrameLocals(i);
            var frameVars = new List<Value>();
            foreach (var (name, val) in bindings.OrderBy(b => b.Name))
            {
                var varMap = new Dictionary<Value, Value> { [Value.CreateString(name)] = val };
                frameVars.Add(Value.CreateHashmap(varMap));
            }
            var frameMap = new Dictionary<Value, Value>
            {
                [Value.CreateString("name")]      = Value.CreateString(frame.Chunk.PackagePrefix + frame.Name),
                [Value.CreateString("variables")] = Value.CreateList(frameVars),
            };
            stackFrames.Add(Value.CreateHashmap(frameMap));
        }

        result[Value.CreateString("packages")]  = Value.CreateList(packages);
        result[Value.CreateString("structs")]   = Value.CreateList(structs);
        result[Value.CreateString("functions")] = Value.CreateList(functions);
        result[Value.CreateString("stack")]     = Value.CreateList(stackFrames);

        return Value.CreateHashmap(result);
    }
}