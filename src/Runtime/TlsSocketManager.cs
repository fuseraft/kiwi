using System.Collections.Concurrent;
using System.Net;
using System.Net.Security;
using System.Net.Sockets;
using System.Runtime.InteropServices;
using System.Security.Authentication;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using kiwi.Typing;

namespace kiwi.Runtime;

public sealed class TlsSocketManager
{
    const string DN = "cn=kiwi.local";
    public static readonly TlsSocketManager Instance = new();

    private readonly ConcurrentDictionary<long, TlsState> _sockets = new();
    private readonly ConcurrentDictionary<Socket, long> _socketToId = new();
    private long _nextId = 1;

    private readonly List<Socket> _readCheck = new(1024);
    private readonly List<Socket> _writeCheck = new(1024);
    private readonly List<Socket> _errorCheck = new(1024);

    private volatile bool _running;
    private Thread _loopThread;
    private readonly object _lock = new();

    private readonly X509Certificate2 _serverCert;

    private TlsSocketManager()
    {
        _serverCert = GenerateSelfSignedCertificate();
    }

    private static X509Certificate2 GenerateSelfSignedCertificate()
    {
        using var rsa = RSA.Create(2048);
        var req = new CertificateRequest(DN, rsa, HashAlgorithmName.SHA256, RSASignaturePadding.Pkcs1);
        return req.CreateSelfSigned(DateTimeOffset.Now, DateTimeOffset.Now.AddYears(1));
    }

    private static List<PendingOp> SnapshotOps(TlsState state, OpType type)
    {
        return [.. state.PendingOps.Where(p => p.Type == type)];
    }

    public void Start()
    {
        if (_running)
        {
            return;
        }
        
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
                try { state.SslStream?.Close(); } catch { }
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
        var state = new TlsState { Id = id, Socket = listener, Role = SocketRole.Listener };

        lock (_lock)
        {
            _sockets[id] = state;
            _socketToId[listener] = id;
        }

        return id;
    }

    public void TcpConnect(string host, int port, long taskId, string? expectedHost = null)
    {
        var socket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp)
        {
            Blocking = false
        };

        long id = Interlocked.Increment(ref _nextId);
        var state = new TlsState
        {
            Id = id,
            Socket = socket,
            Role = SocketRole.Client,
            Host = host,
            SniHost = expectedHost ?? host,
            IsServer = false,
            ConnectTaskId = taskId
        };

        lock (_lock)
        {
            _sockets[id] = state;
            _socketToId[socket] = id;
            state.PendingOps.Add(new PendingOp { Type = OpType.Connect, TaskId = taskId });
        }

