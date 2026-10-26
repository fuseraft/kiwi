using System.Net;
using Microsoft.AspNetCore.Hosting.Server;
using Microsoft.AspNetCore.Http.Features;
using Microsoft.AspNetCore.Server.Kestrel.Core;
using Microsoft.AspNetCore.Server.Kestrel.Transport.Sockets;
using Microsoft.Extensions.Logging.Abstractions;
using Microsoft.Extensions.Primitives;
using Microsoft.Extensions.Options;
using Queue = System.Threading.Channels.Channel;

namespace kiwi.Runtime.Builtin.Handler;

/// <summary>
/// The HTTP server behind the httpserver package. Kestrel parses requests and enforces
/// connection-level limits (body size, header timeout, keep-alive); this class hands each
/// request to Kiwi through <see cref="Accept"/> and waits for the matching reply.
/// At most <see cref="Limits.MaxInFlight"/> requests wait on Kiwi code at once. A request that
/// cannot get a slot within <see cref="Limits.QueueTimeoutMs"/> gets 503, and one Kiwi does not
/// answer within <see cref="Limits.RequestTimeoutMs"/> gets 504.
/// </summary>
public sealed class KestrelBackend
{
    public sealed record Limits(
        int MaxInFlight = 64,
        long MaxBodySize = 8 * 1024 * 1024,
        int RequestTimeoutMs = 30_000,
        int QueueTimeoutMs = 5_000);

    public sealed record Reply(int Status, List<KeyValuePair<string, string>> Headers, byte[] Body);

    private static readonly Reply Unavailable = new(503, [new("Retry-After", "1")], []);

    public sealed class Pending
    {
        public required string Method { get; init; }
        public required string Path { get; init; }
        public required string QueryString { get; init; }
        public required Dictionary<string, string> Headers { get; init; }
        public required byte[] Body { get; init; }
        public required string RemoteAddr { get; init; }
        public TaskCompletionSource<Reply> Reply { get; } = new(TaskCreationOptions.RunContinuationsAsynchronously);
    }

    private readonly KestrelServer _server;
    private readonly System.Threading.Channels.Channel<Pending> _queue = Queue.CreateUnbounded<Pending>();
    private readonly HashSet<Pending> _waiting = [];
    private readonly Lock _waitingLock = new();
    private readonly SemaphoreSlim _inFlight;
    private readonly TimeSpan _queueTimeout;
    private readonly TimeSpan _requestTimeout;
    private volatile bool _listening;

    public KestrelBackend(string host, int port, Limits limits)
    {
        var options = new KestrelServerOptions { AddServerHeader = false };
        options.Limits.MaxRequestBodySize    = limits.MaxBodySize;
        options.Limits.RequestHeadersTimeout = TimeSpan.FromSeconds(10);
        options.Limits.KeepAliveTimeout      = TimeSpan.FromSeconds(60);

        if (host is "localhost")
            options.ListenLocalhost(port);
        else if (IPAddress.TryParse(host, out var address))
            options.Listen(address, port);
        else
            options.ListenAnyIP(port); // "*", "+" or a host name: bind every interface

        _inFlight       = new SemaphoreSlim(limits.MaxInFlight);
        _queueTimeout   = TimeSpan.FromMilliseconds(limits.QueueTimeoutMs);
        _requestTimeout = TimeSpan.FromMilliseconds(limits.RequestTimeoutMs);

        var transport = new SocketTransportFactory(Options.Create(new SocketTransportOptions()), NullLoggerFactory.Instance);
        _server = new KestrelServer(Options.Create(options), transport, NullLoggerFactory.Instance);
        _server.StartAsync(new Application(this), CancellationToken.None).GetAwaiter().GetResult();
        _listening = true;
    }

    public bool IsListening => _listening;

