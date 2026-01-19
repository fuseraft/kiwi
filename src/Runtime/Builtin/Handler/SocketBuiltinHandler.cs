using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Runtime.Builtin.Operation;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class SocketBuiltinHandler
{
    private static SocketManager Mgr => SocketManager.Instance;

    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Socket_TcpServer   => TcpServer(token, args),
            TokenName.Builtin_Socket_TcpConnect  => TcpConnect(token, args),
            TokenName.Builtin_Socket_Accept      => Accept(token, args),
            TokenName.Builtin_Socket_Send        => Send(token, args),
            TokenName.Builtin_Socket_Receive     => Recv(token, args),
            TokenName.Builtin_Socket_Close       => Close(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text)
        };
    }

    private static Value TcpServer(Token token, List<Value> args)
    {
        if (args.Count != 2 && args.Count != 3)
        {        
            throw new ParameterCountMismatchError(token, SocketBuiltin.TcpServer);
        }

        ParameterTypeMismatchError.ExpectString(token, SocketBuiltin.TcpServer, 0, args[0]);
        ParameterTypeMismatchError.ExpectInteger(token, SocketBuiltin.TcpServer, 1, args[1]);

        string host = args[0].GetString();
        int port = (int)args[1].GetInteger();
        int backlog = 128;
        
        if (args.Count == 3)
        {
            ParameterTypeMismatchError.ExpectInteger(token, SocketBuiltin.TcpServer, 2, args[2]);
            backlog = (int)args[2].GetInteger();
        }

        long id = Mgr.TcpServer(host, port, backlog);
        return Value.CreateInteger(id);
    }

    private static Value TcpConnect(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SocketBuiltin.TcpConnect, 2, args.Count);
        ParameterTypeMismatchError.ExpectString(token, SocketBuiltin.TcpConnect, 0, args[0]);
        ParameterTypeMismatchError.ExpectInteger(token, SocketBuiltin.TcpConnect, 1, args[1]);
        
        string host = args[0].GetString();
        int port = (int)args[1].GetInteger();

        long id = Mgr.TcpConnect(host, port);
        return Value.CreateInteger(id);
    }

    private static Value Accept(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SocketBuiltin.Accept, 1, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, SocketBuiltin.Accept, 0, args[0]);
        long serverId = args[0].GetInteger();
        long taskId = SocketManager.Instance.EnqueueAccept(serverId);
        return Value.CreateInteger(taskId); // Awaitable!
    }

    private static Value Send(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SocketBuiltin.Send, 2, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, SocketBuiltin.Send, 0, args[0]);
        ParameterTypeMismatchError.ExpectBytes(token, SocketBuiltin.Send, 1, args[1]);

        ParameterCountMismatchError.Check(token, SocketBuiltin.Send, 2, args.Count);
        long sockId = args[0].GetInteger();
        byte[] data = args[1].GetBytes();

        long taskId = Mgr.EnqueueSend(sockId, data);
        return Value.CreateInteger(taskId);
    }

    private static Value Recv(Token token, List<Value> args)
    {
        int expected = args.Count == 1 ? 1 : 2;
        ParameterCountMismatchError.Check(token, SocketBuiltin.Receive, expected, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, SocketBuiltin.Receive, 0, args[0]);
        long sockId = args[0].GetInteger();
        int max = args.Count > 1 ? (int)args[1].GetInteger() : 4096;
        long taskId = SocketManager.Instance.EnqueueRecv(sockId, max);
        return Value.CreateInteger(taskId);
    }

    private static Value Close(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SocketBuiltin.Close, 1, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, SocketBuiltin.Close, 0, args[0]);

        long sockId = args[0].GetInteger();
        Mgr.Close(sockId);
        return Value.Default;
    }
}