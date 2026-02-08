using System;
using System.Collections.Generic;
using System.IO;
using System.Net.Http;
using System.Net.Http.Headers;
using System.Text;
using System.Threading.Tasks;
using kiwi.Parsing;
using kiwi.Parsing.Keyword;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Handler;

public static class HttpBuiltinHandler
{
    private static readonly HttpClient Client = new()
    {
        Timeout = TimeSpan.FromSeconds(30)
    };

    public static Value Execute(Token token, TokenName builtin, List<Value> args)
    {
        return builtin switch
        {
            TokenName.Builtin_Http_Get      => HttpRequest(token, HttpMethod.Get, args),
            TokenName.Builtin_Http_Post     => HttpRequest(token, HttpMethod.Post, args),
            TokenName.Builtin_Http_Put      => HttpRequest(token, HttpMethod.Put, args),
            TokenName.Builtin_Http_Patch    => HttpRequest(token, HttpMethod.Patch, args),
            TokenName.Builtin_Http_Delete   => HttpRequest(token, HttpMethod.Delete, args),
            TokenName.Builtin_Http_Head     => HttpRequest(token, HttpMethod.Head, args),
            TokenName.Builtin_Http_Options  => HttpRequest(token, HttpMethod.Options, args),
            TokenName.Builtin_Http_Timeout  => SetTimeout(token, args),
            TokenName.Builtin_Http_Download => HttpDownload(token, args),

            _ => throw new FunctionUndefinedError(token, token.Text)
        };
    }

    private static Value SetTimeout(Token token, List<Value> args)
    {
        ParameterCountMismatchError.Check(token, HttpBuiltin.HttpTimeout, 1, args.Count);
        ParameterTypeMismatchError.ExpectInteger(token, HttpBuiltin.HttpTimeout, 0, args[0]);

        var timeoutMilliseconds = new long[] {1L, args[0].GetInteger()}.Max();
        Client.Timeout = TimeSpan.FromMilliseconds(timeoutMilliseconds);

        return Value.CreateInteger(timeoutMilliseconds);
    }

    private static Value HttpRequest(Token token, HttpMethod method, List<Value> args)
    {
        int minArgs = method == HttpMethod.Get || method == HttpMethod.Head || method == HttpMethod.Options ? 1 : 2;
        int maxArgs = 3;

        if (args.Count < minArgs || args.Count > maxArgs)
        {
            throw new ParameterCountMismatchError(token, $"__http_{method.Method.ToLower()}__", args.Count, [minArgs, maxArgs]);
        }

        ParameterTypeMismatchError.ExpectString(token, "url", 0, args[0]);

        string url = args[0].GetString();
        Value? bodyValue = null;
        Dictionary<Value, Value>? headers = null;

        if (args.Count >= 2)
        {
            if (args[1].IsHashmap())
            {
                headers = args[1].GetHashmap();
            }
            else if (method != HttpMethod.Get && method != HttpMethod.Head && method != HttpMethod.Options)
            {
                bodyValue = args[1];
            }
        }

        if (args.Count == 3)
        {
            ParameterTypeMismatchError.ExpectHashmap(token, "headers", 2, args[2]);
            headers = args[2].GetHashmap();
        }

        long taskId = TaskManager.Instance.AllocateAndRegisterAsyncTask();

        _ = Task.Run(async () =>
        {
            try
            {
                using var req = new HttpRequestMessage(method, url);
                AddHeaders(req, headers);

                if (bodyValue != null && !bodyValue.IsNull())
                {
                    if (bodyValue.IsString())
                    {
                        req.Content = new StringContent(bodyValue.GetString(), Encoding.UTF8);
                    }
                    else if (bodyValue.IsBytes())
                    {
                        req.Content = new ByteArrayContent(bodyValue.GetBytes());
                    }
                    else if (bodyValue.IsHashmap())
                    {
                        string json = Serializer.Serialize(bodyValue, wrapStrings: true);
                        req.Content = new StringContent(json, Encoding.UTF8, "application/json");
                    }
                    else
                    {
                        throw new ParameterTypeMismatchError(
                            token, 
                            $"http_{method.Method.ToLower()}", 
                            1, 
                            bodyValue.Type, 
                            [Typing.ValueType.String, Typing.ValueType.Bytes, Typing.ValueType.Hashmap]
                        );
                    }
                }

                using var resp = await Client.SendAsync(req, HttpCompletionOption.ResponseHeadersRead);
                Value responseValue = await BuildResponseValue(resp);
                TaskManager.Instance.Complete(taskId, responseValue);
            }
            catch (Exception ex)
            {
                TaskManager.Instance.CompleteWithFault(taskId, ex);
            }
        });

        return Value.CreateInteger(taskId);
    }

