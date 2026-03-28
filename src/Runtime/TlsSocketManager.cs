using System.Collections.Concurrent;
using System.Net;
using System.Net.Security;
using System.Net.Sockets;
using System.Security.Authentication;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using kiwi.Typing;

namespace kiwi.Runtime;

/// <summary>
/// Async TLS socket manager.
/// All operations use true async I/O (AcceptAsync, AuthenticateAsServerAsync,
/// ReadAsync, WriteAsync) so no thread pool threads are blocked indefinitely.
/// </summary>
public sealed class TlsSocketManager
{
    public static readonly TlsSocketManager Instance = new();

    private readonly ConcurrentDictionary<long, TlsConn> _conns = new();
    private long _nextId = 1;

    private readonly X509Certificate2 _serverCert;

    private TlsSocketManager()
    {
        _serverCert = GenerateSelfSignedCertificate();
    }

    public void Start() { }
    public void Stop()
    {
        foreach (var conn in _conns.Values)
            conn.Dispose();
        _conns.Clear();
    }

    // ── listener ─────────────────────────────────────────────────────────────

    public long TcpServer(string host, int port, int backlog = 128)
    {
        var listener = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
        listener.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReuseAddress, true);
        var ip = string.IsNullOrEmpty(host) ? IPAddress.Any : Dns.GetHostAddresses(host)[0];
        listener.Bind(new IPEndPoint(ip, port));
        listener.Listen(backlog);

