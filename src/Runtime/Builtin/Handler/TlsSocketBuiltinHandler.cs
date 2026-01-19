using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class TlsSocketBuiltinHandler
{
    private static TlsSocketManager Mgr => TlsSocketManager.Instance;

    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Tls_TcpServer => TlsServer(token, args),
            TokenName.Builtin_Tls_TcpConnect => TlsConnect(token, args),
            TokenName.Builtin_Tls_Accept => Accept(token, args),
            TokenName.Builtin_Tls_Send => Send(token, args),
            TokenName.Builtin_Tls_Receive => Recv(token, args),
            TokenName.Builtin_Tls_Close => Close(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text)
        };
    }

    private static Value TlsServer(Token token, List<Value> args)
    {
        if (args.Count != 2 && args.Count != 3)
        {
            throw new ParameterCountMismatchError(token, TlsSocketBuiltin.TcpServer);
        }

        ParameterTypeMismatchError.ExpectString(token, TlsSocketBuiltin.TcpServer, 0, args[0]);
        ParameterTypeMismatchError.ExpectInteger(token, TlsSocketBuiltin.TcpServer, 1, args[1]);

        string host = args[0].GetString();
        int port = (int)args[1].GetInteger();
        int backlog = 128;
        
        if (args.Count == 3)
        {
            ParameterTypeMismatchError.ExpectInteger(token, TlsSocketBuiltin.TcpServer, 2, args[2]);
            backlog = (int)args[2].GetInteger();
        }

        long id = Mgr.TcpServer(host, port, backlog);
        return Value.CreateInteger(id);
    }

    private static Value TlsConnect(Token token, List<Value> args)
    {
        // host, port, SNI host
        if (args.Count < 2 || args.Count > 3)
        {
            throw new ParameterCountMismatchError(token, TlsSocketBuiltin.TcpConnect);
        }

        ParameterTypeMismatchError.ExpectString(token, TlsSocketBuiltin.TcpConnect, 0, args[0]);
        ParameterTypeMismatchError.ExpectInteger(token, TlsSocketBuiltin.TcpConnect, 1, args[1]);

        string host = args[0].GetString();
        int port = (int)args[1].GetInteger();

        string? sniHost = null;
        if (args.Count == 3)
        {
            ParameterTypeMismatchError.ExpectString(token, TlsSocketBuiltin.TcpConnect, 2, args[2]);
            sniHost = args[2].GetString();
            if (string.IsNullOrWhiteSpace(sniHost))
            {
                sniHost = null;
            }
        }

        long taskId = TaskManager.Instance.AllocateAndRegisterAsyncTask();

        try
        {
            TlsSocketManager.Instance.TcpConnect(host, port, taskId, sniHost);
            return Value.CreateInteger(taskId);
        }
        catch (Exception ex)
        {
            TaskManager.Instance.CompleteWithFault(taskId, ex);
            return Value.CreateInteger(taskId);
        }
    }

    private static Value Accept(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, TlsSocketBuiltin.Accept, 1, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, TlsSocketBuiltin.Accept, 0, args[0]);
        long serverId = args[0].GetInteger();
        long taskId = Mgr.EnqueueAccept(serverId);
        return Value.CreateInteger(taskId);
    }

    private static Value Send(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, TlsSocketBuiltin.Send, 2, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, TlsSocketBuiltin.Send, 0, args[0]);
        ParameterTypeMismatchError.ExpectBytes(token, TlsSocketBuiltin.Send, 1, args[1]);

        long sockId = args[0].GetInteger();
        byte[] data = args[1].GetBytes();

        long taskId = Mgr.EnqueueSend(sockId, data);
        return Value.CreateInteger(taskId);
    }

    private static Value Recv(Token token, List<Value> args)
    {
        int expected = args.Count == 1 ? 1 : 2;
        ParameterCountMismatchError.Check(token, TlsSocketBuiltin.Receive, expected, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, TlsSocketBuiltin.Receive, 0, args[0]);
        long sockId = args[0].GetInteger();
        int max = args.Count > 1 ? (int)args[1].GetInteger() : 4096;
        long taskId = Mgr.EnqueueRecv(sockId, max);
        return Value.CreateInteger(taskId);
    }

    private static Value Close(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, TlsSocketBuiltin.Close, 1, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, TlsSocketBuiltin.Close, 0, args[0]);

        long sockId = args[0].GetInteger();
        Mgr.Close(sockId);
        return Value.Default;
    }
}