    private static Value HttpDownload(Token token, List<Value> args)
    {
        if (args.Count < 2 || args.Count > 3)
        {
            throw new ParameterCountMismatchError(token, "http_download", args.Count, [2, 3]);
        }

        ParameterTypeMismatchError.ExpectString(token, "url", 0, args[0]);
        ParameterTypeMismatchError.ExpectString(token, "filename", 1, args[1]);

        string url = args[0].GetString();
        string filename = args[1].GetString();
        Dictionary<Value, Value>? headers = args.Count > 2 ? GetHeaders(token, args[2]) : null;

        long taskId = TaskManager.Instance.AllocateAndRegisterAsyncTask();

        _ = Task.Run(async () =>
        {
            try
            {
                using var req = new HttpRequestMessage(HttpMethod.Get, url);
                AddHeaders(req, headers);

                using var resp = await Client.SendAsync(req, HttpCompletionOption.ResponseHeadersRead);
                resp.EnsureSuccessStatusCode();

                try
                {
                    Directory.CreateDirectory(Path.GetDirectoryName(filename) ?? ".");
                }
                catch { /* swallow */ }

                await using var fs = new FileStream(filename, FileMode.Create, FileAccess.Write, FileShare.None);
                await resp.Content.CopyToAsync(fs);

                var result = new Dictionary<Value, Value>
                {
                    [Value.CreateString("ok")]       = Value.True,
                    [Value.CreateString("filename")] = Value.CreateString(filename),
                    [Value.CreateString("status")]   = Value.CreateInteger((long)resp.StatusCode),
                    [Value.CreateString("size")]     = Value.CreateInteger(new FileInfo(filename).Length)
                };

                TaskManager.Instance.Complete(taskId, Value.CreateHashmap(result));
            }
            catch (Exception ex)
            {
                TaskManager.Instance.CompleteWithFault(taskId, ex);
            }
        });

        return Value.CreateInteger(taskId);
    }

    private static Dictionary<Value, Value>? GetHeaders(Token token, Value arg)
    {
        if (arg.IsNull())
        {
            return null;
        }
        ParameterTypeMismatchError.ExpectHashmap(token, "headers", -1, arg);
        return arg.GetHashmap();
    }

    private static void AddHeaders(HttpRequestMessage req, Dictionary<Value, Value>? headers)
    {
        if (headers == null)
        {
            return;
        }

        foreach (var kv in headers)
        {
            if (!kv.Key.IsString() || !kv.Value.IsString())
            {
                continue; // skip invalid entries
            }

            string key = kv.Key.GetString();
            string val = kv.Value.GetString();

            // Content-* headers must go on Content, not request headers
            if (key.StartsWith("Content-", StringComparison.OrdinalIgnoreCase))
            {
                continue;
            }

            req.Headers.TryAddWithoutValidation(key, val);
        }
    }

    private static async Task<Value> BuildResponseValue(HttpResponseMessage resp)
    {
        var respMap = new Dictionary<Value, Value>();
        var headersMap = new Dictionary<Value, Value>();

        /*
        {
            "status": integer,
            "ok": boolean,
            "url": string,
            "headers": hashmap,
            "body": string|bytes
        }
        */

        respMap[Value.CreateString("status")] = Value.CreateInteger((long)resp.StatusCode);
        respMap[Value.CreateString("ok")]     = Value.CreateBoolean(resp.IsSuccessStatusCode);
        respMap[Value.CreateString("url")]    = Value.CreateString(resp.RequestMessage?.RequestUri?.ToString() ?? "");

        foreach (var header in resp.Headers)
        {
            headersMap[Value.CreateString(header.Key)] = Value.CreateString(string.Join(", ", header.Value));
        }
        foreach (var header in resp.Content.Headers)
        {
            headersMap[Value.CreateString(header.Key)] = Value.CreateString(string.Join(", ", header.Value));
        }
        respMap[Value.CreateString("headers")] = Value.CreateHashmap(headersMap);

        string? contentType = resp.Content.Headers.ContentType?.MediaType;
        bool isTextLike = contentType?.StartsWith("text/") == true ||
                          contentType == "application/json" ||
                          contentType == "application/xml" ||
                          contentType == "application/javascript";

        if (isTextLike)
        {
            using Stream stream = await resp.Content.ReadAsStreamAsync();
            using StreamReader reader = new StreamReader(stream, Encoding.UTF8, detectEncodingFromByteOrderMarks: true, bufferSize: 8192);
            string body = await reader.ReadToEndAsync();
            respMap[Value.CreateString("body")] = Value.CreateString(body);
        }
        else
        {
            await using var responseStream = await resp.Content.ReadAsStreamAsync();
            string tempFile = Path.GetTempFileName();
            await using (var fileStream = File.Create(tempFile))
            {
                await responseStream.CopyToAsync(fileStream);
            }

            byte[] bodyBytes = await File.ReadAllBytesAsync(tempFile);
            respMap[Value.CreateString("body")] = Value.CreateBytes(bodyBytes);
        }

        return Value.CreateHashmap(respMap);
    }
}