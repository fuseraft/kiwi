# http

> An HTTP client, plus helpers for parsing requests and building responses.

The `http` package is an HTTP client. Requests run asynchronously on background tasks; the `get` and `await_*` functions wait for the result and wrap it in an `HttpResponse`. The package also parses raw HTTP requests and builds raw responses for servers written on top of [`socket`](socket.md). For a full web server, see [`httpserver`](httpserver.md).

## Making requests

```kiwi
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

```kiwi
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

`__get__`, `post`, `put` and `delete` start a request and return a task id immediately. Start several, then await them with [`task::await`](task.md) and wrap each result with `http::response`:

```kiwi
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

## Functions

### `http::parse_hex`

```kiwi
http::parse_hex(s: string): integer
```

Parses a hexadecimal string to an integer.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `s` | `string` |  | A lowercase hexadecimal string (e.g., `"1f"`). |

**Returns** `integer` — The integer value.

**Throws** `string` — If an invalid hex character is encountered.

### `http::url_decode`

```kiwi
http::url_decode(s: string, decode_plus: boolean = true): string
```

Decodes a percent-encoded URL string.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `s` | `string` |  | The URL-encoded string to decode. |
| `decode_plus` | `boolean` | `true` | If `true`, decodes `+` as a space (default `true`). |

**Returns** `string` — The decoded string.

### `http::read_crlf_line`

```kiwi
http::read_crlf_line(iter): string
```

Reads a CRLF-terminated line from a byte iterator.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `iter` | `object` |  | A `ListIterator` over the raw bytes. |

**Returns** `string` — The line contents, without the trailing CRLF.

**Throws** `string` — If no CRLF terminator is found before the iterator is exhausted.

### `http::dechunk`

```kiwi
http::dechunk(data: bytes): bytes
```

Decodes a chunked transfer-encoded byte sequence.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `bytes` |  | The raw chunked-encoded bytes. |

**Returns** `bytes` — The decoded body bytes.

**Throws** `string` — If a chunk size or terminator is malformed or the data is incomplete.

### `http::parse`

```kiwi
http::parse(raw: bytes)
```

Parses raw socket bytes into a request hashmap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `raw` | `bytes` |  | The raw bytes received from a socket. |

**Returns** `hashmap|none` — A hashmap with `method`, `path`, `query`, `version`, `headers`, `body`, `form`, and `raw` keys, or `null` if the request is malformed or incomplete.

### `http::response_bytes`

```kiwi
http::response_bytes(status: integer, body: any = "", headers: hashmap = {}): bytes
```

Builds a raw HTTP/1.1 response as bytes.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `status` | `integer` |  | The HTTP status code (e.g., `200`, `404`). |
| `body` | `any` | `""` | The response body — a string or bytes (default `""`). |
| `headers` | `hashmap` | `{}` | Additional headers to include or override defaults (default `{}`). |

**Returns** `bytes` — The fully-formed HTTP response bytes, including headers and body.

### `http::html_response`

```kiwi
http::html_response(status: integer = 200, body: string, headers: hashmap = {}): bytes
```

Builds an HTTP response with `Content-Type: text/html`.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `status` | `integer` | `200` | The HTTP status code (default `200`). |
| `body` | `string` |  | The HTML body string. |
| `headers` | `hashmap` | `{}` | Additional headers (default `{}`). |

**Returns** `bytes` — The fully-formed HTTP response bytes.

### `http::json_response`

```kiwi
http::json_response(status: integer = 200, data: any, headers: hashmap = {}): bytes
```

Builds an HTTP response with `Content-Type: application/json`.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `status` | `integer` | `200` | The HTTP status code (default `200`). |
| `data` | `any` |  | The data to serialize as JSON. |
| `headers` | `hashmap` | `{}` | Additional headers (default `{}`). |

**Returns** `bytes` — The fully-formed HTTP response bytes.

### `http::set_timeout`

```kiwi
http::set_timeout(timeout_ms: integer)
```

Sets the global HTTP client timeout.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `timeout_ms` | `integer` |  | The timeout in milliseconds. |

### `http::__get__`

```kiwi
http::__get__(url: string, headers: hashmap = {})
```

Enqueues an async GET request and returns an awaitable task ID.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `url` | `string` |  | The URL to request. |
| `headers` | `hashmap` | `{}` | Optional request headers (default `{}`). |

**Returns** `integer` — An awaitable task ID that resolves to a raw response hashmap.

### `http::post`

```kiwi
http::post(url: string, body: any = null, headers: hashmap = {})
```

Enqueues an async POST request and returns an awaitable task ID.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `url` | `string` |  | The URL to request. |
| `body` | `any` | `null` | The request body — string, bytes, or hashmap (auto-serialized to JSON) (default `null`). |
| `headers` | `hashmap` | `{}` | Optional request headers (default `{}`). |

**Returns** `integer` — An awaitable task ID that resolves to a raw response hashmap.

### `http::put`

```kiwi
http::put(url: string, body: any = null, headers: hashmap = {})
```

Enqueues an async PUT request and returns an awaitable task ID.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `url` | `string` |  | The URL to request. |
| `body` | `any` | `null` | The request body — string, bytes, or hashmap (auto-serialized to JSON) (default `null`). |
| `headers` | `hashmap` | `{}` | Optional request headers (default `{}`). |

**Returns** `integer` — An awaitable task ID that resolves to a raw response hashmap.

### `http::delete`

```kiwi
http::delete(url: string, headers: hashmap = {})
```

Enqueues an async DELETE request and returns an awaitable task ID.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `url` | `string` |  | The URL to request. |
| `headers` | `hashmap` | `{}` | Optional request headers (default `{}`). |

**Returns** `integer` — An awaitable task ID that resolves to a raw response hashmap.

### `http::await_get`

```kiwi
http::await_get(url: string, headers: hashmap = {}): HttpResponse
```

Sends a GET request and awaits the result as an `HttpResponse`.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `url` | `string` |  | The URL to request. |
| `headers` | `hashmap` | `{}` | Optional request headers (default `{}`). |

**Returns** `HttpResponse` — The wrapped HTTP response.

### `http::get`

```kiwi
http::get(url: string, headers: hashmap = {}): HttpResponse
```

Sends a GET request and awaits the result as an `HttpResponse`.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `url` | `string` |  | The URL to request. |
| `headers` | `hashmap` | `{}` | Optional request headers (default `{}`). |

**Returns** `HttpResponse` — The wrapped HTTP response.

### `http::await_post`

```kiwi
http::await_post(url: string, body: any = null, headers: hashmap = {}): HttpResponse
```

Sends a POST request and awaits the result as an `HttpResponse`.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `url` | `string` |  | The URL to request. |
| `body` | `any` | `null` | The request body (default `null`). |
| `headers` | `hashmap` | `{}` | Optional request headers (default `{}`). |

**Returns** `HttpResponse` — The wrapped HTTP response.

### `http::await_json_get`

```kiwi
http::await_json_get(url: string, headers: hashmap = {}): hashmap
```

Sends a GET request, awaits the result, and returns the JSON body.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `url` | `string` |  | The URL to request. |
| `headers` | `hashmap` | `{}` | Optional request headers (default `{}`). |

**Returns** `hashmap` — The parsed JSON response body.

### `http::await_json_post`

```kiwi
http::await_json_post(url: string, data: hashmap, headers: hashmap = {}): hashmap
```

Sends a POST request with JSON data, awaits the result, and returns the JSON body.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `url` | `string` |  | The URL to request. |
| `data` | `hashmap` |  | The data to send as JSON. |
| `headers` | `hashmap` | `{}` | Optional request headers (default `{}`). |

**Returns** `hashmap` — The parsed JSON response body.

### `http::await_download`

```kiwi
http::await_download(url: string, filename: string, headers: hashmap = {}): boolean
```

Downloads a URL to a file and returns whether it succeeded.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `url` | `string` |  | The URL to download. |
| `filename` | `string` |  | The local file path to save the download to. |
| `headers` | `hashmap` | `{}` | Optional request headers (default `{}`). |

**Returns** `boolean` — `true` if the download succeeded.

### `http::response`

```kiwi
http::response(raw: hashmap): HttpResponse
```

Wraps a raw response hashmap in an `HttpResponse` object.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `raw` | `hashmap` |  | The raw response hashmap. |

**Returns** `HttpResponse` — The wrapped response.

## `HttpResponse` struct

A structured HTTP response object wrapping the raw response hashmap.

### `HttpResponse.new`

```kiwi
HttpResponse.new(raw: hashmap)
```

Creates an `HttpResponse` from a raw response hashmap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `raw` | `hashmap` |  | A hashmap with `status`, `ok`, `headers`, `body`, and `url` keys. |

### `.get_raw`

```kiwi
get_raw(): hashmap
```

Returns the underlying raw response hashmap.

**Returns** `hashmap` — The raw response data.

### `.json`

```kiwi
json()
```

Parses and returns the response body as a hashmap (JSON).

**Returns** `hashmap` — The parsed JSON body.

**Throws** `string` — If the body cannot be interpreted as JSON.

### `.text`

```kiwi
text()
```

Returns the response body as a string.

**Returns** `string` — The body as text.

**Throws** `string` — If the body cannot be converted to a string.

### `.pretty`

```kiwi
pretty()
```

Pretty-prints the response body as formatted JSON.

### `.save`

```kiwi
save(filename: string)
```

Saves the response body to a file.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `filename` | `string` |  | The path to write the body to. |

**Returns** `boolean` — `true` if the body was written successfully, `false` otherwise.
