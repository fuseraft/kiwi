using System.Collections.Concurrent;
using System.Text;
using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Settings;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class HttpServerBuiltinHandler
{
    private static long _nextServerId;
    private static long _nextRequestId;

    private static readonly ConcurrentDictionary<long, KestrelBackend>         _servers = new();
    // Requests handed to Kiwi by accept() and not yet answered by respond() or stream()
    private static readonly ConcurrentDictionary<long, KestrelBackend.Pending> _pending = new();
    // Streaming responses started by stream() and not yet closed by end()
    private static readonly ConcurrentDictionary<long, KestrelBackend.Pending> _streams = new();

    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        if (Kiwi.Settings.SafeMode) throw new SafeModeError(token, "httpserver");
        return builtin switch
        {
            TokenName.Builtin_HttpServer_Create  => Create(token, args),
            TokenName.Builtin_HttpServer_Accept  => Accept(token, args),
            TokenName.Builtin_HttpServer_Respond => Respond(token, args),
            TokenName.Builtin_HttpServer_Stop    => Stop(token, args),
            TokenName.Builtin_HttpServer_Stream  => Stream(token, args),
            TokenName.Builtin_HttpServer_Write   => Write(token, args),
            TokenName.Builtin_HttpServer_End     => End(token, args),
            _ => throw new FunctionUndefinedError(token, token.Text),
        };
    }

    private static Value Create(Token token, List<Value> args)
    {
        if (args.Count < 1 || args.Count > 3)
            throw new ParameterCountMismatchError(token, HttpServerBuiltin.Create, args.Count, [1, 2, 3]);

        string host = "localhost";
        int port;
        var limits = new KestrelBackend.Limits();

        if (args.Count >= 2)
        {
            ParameterTypeMismatchError.ExpectString(token, HttpServerBuiltin.Create, 0, args[0]);
            ParameterTypeMismatchError.ExpectInteger(token, HttpServerBuiltin.Create, 1, args[1]);
            host = args[0].GetString();
            port = (int)args[1].GetInteger();
        }
        else
        {
            ParameterTypeMismatchError.ExpectInteger(token, HttpServerBuiltin.Create, 0, args[0]);
            port = (int)args[0].GetInteger();
        }

        if (args.Count == 3)
        {
            ParameterTypeMismatchError.ExpectHashmap(token, HttpServerBuiltin.Create, 2, args[2]);
            var opts = args[2].GetHashmap();
            limits = new KestrelBackend.Limits(
                MaxInFlight:      (int)Limit(token, opts, "max_in_flight", limits.MaxInFlight),
                MaxBodySize:      Limit(token, opts, "max_body_size", limits.MaxBodySize),
                RequestTimeoutMs: (int)Limit(token, opts, "request_timeout_ms", limits.RequestTimeoutMs));
        }

        try
        {
            var server = new KestrelBackend(host, port, limits);
            var id = Interlocked.Increment(ref _nextServerId);
            _servers[id] = server;
            return Value.CreateInteger(id);
        }
        catch (Exception ex)
        {
            throw new SystemError(token, $"Failed to start HTTP server on {host}:{port}: {ex.Message}");
        }
    }

    private static long Limit(Token token, SyncDictionary<Value, Value> opts, string name, long fallback)
    {
        if (!opts.TryGetValue(Value.CreateString(name), out var v) || v.IsNull())
            return fallback;
        if (!v.IsInteger() || v.GetInteger() <= 0 || (name != "max_body_size" && v.GetInteger() > int.MaxValue))
            throw new InvalidOperationError(token, $"httpserver option `{name}` must be a positive integer.");
        return v.GetInteger();
    }

    private static Value Accept(Token token, List<Value> args)
    {
        if (args.Count < 1 || args.Count > 2)
            throw new ParameterCountMismatchError(token, HttpServerBuiltin.Accept, args.Count, [1, 2]);

        ParameterTypeMismatchError.ExpectInteger(token, HttpServerBuiltin.Accept, 0, args[0]);
        var serverId  = args[0].GetInteger();
        int timeoutMs = args.Count == 2 && args[1].IsInteger() ? (int)args[1].GetInteger() : 5000;

        if (!_servers.TryGetValue(serverId, out var server))
            throw new InvalidOperationError(token, $"No HTTP server with id {serverId}.");

        if (!server.IsListening) return Value.CreateNull();
        var pending = server.Accept(timeoutMs);
        if (pending is null) return Value.CreateNull();

        var reqId = Interlocked.Increment(ref _nextRequestId);
        _pending[reqId] = pending;

        var headers = new Dictionary<Value, Value>();
        foreach (var kv in pending.Headers)
            headers[Value.CreateString(kv.Key)] = Value.CreateString(kv.Value);

        return Value.CreateHashmap(new Dictionary<Value, Value>
        {
            [Value.CreateString("id")]          = Value.CreateInteger(reqId),
            [Value.CreateString("method")]      = Value.CreateString(pending.Method),
            [Value.CreateString("path")]         = Value.CreateString(pending.Path),
            [Value.CreateString("query_string")] = Value.CreateString(pending.QueryString.TrimStart('?')),
            [Value.CreateString("headers")]      = Value.CreateHashmap(headers),
            [Value.CreateString("body")]         = Value.CreateBytes(pending.Body),
            [Value.CreateString("remote_addr")]  = Value.CreateString(pending.RemoteAddr),
            [Value.CreateString("protocol")]     = Value.CreateString(pending.Protocol),
            [Value.CreateString("scheme")]       = Value.CreateString(pending.Scheme),
        });
    }

    private static Value Respond(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, HttpServerBuiltin.Respond, 5, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, HttpServerBuiltin.Respond, 0, args[0]);
        ParameterTypeMismatchError.ExpectInteger(token, HttpServerBuiltin.Respond, 1, args[1]);
        ParameterTypeMismatchError.ExpectInteger(token, HttpServerBuiltin.Respond, 2, args[2]);
        ParameterTypeMismatchError.ExpectHashmap(token, HttpServerBuiltin.Respond, 3, args[3]);
        if (!args[4].IsString() && !args[4].IsBytes())
            throw new ParameterTypeMismatchError(token, HttpServerBuiltin.Respond, 4,
                args[4].Type, new List<kiwi.Typing.ValueType> { kiwi.Typing.ValueType.String, kiwi.Typing.ValueType.Bytes });

        var requestId = args[1].GetInteger();
        var status    = (int)args[2].GetInteger();
        var hdrs      = args[3].GetHashmap();

        // Body can be a string (UTF-8 encoded) or raw bytes (for binary files)
        var bodyBytes = BodyBytes(args[4]);

        // Build the reply before claiming the request, so nothing can fail between the two
        // and leave the client waiting for the request timeout.
        var headers = BuildHeaders(hdrs);

        // A request that timed out, or was turned away by stop(), is already answered.
        if (!_pending.TryRemove(requestId, out var pending))
            return Value.CreateBoolean(false);

        return Value.CreateBoolean(pending.Reply.TrySetResult(new KestrelBackend.Reply(status, headers, bodyBytes)));
    }

    /// <summary>
    /// Response headers from a Kiwi hashmap. A list value sends one header per element
    /// (`"set-cookie": ["a=1", "b=2"]`); any other value is sent as its text, so 5 sends "5".
    /// </summary>
    private static List<KeyValuePair<string, string>> BuildHeaders(SyncDictionary<Value, Value> hdrs)
    {
        var headers = new List<KeyValuePair<string, string>>();
        foreach (var kv in hdrs)
        {
            var key = HeaderText(kv.Key);
            if (kv.Value.IsList())
            {
                foreach (var item in kv.Value.GetList())
                    headers.Add(new(key, HeaderText(item)));
            }
            else
            {
                headers.Add(new(key, HeaderText(kv.Value)));
            }
        }
        return headers;
    }

    private static string HeaderText(Value v) => v.IsString() ? v.GetString() : Serializer.Serialize(v);

    private static byte[] BodyBytes(Value v) => v.IsBytes() ? v.GetBytes() : Encoding.UTF8.GetBytes(v.GetString());

    /// <summary>
    /// Starts a streaming response: `__httpserver_stream__(server, request, status, headers)`.
    /// The headers go out at once; the body follows through write() until end().
    /// Returns false when the request was already answered (it timed out or the server stopped).
    /// </summary>
    private static Value Stream(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, HttpServerBuiltin.Stream, 4, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, HttpServerBuiltin.Stream, 0, args[0]);
        ParameterTypeMismatchError.ExpectInteger(token, HttpServerBuiltin.Stream, 1, args[1]);
        ParameterTypeMismatchError.ExpectInteger(token, HttpServerBuiltin.Stream, 2, args[2]);
        ParameterTypeMismatchError.ExpectHashmap(token, HttpServerBuiltin.Stream, 3, args[3]);

        var requestId = args[1].GetInteger();
        var headers   = BuildHeaders(args[3].GetHashmap());

        if (!_pending.TryRemove(requestId, out var pending))
            return Value.CreateBoolean(false);

        _streams[requestId] = pending;
        var reply = new KestrelBackend.Reply((int)args[2].GetInteger(), headers, [], Streaming: true);
        if (pending.Reply.TrySetResult(reply))
            return Value.CreateBoolean(true);

        _streams.TryRemove(requestId, out _);
        return Value.CreateBoolean(false);
    }

    /// <summary>
    /// Writes one chunk of a streaming response: `__httpserver_write__(request, chunk)`.
    /// Waits while the client is behind. Returns false once the client has gone away or the
    /// stream has ended, which tells the producer to stop.
    /// </summary>
    private static Value Write(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, HttpServerBuiltin.Write, 2, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, HttpServerBuiltin.Write, 0, args[0]);
        if (!args[1].IsString() && !args[1].IsBytes())
            throw new ParameterTypeMismatchError(token, HttpServerBuiltin.Write, 1,
                args[1].Type, new List<kiwi.Typing.ValueType> { kiwi.Typing.ValueType.String, kiwi.Typing.ValueType.Bytes });

        if (!_streams.TryGetValue(args[0].GetInteger(), out var pending))
            return Value.CreateBoolean(false);

        var chunk = BodyBytes(args[1]);
        if (chunk.Length == 0)
            return Value.CreateBoolean(true);

        try
        {
            pending.Chunks.Writer.WriteAsync(chunk).AsTask().GetAwaiter().GetResult();
            return Value.CreateBoolean(true);
        }
        catch (System.Threading.Channels.ChannelClosedException) { return Value.CreateBoolean(false); }
        catch (OperationCanceledException) { return Value.CreateBoolean(false); }
    }

    /// <summary>
    /// Ends a streaming response: `__httpserver_end__(request, aborted = false)`. With
    /// aborted, the connection is reset so the client sees a truncated response, not a
    /// complete one.
    /// </summary>
    private static Value End(Token token, List<Value> args)
    {
        if (args.Count < 1 || args.Count > 2)
            throw new ParameterCountMismatchError(token, HttpServerBuiltin.End, args.Count, [1, 2]);
        ParameterTypeMismatchError.ExpectInteger(token, HttpServerBuiltin.End, 0, args[0]);

        if (!_streams.TryRemove(args[0].GetInteger(), out var pending))
            return Value.CreateBoolean(false);

        if (args.Count == 2 && args[1].IsBoolean() && args[1].GetBoolean())
            pending.AbortRequested = true;
        pending.Chunks.Writer.TryComplete();
        return Value.CreateBoolean(true);
    }

    private static Value Stop(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, HttpServerBuiltin.Stop, 1, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, HttpServerBuiltin.Stop, 0, args[0]);

        if (_servers.TryRemove(args[0].GetInteger(), out var server))
        {
            try { server.Stop(); } catch { }
        }

        return Value.Default;
    }
}