        try
        {
            state.PendingAsyncResult = socket.BeginConnect(host, port, null, null);
        }
        catch (Exception ex)
        {
            TaskManager.Instance.CompleteWithFault(taskId, ex);
            lock (_lock)
            {
                state.PendingOps.RemoveAll(p => p.TaskId == taskId);
            }
        }
    }

    public long EnqueueAccept(long serverId)
    {
        if (!_sockets.TryGetValue(serverId, out var state) || state.Role != SocketRole.Listener)
        {
            throw new InvalidOperationException($"Socket {serverId} is not a listening TLS server socket");
        }

        long taskId = TaskManager.Instance.AllocateAndRegisterAsyncTask();

        var op = new PendingOp
        {
            Type = OpType.Accept,
            TaskId = taskId,
            AsyncResult = null
        };

        lock (_lock)
        {
            state.PendingOps.Add(op);
        }

        return taskId;
    }

    public long EnqueueRecv(long socketId, int maxBytes = 4096)
    {
        maxBytes = Math.Max(maxBytes, 4096);

        if (!_sockets.TryGetValue(socketId, out var state) || !state.Handshaked)
        {
            throw new InvalidOperationException($"Socket {socketId} is not in a connected/handshaked TLS state");
        }

        long taskId = TaskManager.Instance.AllocateAndRegisterAsyncTask();

        var op = new PendingOp
        {
            Type = OpType.Recv,
            TaskId = taskId,
            MaxBytes = maxBytes,
            Buffer = new byte[maxBytes],
            AsyncResult = null
        };

        lock (_lock)
        {
            state.PendingOps.Add(op);
        }

        return taskId;
    }

    public long EnqueueSend(long socketId, byte[] data)
    {
        if (data == null || data.Length == 0)
        {
            throw new ArgumentException("Cannot send empty data", nameof(data));
        }

        if (!_sockets.TryGetValue(socketId, out var state))
        {
            throw new InvalidOperationException($"Socket {socketId} not found");
        }

        long taskId = TaskManager.Instance.AllocateAndRegisterAsyncTask();

        var op = new PendingOp
        {
            Type = OpType.Send,
            TaskId = taskId,
            Data = data,
            Offset = 0,
            AsyncResult = null
        };

        lock (_lock)
        {
            state.PendingOps.Add(op);
        }

        return taskId;
    }

    public void Close(long id)
    {
        if (!_sockets.TryGetValue(id, out var state))
        {
            return;
        }

        FaultAllPendingOps(state, new SocketException((int)SocketError.OperationAborted));

        try { state.SslStream?.Close(); } catch { }
        try { state.Socket.Shutdown(SocketShutdown.Both); } catch { }
        try { state.Socket.Close(); } catch { }

        lock (_lock)
        {
            _sockets.TryRemove(id, out _);
            _socketToId.TryRemove(state.Socket, out _);
        }
    }

    private void RunLoop()
    {
        const int TimeoutMilliseconds = 100;
        const int SelectLoopTimeoutMicroseconds = 100000; // 100 ms timeout
        
        while (_running)
        {
            lock (_lock)
            {
                _readCheck.Clear();
                _writeCheck.Clear();
                _errorCheck.Clear();

                foreach (var state in _sockets.Values)
                {
                    if (state.Socket == null)
                    {
                        continue;
                    }

                    bool handshakePending = state.PendingAsyncResult != null && !state.Handshaked;

                    if (state.Role == SocketRole.Listener)
                    {
                        if (state.PendingOps.Any(p => p.Type == OpType.Accept))
                        {
                            _readCheck.Add(state.Socket);
                        }
                    }
                    else
                    {
                        bool hasRecv = state.PendingOps.Any(p => p.Type == OpType.Recv);
                        bool hasSend = state.PendingOps.Any(p => p.Type == OpType.Send);
                        bool hasConnect = state.PendingOps.Any(p => p.Type == OpType.Connect);

                        if (hasRecv || handshakePending || hasConnect)
                        {
                            _readCheck.Add(state.Socket);
                        }

                        if (hasSend || handshakePending || hasConnect)
                        {
                            bool wantsWrite = hasSend || state.PendingAsyncResult != null;

                            if (wantsWrite)
                            {
                                _writeCheck.Add(state.Socket);
                            }
                        }

                        if (hasRecv || hasSend || handshakePending || hasConnect)
                        {
                            _errorCheck.Add(state.Socket);
                        }
                    }
                }
            }

            if (_readCheck.Count == 0 && _writeCheck.Count == 0 && _errorCheck.Count == 0)
            {
                Thread.Sleep(TimeoutMilliseconds);
                continue;
            }

            Socket.Select(_readCheck, _writeCheck, _errorCheck, SelectLoopTimeoutMicroseconds);

            lock (_lock)
            {
                foreach (var socket in _errorCheck)
                {
                    if (_socketToId.TryGetValue(socket, out long id) && _sockets.TryGetValue(id, out var state))
                    {
                        FaultAllPendingOps(state, new SocketException((int)SocketError.SocketError));
                        Close(id);
                    }
                }

                foreach (var socket in _readCheck)
                {
                    if (_socketToId.TryGetValue(socket, out long id) && _sockets.TryGetValue(id, out var state))
                    {
                        if (state.Role == SocketRole.Listener)
                        {
                            HandleAccept(state);
                        }
                        else
                        {
                            HandleRecv(state);
                        }
                    }
                }

                foreach (var socket in _writeCheck)
                {
                    if (_socketToId.TryGetValue(socket, out long id) && _sockets.TryGetValue(id, out var state))
                    {
                        if (state.Role != SocketRole.Listener)
                        {
                            HandleConnectComplete(state);
                            HandleSend(state);
                        }
                    }
                }
            }
        }
    }

    private void HandleAccept(TlsState state)
    {
        List<PendingOp> ops = SnapshotOps(state, OpType.Accept);
        foreach (var op in ops)
        {
            Socket? client = null;
            try
            {
                client = state.Socket.Accept();
            }
            catch (SocketException ex) when (ex.SocketErrorCode == SocketError.WouldBlock)
            {
                return;
            }

            if (client == null)
            {
                return;
            }

            client.Blocking = false;

            long clientId = Interlocked.Increment(ref _nextId);

            var networkStream = new NetworkStream(client, true)
            {
                ReadTimeout = 0,
                WriteTimeout = 0
            };

            var sslStream = new SslStream(networkStream, false, (sender, cert, chain, errors) => {
                if (errors == SslPolicyErrors.None)
                {
                    return true;
                }

                // Only continue if the only problem is that it's self-signed or untrusted
                if ((errors & ~SslPolicyErrors.RemoteCertificateChainErrors) == SslPolicyErrors.None)
                {
                    // WIP: thumbprint, subject name, validity period, issuer, etc
                    return true;
                }

                return false;
            });

            var clientState = new TlsState
            {
                Id = clientId,
                Socket = client,
                Role = SocketRole.Client,
                SslStream = sslStream,
                IsServer = true,
                ConnectTaskId = op.TaskId,
                PendingAsyncResult = sslStream.BeginAuthenticateAsServer(_serverCert, null, null)
            };

            lock (_lock)
            {
                _sockets[clientId] = clientState;
                _socketToId[client] = clientId;
            }
        }
    }

    private void HandleConnectComplete(TlsState state)
    {
        if (!state.ConnectTaskId.HasValue)
        {
            return;
        }

        if (state.PendingAsyncResult == null)
        {
            return;
        }

        if (state.Handshaked)
        {
            return;
        }

        try
        {
            state.Socket.EndConnect(state.PendingAsyncResult);
            state.PendingAsyncResult = null;
        }
        catch (Exception ex)
        {
            TaskManager.Instance.CompleteWithFault(state.ConnectTaskId.Value, ex);
            state.ConnectTaskId = null;
            Close(state.Id);
            return;
        }

        try
        {
            // temporarily set blocking for synchronous handshake
            state.Socket.Blocking = true;

            var networkStream = new NetworkStream(state.Socket, ownsSocket: true)
            {
                ReadTimeout = Timeout.Infinite,
                WriteTimeout = Timeout.Infinite
            };

            state.SslStream = new SslStream(
                networkStream,
                leaveInnerStreamOpen: false,
                userCertificateValidationCallback: ValidateServerCertificate
            );

            string targetHost = state.SniHost ?? state.Host;

            var options = new SslClientAuthenticationOptions
            {
                TargetHost = targetHost,
                EnabledSslProtocols = SslProtocols.Tls12, // | SslProtocols.Tls13,
                ApplicationProtocols =
                [
                    SslApplicationProtocol.Http11
                ]
            };
            
            if (!RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
            {
                var allowedCipherSuites = Enum.GetValues<TlsCipherSuite>();
                options.CipherSuitesPolicy = new CipherSuitesPolicy(allowedCipherSuites);
            }

            var cts = new CancellationTokenSource(TimeSpan.FromSeconds(15));

            // blocks briefly but completes fully
            state.SslStream.AuthenticateAsClient(options);

            state.Handshaked = true;
            state.IsReadyForAppData = true;
            state.PendingAsyncResult = null;

            // restore non-blocking for polling
            state.Socket.Blocking = false;

            if (state.ConnectTaskId.HasValue)
            {
                TaskManager.Instance.Complete(state.ConnectTaskId.Value, Value.CreateInteger(state.Id));
                state.ConnectTaskId = null;
            }
        }
        catch (OperationCanceledException)
        {
            throw new TimeoutException("TLS handshake timed out");
        }
        catch (Exception ex)
        {
            state.Socket.Blocking = false;

            if (state.ConnectTaskId.HasValue)
            {
                TaskManager.Instance.CompleteWithFault(state.ConnectTaskId.Value, ex);
                state.ConnectTaskId = null;
            }

            state.PendingAsyncResult = null;
            Close(state.Id);
        }
    }

    private void HandleRecv(TlsState state)
    {
        if (!state.Handshaked)
        {
            return;
        }

        if (state.SslStream == null)
        {
            return;
        }

        List<PendingOp> ops = SnapshotOps(state, OpType.Recv);
        foreach (var op in ops)
        {
            if (state.SslStream == null || !state.Handshaked)
            {
                continue;
            }

            if (op.AsyncResult == null)
            {
                op.Buffer = new byte[op.MaxBytes];
                op.AsyncResult = state.SslStream.BeginRead(op.Buffer, 0, op.MaxBytes, null, null);
            }

            if (!op.AsyncResult.IsCompleted)
            {
                continue;
            }

            try
            {
                int bytes = state.SslStream.EndRead(op.AsyncResult);
                byte[] data = bytes == 0 ? Array.Empty<byte>() : op.Buffer![..bytes];
                TaskManager.Instance.Complete(op.TaskId, Value.CreateBytes(data));
            }
            catch (Exception ex)
            {
                TaskManager.Instance.CompleteWithFault(op.TaskId, ex);
            }

            lock (_lock)
            {
                state.PendingOps.Remove(op);
            }
        }
    }

    private void HandleSend(TlsState state)
    {
        if (state.SslStream == null)
        {
            return;
        }

        if (!state.IsReadyForAppData)
        {
            return;
        }

        if (!state.SslStream.IsAuthenticated)
        {
            return;
        }

        var ops = SnapshotOps(state, OpType.Send);

        foreach (var op in ops)
        {
            if (op.AsyncResult == null)
            {
                try
                {
                    op.AsyncResult = state.SslStream.BeginWrite(
                        op.Data!,
                        op.Offset,
                        op.Data!.Length - op.Offset,
                        null,
                        null
                    );
                }
                catch (Exception ex)
                {
                    TaskManager.Instance.CompleteWithFault(op.TaskId, ex);
                    lock (_lock) { state.PendingOps.Remove(op); }
                    continue;
                }
            }

            if (!op.AsyncResult.IsCompleted)
            {
                continue;
            }

            try
            {
                state.SslStream.EndWrite(op.AsyncResult);
                TaskManager.Instance.Complete(op.TaskId, Value.Default);
            }
            catch (Exception ex)
            {
                TaskManager.Instance.CompleteWithFault(op.TaskId, ex);
            }
            finally
            {
                lock (_lock) { state.PendingOps.Remove(op); }
            }
        }
    }

    private static bool ValidateServerCertificate(object sender, X509Certificate? certificate, X509Chain? chain, SslPolicyErrors sslPolicyErrors)
    {
        if (sslPolicyErrors == SslPolicyErrors.None)
        {
            return true;
        }

        /* TODO: return this to the programmer
        List<Value> errors = [];

        if ((sslPolicyErrors & SslPolicyErrors.RemoteCertificateNotAvailable) != 0)
        {
            errors.Add(Value.CreateString("No certificate from server"));
        }
        
        if ((sslPolicyErrors & SslPolicyErrors.RemoteCertificateNameMismatch) != 0)
        {
            errors.Add(Value.CreateString("Certificate name mismatch"));
        }

        if ((sslPolicyErrors & SslPolicyErrors.RemoteCertificateChainErrors) != 0)
        {
            errors.Add(Value.CreateString("Chain validation failed"));
        }
        */

        return false;
    }

    private void FaultAllPendingOps(TlsState state, Exception ex)
    {
        List<PendingOp> ops;
        lock (_lock)
        {
            ops = [.. state.PendingOps];
            state.PendingOps.Clear();
        }

        foreach (var op in ops)
        {
            TaskManager.Instance.CompleteWithFault(op.TaskId, ex);
        }
    }
}

public class TlsState : SocketState
{
    public string Host { get; set; } = string.Empty;
    public string? SniHost { get; set; } = string.Empty;
    public SslStream? SslStream { get; set; }
    public IAsyncResult? PendingAsyncResult { get; set; }
    public bool Handshaked { get; set; }
    public bool IsReadyForAppData { get; set; } = false;
    public bool IsServer { get; set; }
}
