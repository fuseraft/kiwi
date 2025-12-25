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

    public bool TryGetSocket(long serverId, out SocketState? state)
    {
        if (_sockets.TryGetValue(serverId, out var socketState))
        {
            state = socketState;
            return true;
        }
        else
        {
            state = null;
        }

        return false;
    }

    public long TcpServer(string host, int port, int backlog = 128)
    {
        var listener = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
        listener.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, true);
        listener.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ExclusiveAddressUse, false);

        var ip = string.IsNullOrEmpty(host)
            ? IPAddress.Any
            : Dns.GetHostAddresses(host).First(a => a.AddressFamily == AddressFamily.InterNetwork);

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
            var state = listenerState;
            try
            {
                while (true)
                {
                    var clientSocket = state.Socket.Accept();
                    var clientState = CreateClientState(clientSocket);
                    state.AcceptChannel!.Send(Value.CreateInteger(clientState.Id));
                }
            }
            catch (ObjectDisposedException ode)
            {
                Console.Error.WriteLine($"SocketManager.StartAcceptLoop ObjectDisposedException: {ode.Message}");
            }
            catch (SocketException se)
            {
                Console.Error.WriteLine($"SocketManager.StartAcceptLoop SocketException: {se.Message}");
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"SocketManager.StartAcceptLoop Exception: {ex.Message}");
            }
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
            var socket = state.Socket;
            var buffer = new byte[8192]; // fixed-size receive buffer

            try
            {
                while (true)
                {
                    int bytesRead = socket.Receive(buffer, 0, buffer.Length, SocketFlags.None);

                    if (bytesRead == 0)
                    {
                        // remote closed connection
                        break;
                    }

                    // Create exact-sized byte array (no extra zeros)
                    var data = new byte[bytesRead];
                    Buffer.BlockCopy(buffer, 0, data, 0, bytesRead);

                    // blocks if channel buffer full, natural backpressure
                    state.ReadChannel.Send(Value.CreateBytes(data));
                }
            }
            catch (ObjectDisposedException ode)
            {
                Console.Error.WriteLine($"SocketManager.StartReceiveLoop ObjectDisposedException: {ode.Message}");
            }
            catch (SocketException se)
            {
                Console.Error.WriteLine($"SocketManager.StartReceiveLoop SocketException: {se.Message}");
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"SocketManager.StartReceiveLoop Exception: {ex.Message}");
            }
            finally
            {
                // Signal EOF to any waiting recv calls
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
                    try
                    {
                        Value dataVal = state.WriteChannel.Receive(eof);
                        if (!dataVal.IsBytes())
                        {
                            continue; // safety – should never happen
                        }

                        byte[] data = dataVal.GetBytes();
                        int sent = 0;
                        while (sent < data.Length)
                        {
                            sent += socket.Send(data, sent, data.Length - sent, SocketFlags.None);
                        }
                    }
                    catch (InvalidOperationError)
                    {
                        break;
                    }
                }
            }
            catch (ObjectDisposedException ode)
            {
                Console.Error.WriteLine($"SocketManager.StartWriteLoop ObjectDisposedException: {ode.Message}");
            }
            catch (SocketException se) { 
                Console.Error.WriteLine($"SocketManager.StartWriteLoop SocketException: {se.Message}");
            }
            catch (InvalidOperationError) when (state.WriteChannel.Closed) { /* channel closed */ }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"SocketManager.StartWriteLoop Exception: {ex.Message}");
                throw;
            }
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

public sealed class SocketState
{
    // Public init is fine as we control creation.
    public long Id { get; init; }
    public required Socket Socket { get; init; }
    
    public SocketRole Role { get; init; }
    
    // Channels are created at construction time and are never null.
    public Channel ReadChannel { get; init; } = Channel.Create(16);
    public Channel WriteChannel { get; init; } = Channel.Create(16);
    
    // Only listeners have this, so nullable is correct here.
    public Channel? AcceptChannel { get; init; }
}

public enum SocketRole
{
    Listener,
    Client
}
