---
title: http
description: An HTTP client, plus helpers for parsing requests and building responses.
---

The `http` package is an HTTP client. Requests run asynchronously on background tasks; the `get` and `await_*` functions wait for the result and wrap it in an `HttpResponse`. The package also parses raw HTTP requests and builds raw responses for servers written on top of [`socket`](/kiwi/stdlib/socket/). For a full web server, see [`httpserver`](/kiwi/stdlib/httpserver/).

## Making requests

```kiwi nocheck
resp = http::get("https://httpbin.org/get")
println resp.status   # 200
println resp.ok       # true
println resp.text()

data = http::await_json_get("https://httpbin.org/json")   # parsed JSON body
println data.pretty()

created = http::await_post("https://httpbin.org/post", {name: "Kiwi"})
println created.json()["json"] when created.ok
```

Request bodies that are hashmaps or lists are sent as JSON. Pass headers as a hashmap:

```kiwi nocheck
resp = http::get("https://api.github.com/repos/fuseraft/kiwi", {"User-Agent": "kiwi"})
println resp.json()["stargazers_count"]
```

`http::set_timeout(ms)` sets the timeout for later requests, and `await_download(url, path)` saves a response to a file.

### Responses

An `HttpResponse` has these fields and methods:

| Member | Description |
| :-- | :-- |
| `status` | The status code. |
| `ok` | `true` for a 2xx status. |
| `headers` | The response headers. |
| `body` | The response body. |
| `url` | The final URL after redirects. |
| `text()` | The body as a string. |
| `json()` | The body parsed as JSON. |
| `pretty()` | The JSON body, indented. |
| `save(path)` | Writes the body to a file. |

### Concurrent requests

`__get__`, `post`, `put` and `delete` start a request and return a task id immediately. Start several, then await them with [`task::await`](/kiwi/stdlib/task/) and wrap each result with `http::response`:

```kiwi nocheck
first = http::__get__("https://httpbin.org/get")
second = http::__get__("https://httpbin.org/ip")

r1 = http::response(task::await(first))
r2 = http::response(task::await(second))
println [r1.status, r2.status]
```

## Parsing requests and building responses

These helpers work on raw bytes, for servers that handle sockets themselves:

```kiwi
raw = "GET /api/users?page=2 HTTP/1.1\r\nHost: example.com\r\n\r\n".to_bytes()
req = http::parse(raw)

println req["method"]          # GET
println req["path"]            # /api/users
println req["query"]["page"]   # 2
println req["headers"]["host"] # example.com

reply = http::json_response(200, {ok: true})
println reply.to_string().lines()[0]   # HTTP/1.1 200 OK
```

`html_response(status, html)` and `response_bytes(status, body, headers)` build other responses.

<!-- api: http -->
