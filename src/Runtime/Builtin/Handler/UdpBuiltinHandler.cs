using System.Collections.Concurrent;
using System.Net;
using System.Net.Sockets;
using System.Text;
using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class UdpBuiltinHandler
{
    private static long _nextId;
    private static readonly ConcurrentDictionary<long, UdpClient> _sockets = new();

    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Udp_Create     => Create(token, args),
            TokenName.Builtin_Udp_Bind       => Bind(token, args),
            TokenName.Builtin_Udp_Send       => Send(token, args),
            TokenName.Builtin_Udp_Recv       => Recv(token, args),
            TokenName.Builtin_Udp_SetTimeout => SetTimeout(token, args),
            TokenName.Builtin_Udp_Close      => Close(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }

    private static Value Create(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, UdpBuiltin.Create, 0, args.Count);
        var id = Interlocked.Increment(ref _nextId);
        _sockets[id] = new UdpClient();
        return Value.CreateInteger(id);
    }

    private static Value Bind(Token token, List<Value> args)
    {
        if (args.Count < 2 || args.Count > 3)
            throw new ParameterCountMismatchError(token, UdpBuiltin.Bind, args.Count, [2, 3]);

        ParameterTypeMismatchError.ExpectInteger(token, UdpBuiltin.Bind, 0, args[0]);
        ParameterTypeMismatchError.ExpectInteger(token, UdpBuiltin.Bind, 1, args[1]);

        var id   = args[0].GetInteger();
        var port = (int)args[1].GetInteger();
        var host = args.Count == 3 && args[2].IsString() ? args[2].GetString() : "0.0.0.0";

        if (!_sockets.ContainsKey(id))
            throw new InvalidOperationError(token, $"No UDP socket with id {id}.");

        // Replace with a bound client
        _sockets[id].Close();
        _sockets[id] = new UdpClient(new IPEndPoint(IPAddress.Parse(host), port));
        return Value.Default;
    }

    private static Value Send(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, UdpBuiltin.Send, 4, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, UdpBuiltin.Send, 0, args[0]);
        ParameterTypeMismatchError.ExpectString(token, UdpBuiltin.Send, 1, args[1]);
        ParameterTypeMismatchError.ExpectInteger(token, UdpBuiltin.Send, 2, args[2]);

        var id   = args[0].GetInteger();
        var host = args[1].GetString();
        var port = (int)args[2].GetInteger();

        byte[] data;
        if (args[3].IsBytes())
            data = args[3].GetBytes();
        else if (args[3].IsString())
            data = Encoding.UTF8.GetBytes(args[3].GetString());
        else
            throw new InvalidOperationError(token, "UDP send requires bytes or string data.");

        if (!_sockets.TryGetValue(id, out var client))
            throw new InvalidOperationError(token, $"No UDP socket with id {id}.");

        int sent = client.Send(data, data.Length, host, port);
        return Value.CreateInteger(sent);
    }

    private static Value Recv(Token token, List<Value> args)
    {
        if (args.Count < 1 || args.Count > 2)
            throw new ParameterCountMismatchError(token, UdpBuiltin.Recv, args.Count, [1, 2]);

        ParameterTypeMismatchError.ExpectInteger(token, UdpBuiltin.Recv, 0, args[0]);
        var id = args[0].GetInteger();

        if (!_sockets.TryGetValue(id, out var client))
            throw new InvalidOperationError(token, $"No UDP socket with id {id}.");

        try
        {
            var remoteEP = new IPEndPoint(IPAddress.Any, 0);
            var data     = client.Receive(ref remoteEP);

            var result = new Dictionary<Value, Value>
            {
                [Value.CreateString("data")]      = Value.CreateBytes(data),
                [Value.CreateString("from_host")] = Value.CreateString(remoteEP.Address.ToString()),
                [Value.CreateString("from_port")] = Value.CreateInteger(remoteEP.Port),
            };

            return Value.CreateHashmap(result);
        }
        catch (SocketException ex)
        {
            throw new SystemError(token, $"UDP recv error: {ex.Message}");
        }
    }

    private static Value SetTimeout(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, UdpBuiltin.SetTimeout, 2, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, UdpBuiltin.SetTimeout, 0, args[0]);
        ParameterTypeMismatchError.ExpectInteger(token, UdpBuiltin.SetTimeout, 1, args[1]);

        var id = args[0].GetInteger();
        var ms = (int)args[1].GetInteger();

        if (!_sockets.TryGetValue(id, out var client))
            throw new InvalidOperationError(token, $"No UDP socket with id {id}.");

        client.Client.ReceiveTimeout = ms;
        return Value.Default;
    }

    private static Value Close(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, UdpBuiltin.Close, 1, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, UdpBuiltin.Close, 0, args[0]);

        var id = args[0].GetInteger();
        if (_sockets.TryRemove(id, out var client))
            client.Close();

        return Value.Default;
    }
}
