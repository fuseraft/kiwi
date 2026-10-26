using System.Collections.Concurrent;
using System.Text;
using Microsoft.AspNetCore.WebUtilities;
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
    // Requests handed to Kiwi by accept() and not yet answered by respond()
    private static readonly ConcurrentDictionary<long, KestrelBackend.Pending> _pending = new();

    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        if (Kiwi.Settings.SafeMode) throw new SafeModeError(token, "httpserver");
        return builtin switch
        {
            TokenName.Builtin_HttpServer_Create  => Create(token, args),
            TokenName.Builtin_HttpServer_Accept  => Accept(token, args),
            TokenName.Builtin_HttpServer_Respond => Respond(token, args),
            TokenName.Builtin_HttpServer_Stop    => Stop(token, args),
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

        var queryParams = new Dictionary<Value, Value>();
        foreach (var kv in QueryHelpers.ParseQuery(pending.QueryString))
            queryParams[Value.CreateString(kv.Key)] = Value.CreateString(kv.Value.ToString());

        var headers = new Dictionary<Value, Value>();
        foreach (var kv in pending.Headers)
            headers[Value.CreateString(kv.Key)] = Value.CreateString(kv.Value);

        return Value.CreateHashmap(new Dictionary<Value, Value>
        {
            [Value.CreateString("id")]          = Value.CreateInteger(reqId),
            [Value.CreateString("method")]      = Value.CreateString(pending.Method),
            [Value.CreateString("path")]        = Value.CreateString(pending.Path),
            [Value.CreateString("query")]       = Value.CreateHashmap(queryParams),
            [Value.CreateString("headers")]     = Value.CreateHashmap(headers),
            [Value.CreateString("body")]        = Value.CreateString(Encoding.UTF8.GetString(pending.Body)),
            [Value.CreateString("body_bytes")]  = Value.CreateBytes(pending.Body),
            [Value.CreateString("remote_addr")] = Value.CreateString(pending.RemoteAddr),
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
        var bodyBytes = args[4].IsBytes()
            ? args[4].GetBytes()
            : Encoding.UTF8.GetBytes(args[4].GetString());

        // Build the reply before claiming the request, so nothing can fail between the two
        // and leave the client waiting for the request timeout.
        var headers = new List<KeyValuePair<string, string>>();
        foreach (var kv in hdrs)
        {
            var key = HeaderText(kv.Key);
            // "__set_cookies__" holds a list of Set-Cookie strings
            if (key == "__set_cookies__")
            {
                if (kv.Value.IsList())
                {
                    foreach (var cookie in kv.Value.GetList())
                        headers.Add(new("Set-Cookie", HeaderText(cookie)));
                }
            }
            else
            {
                headers.Add(new(key, HeaderText(kv.Value)));
            }
        }

        // A request that timed out, or was turned away by stop(), is already answered.
        if (!_pending.TryRemove(requestId, out var pending))
            return Value.CreateBoolean(false);

        return Value.CreateBoolean(pending.Reply.TrySetResult(new KestrelBackend.Reply(status, headers, bodyBytes)));
    }

    // Header names and values may be any Kiwi value; `res.header("X-Count", 5)` sends "5".
    private static string HeaderText(Value v) => v.IsString() ? v.GetString() : Serializer.Serialize(v);

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
