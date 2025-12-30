using System.Collections.Concurrent;
using System.Net;
using System.Net.Sockets;
using kiwi.Typing;

namespace kiwi.Runtime;

public sealed class SocketManager
{
    public static readonly SocketManager Instance = new();

    private readonly ConcurrentDictionary<long, SocketState> _sockets = new();
    private readonly ConcurrentDictionary<Socket, long> _socketToId = new(); // Fast reverse lookup
    private long _nextId = 1;

    private readonly List<Socket> _readCheck = new(1024);
    private readonly List<Socket> _writeCheck = new(1024);
    private readonly List<Socket> _errorCheck = new(1024);

    private volatile bool _running;
    private Thread _loopThread;
    private readonly object _lock = new();

    private SocketManager() { }

    public void Start()
    {
        if (_running) return;
        _running = true;
        _loopThread = new Thread(RunLoop) { IsBackground = true };
        _loopThread.Start();
    }

    public void Stop()
    {
        _running = false;
        _loopThread?.Join();

        lock (_lock)
        {
            foreach (var state in _sockets.Values.ToList())
            {
                FaultAllPendingOps(state, new SocketException((int)SocketError.Interrupted));
                try { state.Socket.Shutdown(SocketShutdown.Both); } catch { }
                try { state.Socket.Close(); } catch { }
            }
            _sockets.Clear();
            _socketToId.Clear();
        }
    }

    public long TcpServer(string host, int port, int backlog = 128)
    {
        var listener = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
        listener.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, true);
        var ip = string.IsNullOrEmpty(host) ? IPAddress.Any : Dns.GetHostAddresses(host)[0];
        listener.Bind(new IPEndPoint(ip, port));
        listener.Listen(backlog);
        listener.Blocking = false;

        long id = Interlocked.Increment(ref _nextId);
        var state = new SocketState { Id = id, Socket = listener, Role = SocketRole.Listener };

        lock (_lock)
        {
            _sockets[id] = state;
            _socketToId[listener] = id;
        }

