using System.Collections.Concurrent;
using System.IO.Pipes;
using System.Text;
using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class IpcBuiltinHandler
{
    private static int _nextId = 0;
    private static readonly ConcurrentDictionary<int, Stream> _pipes = new();

    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Ipc_Create  => Create(token, args),
            TokenName.Builtin_Ipc_Connect => Connect(token, args),
            TokenName.Builtin_Ipc_Send    => Send(token, args),
            TokenName.Builtin_Ipc_Recv    => Recv(token, args),
            TokenName.Builtin_Ipc_Close   => Close(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }

    private static Value Create(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, IpcBuiltin.Create, 1, args.Count);
        ParameterTypeMismatchError.ExpectString(token, IpcBuiltin.Create, 0, args[0]);

        var name = args[0].GetString();

        try
        {
            var server = new NamedPipeServerStream(
                name, PipeDirection.InOut, 1,
                PipeTransmissionMode.Byte, PipeOptions.None);
            server.WaitForConnection();
            var id = Interlocked.Increment(ref _nextId);
            _pipes[id] = server;
            return Value.CreateInteger(id);
        }
        catch (Exception ex)
        {
            throw new SystemError(token, $"Failed to create pipe '{name}': {ex.Message}");
        }
    }

    private static Value Connect(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, IpcBuiltin.Connect, 1, args.Count);
        ParameterTypeMismatchError.ExpectString(token, IpcBuiltin.Connect, 0, args[0]);

        var name = args[0].GetString();

        try
        {
            var client = new NamedPipeClientStream(".", name, PipeDirection.InOut);
            client.Connect();
            var id = Interlocked.Increment(ref _nextId);
            _pipes[id] = client;
            return Value.CreateInteger(id);
        }
        catch (Exception ex)
        {
            throw new SystemError(token, $"Failed to connect to pipe '{name}': {ex.Message}");
        }
    }

    private static Value Send(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, IpcBuiltin.Send, 2, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, IpcBuiltin.Send, 0, args[0]);
        ParameterTypeMismatchError.ExpectString(token, IpcBuiltin.Send, 1, args[1]);

        var id      = (int)args[0].GetInteger();
        var message = args[1].GetString();

        if (!_pipes.TryGetValue(id, out var stream))
            throw new SystemError(token, $"No open IPC handle with ID {id}.");

        try
        {
            var payload  = Encoding.UTF8.GetBytes(message);
            var lenBytes = BitConverter.GetBytes(payload.Length);
            if (BitConverter.IsLittleEndian) Array.Reverse(lenBytes);
            stream.Write(lenBytes, 0, 4);
            stream.Write(payload, 0, payload.Length);
            return Value.Default;
        }
        catch (Exception ex)
        {
            throw new SystemError(token, $"Failed to send on IPC handle {id}: {ex.Message}");
        }
    }

    private static Value Recv(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, IpcBuiltin.Recv, 1, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, IpcBuiltin.Recv, 0, args[0]);

        var id = (int)args[0].GetInteger();

        if (!_pipes.TryGetValue(id, out var stream))
            throw new SystemError(token, $"No open IPC handle with ID {id}.");

        try
        {
            var lenBytes = new byte[4];
            ReadExact(stream, lenBytes, 4);
            if (BitConverter.IsLittleEndian) Array.Reverse(lenBytes);
            var len     = BitConverter.ToInt32(lenBytes, 0);
            var payload = new byte[len];
            ReadExact(stream, payload, len);
            return Value.CreateString(Encoding.UTF8.GetString(payload));
        }
        catch (Exception ex)
        {
            throw new SystemError(token, $"Failed to receive on IPC handle {id}: {ex.Message}");
        }
    }

    private static Value Close(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, IpcBuiltin.Close, 1, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, IpcBuiltin.Close, 0, args[0]);

        var id = (int)args[0].GetInteger();

        if (_pipes.TryRemove(id, out var stream))
            stream.Dispose();

        return Value.Default;
    }

    private static void ReadExact(Stream stream, byte[] buf, int count)
    {
        int offset = 0;
        while (offset < count)
        {
            int read = stream.Read(buf, offset, count - offset);
            if (read == 0) throw new EndOfStreamException("Pipe closed unexpectedly.");
            offset += read;
        }
    }
}
