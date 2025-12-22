using System.Runtime.InteropServices;
using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Runtime.Builtin.Operation;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class ChannelBuiltinHandler
{
    public static Value Execute(TaskManager mgr, Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Channel_Create  => Create(mgr, token, args),
            TokenName.Builtin_Channel_Send    => Send(token, args),
            TokenName.Builtin_Channel_Recv    => Recv(token, args),
            TokenName.Builtin_Channel_TryRecv => TryRecv(token, args),
            TokenName.Builtin_Channel_Close   => Close(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text)
        };
    }

    private static Value Create(TaskManager mgr, Token token, List<Value> args)
    {
        if (args.Count > 1)
        {
            throw new ParameterCountMismatchError(token, ChannelBuiltin.Create);
        }

        int cap = args.Count > 0 ? (int)ConversionOp.GetInteger(token, args[0]) : 0;
        var chan = mgr.CreateChannel(cap);

        // WIP: returning as a managed reference
        var handle = GCHandle.Alloc(chan);
        return Value.CreatePointer(new IntPtr(GCHandle.ToIntPtr(handle)));
    }

    private static Channel GetChannel(Token token, Value ptrVal)
    {
        if (!ptrVal.IsPointer())
        {
            throw new TypeError(token, "Expected channel");
        }

        var handle = GCHandle.FromIntPtr(ptrVal.GetPointer());
        return (Channel)handle.Target! ?? throw new InvalidOperationError(token, "Invalid channel");
    }

    private static Value Send(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, ChannelBuiltin.Send, 2, args.Count);
        var chan = GetChannel(token, args[0]);
        chan.Send(token, args[1]);
        return Value.Default;
    }

    private static Value Recv(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, ChannelBuiltin.Recv, 1, args.Count);
        var chan = GetChannel(token, args[0]);
        return chan.Receive(token);
    }

    private static Value TryRecv(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, ChannelBuiltin.TryRecv, 1, args.Count);
        var chan = GetChannel(token, args[0]);
        var (ok, v) = chan.TryReceive();
        return Value.CreateList([Value.CreateBoolean(ok), v]);
    }

    private static Value Close(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, ChannelBuiltin.Close, 1, args.Count);
        var chan = GetChannel(token, args[0]);
        chan.Close();
        return Value.Default;
    }
}