using System.Collections.Concurrent;
using System.Net;
using System.Text;
using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class HttpServerBuiltinHandler
{
    private static long _nextServerId;
    private static long _nextRequestId;

    private static readonly ConcurrentDictionary<long, HttpListener>                    _servers      = new();
    private static readonly ConcurrentDictionary<long, HttpListenerContext>             _contexts     = new();
    // One reusable pending-accept task per server to avoid double-calling GetContextAsync
    private static readonly ConcurrentDictionary<long, Task<HttpListenerContext>>       _pendingAccept = new();

    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
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
        if (args.Count < 1 || args.Count > 2)
            throw new ParameterCountMismatchError(token, HttpServerBuiltin.Create, args.Count, [1, 2]);

        string host = "localhost";
        int port;

        if (args.Count == 2)
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

        try
        {
            var listener = new HttpListener();
            listener.Prefixes.Add($"http://{host}:{port}/");
            listener.Start();

            var id = Interlocked.Increment(ref _nextServerId);
            _servers[id] = listener;
            return Value.CreateInteger(id);
        }
        catch (Exception ex)
        {
            throw new SystemError(token, $"Failed to start HTTP server on {host}:{port}: {ex.Message}");
        }
    }

    private static Value Accept(Token token, List<Value> args)
    {
        if (args.Count < 1 || args.Count > 2)
            throw new ParameterCountMismatchError(token, HttpServerBuiltin.Accept, args.Count, [1, 2]);

        ParameterTypeMismatchError.ExpectInteger(token, HttpServerBuiltin.Accept, 0, args[0]);
        var serverId  = args[0].GetInteger();
        int timeoutMs = args.Count == 2 && args[1].IsInteger() ? (int)args[1].GetInteger() : 5000;

        if (!_servers.TryGetValue(serverId, out var listener))
            throw new InvalidOperationError(token, $"No HTTP server with id {serverId}.");

        try
        {
            if (!listener.IsListening)
                return Value.CreateNull();

            // Reuse a pending GetContextAsync task — never call it twice before it completes
            var ctxTask = _pendingAccept.GetOrAdd(serverId, _ => listener.GetContextAsync());
            if (!ctxTask.Wait(timeoutMs))
                return Value.CreateNull();

            // Request completed — remove the pending task so the next call starts fresh
            _pendingAccept.TryRemove(serverId, out _);
            var ctx = ctxTask.Result;
            var reqId = Interlocked.Increment(ref _nextRequestId);
            _contexts[reqId] = ctx;

            var req  = ctx.Request;
            var body = string.Empty;

            if (req.HasEntityBody)
            {
                using var reader = new System.IO.StreamReader(
                    req.InputStream, req.ContentEncoding ?? Encoding.UTF8);
                body = reader.ReadToEnd();
            }

            var queryParams = new Dictionary<Value, Value>();
            foreach (string? key in req.QueryString.AllKeys)
            {
                if (key is null) continue;
                queryParams[Value.CreateString(key)] = Value.CreateString(req.QueryString[key] ?? "");
            }

            var headers = new Dictionary<Value, Value>();
            foreach (string? key in req.Headers.AllKeys)
            {
                if (key is null) continue;
                headers[Value.CreateString(key.ToLowerInvariant())] = Value.CreateString(req.Headers[key] ?? "");
            }

            var result = new Dictionary<Value, Value>
            {
                [Value.CreateString("id")]          = Value.CreateInteger(reqId),
                [Value.CreateString("method")]      = Value.CreateString(req.HttpMethod),
                [Value.CreateString("path")]        = Value.CreateString(req.Url?.AbsolutePath ?? "/"),
                [Value.CreateString("query")]       = Value.CreateHashmap(queryParams),
                [Value.CreateString("headers")]     = Value.CreateHashmap(headers),
                [Value.CreateString("body")]        = Value.CreateString(body),
                [Value.CreateString("remote_addr")] = Value.CreateString(
                    req.RemoteEndPoint?.Address.ToString() ?? ""),
            };

            return Value.CreateHashmap(result);
        }
        catch (ObjectDisposedException)  { return Value.CreateNull(); }
        catch (HttpListenerException)    { return Value.CreateNull(); }
        catch (AggregateException ae) when (ae.InnerException is HttpListenerException)
        {
            return Value.CreateNull();
        }
        catch (Exception ex)
        {
            throw new SystemError(token, $"HTTP accept error: {ex.Message}");
        }
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

        var serverId  = args[0].GetInteger();
        var requestId = args[1].GetInteger();
        var status    = (int)args[2].GetInteger();
        var hdrs      = args[3].GetHashmap();

        // Body can be a string (UTF-8 encoded) or raw bytes (for binary files)
        var bodyBytes = args[4].IsBytes()
            ? args[4].GetBytes()
            : Encoding.UTF8.GetBytes(args[4].GetString());

        if (!_servers.ContainsKey(serverId))
            throw new InvalidOperationError(token, $"No HTTP server with id {serverId}.");

        if (!_contexts.TryRemove(requestId, out var ctx))
            throw new InvalidOperationError(token, $"No pending request with id {requestId}.");

        try
        {
            var response        = ctx.Response;
            response.StatusCode = status;

            var cookiesKey = Value.CreateString("__set_cookies__");
            foreach (var kv in hdrs)
            {
                var key = kv.Key.GetString();
                // "__set_cookies__" holds a JSON array of Set-Cookie strings
                if (key == "__set_cookies__")
                {
                    // Value is a Kiwi list of cookie strings
                    if (kv.Value.IsList())
                    {
                        foreach (var cookie in kv.Value.GetList())
                            response.Headers.Add("Set-Cookie", cookie.GetString());
                    }
                }
                else
                {
                    response.Headers[key] = kv.Value.GetString();
                }
            }

            response.ContentLength64 = bodyBytes.Length;
            response.OutputStream.Write(bodyBytes, 0, bodyBytes.Length);
            response.OutputStream.Close();
            response.Close();

            return Value.CreateBoolean(true);
        }
        catch (Exception ex)
        {
            throw new SystemError(token, $"HTTP respond error: {ex.Message}");
        }
    }

    private static Value Stop(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, HttpServerBuiltin.Stop, 1, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, HttpServerBuiltin.Stop, 0, args[0]);

        var serverId = args[0].GetInteger();

        _pendingAccept.TryRemove(serverId, out _);
        if (_servers.TryRemove(serverId, out var listener))
        {
            try { listener.Stop(); listener.Close(); } catch { }
        }

        return Value.Default;
    }
}
