using System.Net.Sockets;
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

        int backlog = args.Count == 3 
            ? (int)ConversionOp.GetInteger(token, args[2], "backlog must be integer")
            : 128;

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

        if (!Mgr.TryGetSocket(serverId, out var state) ||
            state?.Role != SocketRole.Listener ||
            state.AcceptChannel is null)
        {
            throw new InvalidOperationError(token, "Invalid or non-listener socket for accept");
        }

        // Blocks the current task until a new client arrives
        Value clientIdVal = state.AcceptChannel.Receive();
        return clientIdVal;
    }

    private static Value Send(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, SocketBuiltin.Send, 2, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, SocketBuiltin.Send, 0, args[0]);
        ParameterTypeMismatchError.ExpectBytes(token, SocketBuiltin.Send, 1, args[1]);

        long sockId = args[0].GetInteger();

        if (!Mgr.TryGetSocket(sockId, out var state) || state == null)
        {
            throw new InvalidOperationError(token, $"Socket {sockId} not found.");
        }

        state.WriteChannel.Send(args[1]);
        return Value.Default;
    }

    private static Value Recv(Token token, List<Value> args)
    {
        int expected = args.Count == 1 ? 1 : 2;
        ParameterCountMismatchError.Check(token, SocketBuiltin.Receive, expected, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, SocketBuiltin.Receive, 0, args[0]);

        long sockId = args[0].GetInteger();

        if (!Mgr.TryGetSocket(sockId, out var state) || state == null)
        {
            throw new InvalidOperationError(token, $"Socket {sockId} not found.");
        }

        // Optional max bytes parameter
        int maxBytes = args.Count == 2 
            ? (int)ConversionOp.GetInteger(token, args[1], "max_bytes must be integer.")
            : int.MaxValue;

        // Blocks the current task until data arrives or connection closes
        Value dataVal = state.ReadChannel.Receive();

        if (!dataVal.IsBytes())
        {
            return Value.CreateNull(); // should not happen
        }

        byte[] data = dataVal.GetBytes();

        // If caller requested a max and we have more, truncate
        if (data.Length > maxBytes)
        {
            var truncated = new byte[maxBytes];
            Array.Copy(data, truncated, maxBytes);
            return Value.CreateBytes(truncated);
        }

        return dataVal;
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