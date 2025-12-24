using System.Collections.Concurrent;
using System.Net;
using System.Net.Sockets;
using kiwi.Parsing;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime;

public sealed class SocketManager
{
    private long _nextId = 1;

    private readonly ConcurrentDictionary<long, SocketState> _sockets = new();

    public static SocketManager Instance { get; } = new();

    private SocketManager() { }

    public long TcpServer(string host, int port, int backlog = 128)
    {
        var listener = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
        var ip = string.IsNullOrEmpty(host) ? IPAddress.Any : IPAddress.Parse(host);
        listener.Bind(new IPEndPoint(ip, port));
        listener.Listen(backlog);

        var state = new SocketState
        {
            Id = Interlocked.Increment(ref _nextId),
            Socket = listener,
            Role = SocketRole.Listener,
            AcceptChannel = Channel.Create(0) // unbuffered accept queue
        };

        _sockets[state.Id] = state;
        StartAcceptLoop(state);

        return state.Id;
    }

    public long TcpConnect(string host, int port)
    {
        var socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
        socket.Connect(host, port);

        return CreateClientState(socket).Id;
    }

    private SocketState CreateClientState(Socket socket)
    {
        var state = new SocketState
        {
            Id = Interlocked.Increment(ref _nextId),
            Socket = socket,
            Role = SocketRole.Client
        };

        _sockets[state.Id] = state;

        StartReceiveLoop(state);
        StartWriteLoop(state);

        return state;
    }

    private void StartAcceptLoop(SocketState listenerState)
    {
        ThreadPool.UnsafeQueueUserWorkItem(_ =>
        {
            var eof = Token.Eof;
            var state = listenerState;
            try
            {
                while (true)
                {
                    var clientSocket = state.Socket.Accept();
                    var clientState = CreateClientState(clientSocket);
                    state.AcceptChannel!.Send(eof, Value.CreateInteger(clientState.Id));
                }
            }
            catch (ObjectDisposedException) { /* closed */ }
            catch (SocketException) { /* closed */ }
            finally
            {
                state.AcceptChannel!.Close();
            }
        }, null);
    }

    private void StartReceiveLoop(SocketState state)
    {
        ThreadPool.UnsafeQueueUserWorkItem(_ =>
        {
            var eof = Token.Eof;
            var socket = state.Socket;
            var buffer = new byte[8192]; // TODO: pool later

            try
            {
                while (true)
                {
                    int bytesRead = socket.Receive(buffer, SocketFlags.None);

                    if (bytesRead == 0)
                    {
                        // remote closed connection
                        break;
                    }

                    // Create exact-sized byte array (no extra zeros)
                    var data = new byte[bytesRead];
                    Buffer.BlockCopy(buffer, 0, data, 0, bytesRead);

                    // blocks if channel buffer full, natural backpressure
                    state.ReadChannel.Send(eof, Value.CreateBytes(data));
                }
            }
            catch (ObjectDisposedException) { /* socket closed */ }
            catch (SocketException) { /* connection reset, timeout, etc. */ }
            finally
            {
                // Signal end-of-stream to script side
                state.ReadChannel.Close();
            }
        }, null);
    }

    private void StartWriteLoop(SocketState state)
    {
        ThreadPool.UnsafeQueueUserWorkItem(_ =>
        {
            try
            {
                var eof = Token.Eof;
                var socket = state.Socket;
                
                while (true)
                {
                    Value dataVal = state.WriteChannel.Receive(eof);
                    if (!dataVal.IsBytes())
                    {
                        continue; // safety – should never happen
                    }

                    byte[] data = dataVal.GetBytes();
                    socket.Send(data, SocketFlags.None);
                }
            }
            catch (ObjectDisposedException) { /* closed */ }
            catch (SocketException) { /* closed */ }
            catch (InvalidOperationError) when (state.WriteChannel.Closed) { /* channel closed */ }
            finally
            {
                try { state.Socket.Shutdown(SocketShutdown.Send); }
                catch { }
            }
        }, null);
    }

    public void Close(long id)
    {
        if (_sockets.TryRemove(id, out var state))
        {
            try { state.Socket.Shutdown(SocketShutdown.Both); }
            catch { /* ignore */ }

            state.Socket.Close();

            state.ReadChannel.Close();
            state.WriteChannel.Close();
            state.AcceptChannel?.Close();
        }
    }
}