    /// <summary>Waits up to <paramref name="timeoutMs"/> for the next request, or returns null.</summary>
    public Pending? Accept(int timeoutMs)
    {
        if (_queue.Reader.TryRead(out var next)) return next;
        try
        {
            if (!_queue.Reader.WaitToReadAsync().AsTask().Wait(timeoutMs)) return null;
        }
        catch (AggregateException) { return null; }
        return _queue.Reader.TryRead(out next) ? next : null;
    }

    /// <summary>
    /// Stops accepting, answers every request Kiwi has not replied to with 503, and closes
    /// the listener once in-flight responses finish (or after <paramref name="graceMs"/>).
    /// </summary>
    public void Stop(int graceMs = 5_000)
    {
        _listening = false;
        _queue.Writer.TryComplete();

        lock (_waitingLock)
        {
            foreach (var pending in _waiting)
                pending.Reply.TrySetResult(Unavailable);
        }

        using var grace = new CancellationTokenSource(graceMs);
        try { _server.StopAsync(grace.Token).GetAwaiter().GetResult(); }
        catch (OperationCanceledException) { }
        _server.Dispose();
    }

    private async Task Process(IFeatureCollection features)
    {
        var req  = features.Get<IHttpRequestFeature>()!;
        var res  = features.Get<IHttpResponseFeature>()!;
        var body = features.Get<IHttpResponseBodyFeature>()!;
        var conn = features.Get<IHttpConnectionFeature>();

        if (!_listening || !await _inFlight.WaitAsync(_queueTimeout))
        {
            await Send(res, body, Unavailable);
            return;
        }

        try
        {
            byte[] payload;
            try
            {
                using var ms = new MemoryStream();
                await req.Body.CopyToAsync(ms);
                payload = ms.ToArray();
            }
            catch (Microsoft.AspNetCore.Http.BadHttpRequestException ex)
            {
                res.StatusCode = ex.StatusCode; // 413 past MaxBodySize, 408 for a stalled body
                return;
            }

            var headers = new Dictionary<string, string>();
            foreach (var h in req.Headers)
                headers[h.Key.ToLowerInvariant()] = h.Value.ToString();

            var pending = new Pending
            {
                Method      = req.Method,
                Path        = string.IsNullOrEmpty(req.Path) ? "/" : req.Path,
                QueryString = req.QueryString,
                Headers     = headers,
                Body        = payload,
                RemoteAddr  = conn?.RemoteIpAddress?.ToString() ?? "",
            };

            lock (_waitingLock) _waiting.Add(pending);
            try
            {
                // Stop() completes the queue; a request arriving after that is turned away.
                if (!_queue.Writer.TryWrite(pending))
                    pending.Reply.TrySetResult(Unavailable);

                Reply reply;
                try
                {
                    reply = await pending.Reply.Task.WaitAsync(_requestTimeout);
                }
                catch (TimeoutException)
                {
                    // Fail the completion so a late respond() from Kiwi is a no-op.
                    pending.Reply.TrySetCanceled();
                    reply = new Reply(504, [], []);
                }

                await Send(res, body, reply);
            }
            finally
            {
                lock (_waitingLock) _waiting.Remove(pending);
            }
        }
        finally
        {
            _inFlight.Release();
        }
    }

    private static async Task Send(IHttpResponseFeature res, IHttpResponseBodyFeature body, Reply reply)
    {
        res.StatusCode = reply.Status;
        foreach (var (key, value) in reply.Headers)
            res.Headers[key] = StringValues.Concat(res.Headers[key], value);
        res.Headers.ContentLength = reply.Body.Length;
        if (reply.Body.Length > 0)
            await body.Writer.WriteAsync(reply.Body);
    }

    private sealed class Application(KestrelBackend owner) : IHttpApplication<IFeatureCollection>
    {
        public IFeatureCollection CreateContext(IFeatureCollection contextFeatures) => contextFeatures;

        public Task ProcessRequestAsync(IFeatureCollection context) => owner.Process(context);

        public void DisposeContext(IFeatureCollection context, Exception? exception) { }
    }
}