        long id = Interlocked.Increment(ref _nextId);
        _conns[id] = new TlsConn { Id = id, ListenerSocket = listener };
        return id;
    }

    // ── accept (server side) ─────────────────────────────────────────────────

    public long EnqueueAccept(long serverId)
    {
        if (!_conns.TryGetValue(serverId, out var server) || server.ListenerSocket == null)
            throw new InvalidOperationException($"TLS socket {serverId} is not a listening server");

        long taskId = TaskManager.Instance.AllocateAndRegisterAsyncTask();
        _ = DoAcceptAsync(taskId, server.ListenerSocket);
        return taskId;
    }

    private async Task DoAcceptAsync(long taskId, Socket listenerSocket)
    {
        try
        {
            Socket raw = await listenerSocket.AcceptAsync().ConfigureAwait(false);
            var netStream = new NetworkStream(raw, ownsSocket: true);
            var ssl = new SslStream(netStream, leaveInnerStreamOpen: false);
            await ssl.AuthenticateAsServerAsync(new SslServerAuthenticationOptions
            {
                ServerCertificate = _serverCert,
                EnabledSslProtocols = SslProtocols.Tls12
            }).ConfigureAwait(false);

            long clientId = Interlocked.Increment(ref _nextId);
            _conns[clientId] = new TlsConn { Id = clientId, RawSocket = raw, Ssl = ssl, Ready = true };
            TaskManager.Instance.Complete(taskId, Value.CreateInteger(clientId));
        }
        catch (Exception ex)
        {
            TaskManager.Instance.CompleteWithFault(taskId, ex);
        }
    }

    // ── connect (client side) ────────────────────────────────────────────────

    public void TcpConnect(string host, int port, long taskId, string? sniHost = null)
    {
        _ = DoConnectAsync(host, port, taskId, sniHost);
    }

    private async Task DoConnectAsync(string host, int port, long taskId, string? sniHost)
    {
        try
        {
            var raw = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            await raw.ConnectAsync(host, port).ConfigureAwait(false);

            var netStream = new NetworkStream(raw, ownsSocket: true);
            var ssl = new SslStream(netStream, leaveInnerStreamOpen: false);

            string target = sniHost ?? host;
            await ssl.AuthenticateAsClientAsync(new SslClientAuthenticationOptions
            {
                TargetHost = target,
                RemoteCertificateValidationCallback = ValidateServerCertificate,
                EnabledSslProtocols = SslProtocols.Tls12
            }).ConfigureAwait(false);

            long clientId = Interlocked.Increment(ref _nextId);
            _conns[clientId] = new TlsConn { Id = clientId, RawSocket = raw, Ssl = ssl, Ready = true };
            TaskManager.Instance.Complete(taskId, Value.CreateInteger(clientId));
        }
        catch (Exception ex)
        {
            TaskManager.Instance.CompleteWithFault(taskId, ex);
        }
    }

    // ── send ─────────────────────────────────────────────────────────────────

    public long EnqueueSend(long socketId, byte[] data)
    {
        if (!_conns.TryGetValue(socketId, out var conn) || !conn.Ready)
            throw new InvalidOperationException($"TLS socket {socketId} is not connected/handshaked");

        long taskId = TaskManager.Instance.AllocateAndRegisterAsyncTask();
        _ = DoSendAsync(taskId, conn.Ssl!, data);
        return taskId;
    }

    private static async Task DoSendAsync(long taskId, SslStream ssl, byte[] data)
    {
        try
        {
            await ssl.WriteAsync(data).ConfigureAwait(false);
            await ssl.FlushAsync().ConfigureAwait(false);
            TaskManager.Instance.Complete(taskId, Value.Default);
        }
        catch (Exception ex)
        {
            TaskManager.Instance.CompleteWithFault(taskId, ex);
        }
    }

    // ── recv ─────────────────────────────────────────────────────────────────

    public long EnqueueRecv(long socketId, int maxBytes = 4096)
    {
        maxBytes = Math.Max(maxBytes, 4096);

        if (!_conns.TryGetValue(socketId, out var conn) || !conn.Ready)
            throw new InvalidOperationException($"TLS socket {socketId} is not connected/handshaked");

        long taskId = TaskManager.Instance.AllocateAndRegisterAsyncTask();
        _ = DoRecvAsync(taskId, conn.Ssl!, maxBytes);
        return taskId;
    }

    private static async Task DoRecvAsync(long taskId, SslStream ssl, int maxBytes)
    {
        try
        {
            var buf = new byte[maxBytes];
            int n = await ssl.ReadAsync(buf.AsMemory(0, maxBytes)).ConfigureAwait(false);
            byte[] data = n == 0 ? [] : buf[..n];
            TaskManager.Instance.Complete(taskId, Value.CreateBytes(data));
        }
        catch (Exception ex)
        {
            TaskManager.Instance.CompleteWithFault(taskId, ex);
        }
    }

    // ── close ─────────────────────────────────────────────────────────────────

    public void Close(long id)
    {
        if (_conns.TryRemove(id, out var conn))
            conn.Dispose();
    }

    // ── certificate validation ────────────────────────────────────────────────

    private static bool ValidateServerCertificate(
        object sender,
        X509Certificate? certificate,
        X509Chain? chain,
        SslPolicyErrors sslPolicyErrors)
    {
        // Accept self-signed certs (chain errors only); reject name-mismatch.
        return sslPolicyErrors == SslPolicyErrors.None
            || (sslPolicyErrors & ~SslPolicyErrors.RemoteCertificateChainErrors) == SslPolicyErrors.None;
    }

    // ── self-signed certificate ───────────────────────────────────────────────

    private static X509Certificate2 GenerateSelfSignedCertificate()
    {
        using var rsa = RSA.Create(2048);
        var req = new CertificateRequest(
            "cn=kiwi.local", rsa, HashAlgorithmName.SHA256, RSASignaturePadding.Pkcs1);

        var san = new SubjectAlternativeNameBuilder();
        san.AddDnsName("kiwi.local");
        san.AddIpAddress(IPAddress.Loopback);
        req.CertificateExtensions.Add(san.Build());

        using var ephemeral = req.CreateSelfSigned(DateTimeOffset.Now, DateTimeOffset.Now.AddYears(1));

        // On Windows, Schannel (the platform TLS provider) requires the server certificate's
        // private key to be accessible via a CNG key storage provider. An in-memory key from
        // CreateSelfSigned is not registered with CNG, so AuthenticateAsServerAsync hangs
        // indefinitely while Schannel tries to locate the key. Exporting to PFX and reimporting
        // with EphemeralKeySet registers the key with CNG without persisting anything to disk,
        // and works correctly on both Windows (Schannel) and Linux (OpenSSL).
        var pfx = ephemeral.Export(X509ContentType.Pfx);
        return new X509Certificate2(pfx, (string?)null,
            X509KeyStorageFlags.Exportable | X509KeyStorageFlags.EphemeralKeySet);
    }
}

// ── connection state ──────────────────────────────────────────────────────────

internal sealed class TlsConn : IDisposable
{
    public long Id { get; init; }

    // Set only for listener sockets.
    public Socket? ListenerSocket { get; init; }

    // Set for connected/accepted client sockets.
    public Socket? RawSocket { get; init; }
    public SslStream? Ssl { get; init; }
    public bool Ready { get; set; }

    public void Dispose()
    {
        try { Ssl?.Close(); } catch { }
        try { ListenerSocket?.Close(); } catch { }
        try { RawSocket?.Close(); } catch { }
    }
}