        return id;
    }

    public long TcpConnect(string host, int port)
    {
        var socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
        socket.Blocking = false;

        long id = Interlocked.Increment(ref _nextId);
        var state = new SocketState { Id = id, Socket = socket, Role = SocketRole.Client };

        lock (_lock)
        {
            _sockets[id] = state;
            _socketToId[socket] = id;

            long taskId = TaskManager.Instance.AllocateAndRegisterAsyncTask();
            state.ConnectTaskId = taskId;

            try
            {
                socket.Connect(host, port);
                // Immediate success
                TaskManager.Instance.Complete(taskId, Value.CreateInteger(id));
            }
            catch (SocketException ex) when (ex.SocketErrorCode == SocketError.WouldBlock ||
                                             ex.SocketErrorCode == SocketError.InProgress)
            {
                Console.Error.WriteLine($"TcpConnect: {ex.Message}");
                // Connection in progress — will complete on writable
                state.PendingOps.Add(new PendingOp { Type = OpType.Connect, TaskId = taskId });
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"TcpConnect: {ex.Message}");
                TaskManager.Instance.CompleteWithFault(taskId, ex);
            }
        }

        return id;
    }

    public long EnqueueAccept(long serverId)
    {
        long taskId = TaskManager.Instance.AllocateAndRegisterAsyncTask();

        lock (_lock)
        {
            if (_sockets.TryGetValue(serverId, out var state))
            {
                state.PendingOps.Add(new PendingOp { Type = OpType.Accept, TaskId = taskId });
            }
            else
            {
                Console.Error.WriteLine("EnqueueAccept: Invalid server socket");
                TaskManager.Instance.CompleteWithFault(taskId, new Exception("Invalid server socket"));
            }
        }

        return taskId;
    }

    public long EnqueueRecv(long sockId, int maxBytes = 4096)
    {
        long taskId = TaskManager.Instance.AllocateAndRegisterAsyncTask();

        lock (_lock)
        {
            if (_sockets.TryGetValue(sockId, out var state))
            {
                if (state.PendingOps.Count > 32)
                    throw new InvalidOperationException("Too many pending operations");

                state.PendingOps.Add(new PendingOp { Type = OpType.Recv, TaskId = taskId, MaxBytes = maxBytes });
            }
            else
            {
                Console.Error.WriteLine("EnqueueRecv: Invalid socket");
                TaskManager.Instance.CompleteWithFault(taskId, new Exception("Invalid socket"));
            }
        }

        return taskId;
    }

    public long EnqueueSend(long sockId, byte[] data)
    {
        long taskId = TaskManager.Instance.AllocateAndRegisterAsyncTask();

        lock (_lock)
        {
            if (_sockets.TryGetValue(sockId, out var state))
            {
                state.PendingOps.Add(new PendingOp
                {
                    Type = OpType.Send,
                    TaskId = taskId,
                    Data = data
                });
            }
            else
            {
                TaskManager.Instance.CompleteWithFault(taskId, new Exception("Invalid socket"));
            }
        }

        return taskId;
    }

    public void Close(long sockId)
    {
        lock (_lock)
        {
            if (_sockets.TryRemove(sockId, out var state))
            {
                _socketToId.TryRemove(state.Socket, out _);
                FaultAllPendingOps(state, new SocketException((int)SocketError.ConnectionAborted));

                try { state.Socket.Shutdown(SocketShutdown.Both); } catch { }
                try { state.Socket.Close(); } catch { }
            }
        }
    }

    private void RunLoop()
    {
        while (_running)
        {
            BuildSelectLists();

            if (_readCheck.Count == 0 && _writeCheck.Count == 0 && _errorCheck.Count == 0)
            {
                Thread.Sleep(1);
                continue;
            }

            try
            {
                Socket.Select(_readCheck, _writeCheck, _errorCheck, 100000);
            }
            catch
            {
                // Select can throw on shutdown
                continue;
            }

            ProcessErrors();
            ProcessWrites();
            ProcessReads();
        }
    }

    private void BuildSelectLists()
    {
        lock (_lock)
        {
            _readCheck.Clear();
            _writeCheck.Clear();
            _errorCheck.Clear();

            foreach (var state in _sockets.Values)
            {
                bool needsRead = state.Role == SocketRole.Listener ||
                                 state.PendingOps.Any(p => p.Type == OpType.Recv);
                bool needsWrite = state.PendingOps.Any(p => p.Type == OpType.Send || p.Type == OpType.Connect);

                if (needsRead) _readCheck.Add(state.Socket);
                if (needsWrite) _writeCheck.Add(state.Socket);
                _errorCheck.Add(state.Socket);
            }
        }
    }

    private void ProcessReads()
    {
        foreach (var sock in _readCheck)
        {
            if (!_socketToId.TryGetValue(sock, out long id) || !_sockets.TryGetValue(id, out var state)) continue;

            if (state.Role == SocketRole.Listener)
                HandleAccept(state);
            else
                HandleRecv(state);
        }
    }

    private void ProcessWrites()
    {
        foreach (var sock in _writeCheck)
        {
            if (!_socketToId.TryGetValue(sock, out long id) || !_sockets.TryGetValue(id, out var state)) continue;

            if (state.ConnectTaskId.HasValue)
                HandleConnectComplete(state);
            else
                HandleSend(state);
        }
    }

    private void ProcessErrors()
    {
        foreach (var sock in _errorCheck)
        {
            if (!_socketToId.TryGetValue(sock, out long id) || !_sockets.TryGetValue(id, out var state)) continue;

            FaultAllPendingOps(state, new SocketException((int)SocketError.ConnectionReset));
        }
    }

    private void HandleAccept(SocketState state)
    {
        // Collect pending accepts once (safe snapshot)
        var pendingAccepts = new List<PendingOp>(state.PendingOps.Where(p => p.Type == OpType.Accept));

        if (pendingAccepts.Count == 0) return;

        int completedCount = 0;

        while (completedCount < pendingAccepts.Count)
        {
            try
            {
                var client = state.Socket.Accept();
                client.Blocking = false;

                long clientId = Interlocked.Increment(ref _nextId);
                var clientState = new SocketState { Id = clientId, Socket = client, Role = SocketRole.Client };

                lock (_lock)
                {
                    _sockets[clientId] = clientState;
                    _socketToId[client] = clientId;
                }

                // Complete the next pending accept task
                var op = pendingAccepts[completedCount];
                TaskManager.Instance.Complete(op.TaskId, Value.CreateInteger(clientId));

                completedCount++;
            }
            catch (SocketException ex) when (ex.SocketErrorCode == SocketError.WouldBlock)
            {
                Console.Error.WriteLine($"HandleAccept: {ex.Message}");
                // No more connections ready right now
                break;
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"HandleAccept: {ex.Message}");
                // Fault remaining accepts
                for (int i = completedCount; i < pendingAccepts.Count; i++)
                {
                    TaskManager.Instance.CompleteWithFault(pendingAccepts[i].TaskId, ex);
                }
                break;
            }
        }

        // Remove only the completed ones
        lock (_lock)
        {
            for (int i = 0; i < completedCount; i++)
            {
                state.PendingOps.Remove(pendingAccepts[i]);
            }
        }
    }

    private void HandleRecv(SocketState state)
    {
        var recvs = state.PendingOps.Where(p => p.Type == OpType.Recv).ToList();
        foreach (var op in recvs)
        {
            try
            {
                byte[] buffer = new byte[op.MaxBytes];
                int read = state.Socket.Receive(buffer);
                byte[] data = read > 0 ? buffer.AsSpan(0, read).ToArray() : Array.Empty<byte>();

                TaskManager.Instance.Complete(op.TaskId, Value.CreateBytes(data));
            }
            catch (SocketException ex) when (ex.SocketErrorCode == SocketError.WouldBlock)
            {
                Console.Error.WriteLine($"HandleRecv: {ex.Message}");
                return; // Keep pending
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"HandleRecv: {ex.Message}");
                TaskManager.Instance.CompleteWithFault(op.TaskId, ex);
            }

            state.PendingOps.Remove(op);
        }
    }

    private void HandleSend(SocketState state)
    {
        var sends = state.PendingOps.Where(p => p.Type == OpType.Send).ToList();
        foreach (var op in sends)
        {
            try
            {
                if (op.Data == null)
                {
                    continue;
                }

                int sent = state.Socket.Send(op.Data);
                if (sent == op.Data.Length)
                {
                    TaskManager.Instance.Complete(op.TaskId, Value.Default);
                }
                else
                {
                    return;
                }
            }
            catch (SocketException ex) when (ex.SocketErrorCode == SocketError.WouldBlock)
            {
                Console.Error.WriteLine($"HandleSend: {ex.Message}");
                return;
            }
            catch (Exception ex)
            {
                Console.Error.WriteLine($"HandleSend: {ex.Message}");
                TaskManager.Instance.CompleteWithFault(op.TaskId, ex);
            }

            state.PendingOps.Remove(op);
        }
    }

    private void HandleConnectComplete(SocketState state)
    {
        if (state.ConnectTaskId.HasValue)
        {
            TaskManager.Instance.Complete(state.ConnectTaskId.Value, Value.CreateInteger(state.Id));
            state.ConnectTaskId = null;
            state.PendingOps.RemoveAll(p => p.Type == OpType.Connect);
        }
    }

    private void FaultAllPendingOps(SocketState state, Exception ex)
    {
        foreach (var op in state.PendingOps)
        {
            TaskManager.Instance.CompleteWithFault(op.TaskId, ex);
        }
        state.PendingOps.Clear();
    }
}

public class SocketState
{
    public long Id { get; init; }
    public Socket Socket { get; init; } = null!;
    public SocketRole Role { get; init; }
    public List<PendingOp> PendingOps { get; set; } = new();
    public long? ConnectTaskId { get; set; }
}

public class PendingOp
{
    public OpType Type;
    public long TaskId;
    public int MaxBytes;
    public byte[]? Data;
}

public enum OpType { Accept, Recv, Send, Connect }
public enum SocketRole { Listener, Client }