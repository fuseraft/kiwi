# `http`

The `http` package provides a high-level, idiomatic HTTP client for Kiwi.  

It builds on the low-level `__http_get__`, `__http_post__`, and `__http_download__` builtins, adding conveniences such as automatic JSON handling, response wrapping, and utility functions for common use cases.

---

## Package Functions

### `get(url, headers?, opts?)`
Performs an HTTP GET request. Returns a task ID that can be awaited.

**Parameters**

| Type      | Name      | Description                                      | Default |
|-----------|-----------|--------------------------------------------------|---------|
| `string`  | `url`     | The target URL                                   | —       |
| `hashmap` | `headers` | Optional request headers                         | `{}`    |
| `hashmap` | `opts`    | Optional settings (not yet used)                 | `{}`    |

**Returns**

| Type      | Description                  |
|-----------|------------------------------|
| `integer` | Task ID (await with `task.await()`) |

---

### `post(url, body?, headers?, opts?)`
Performs an HTTP POST request. If `body` is a hashmap, it is automatically JSON-encoded and the `Content-Type` header is set accordingly.

**Parameters**

| Type      | Name      | Description                                      | Default |
|-----------|-----------|--------------------------------------------------|---------|
| `string`  | `url`     | The target URL                                   | —       |
| `any`     | `body`    | Request body (string, bytes, or hashmap)         | `null`  |
| `hashmap` | `headers` | Optional request headers                         | `{}`    |
| `hashmap` | `opts`    | Optional settings (not yet used)                 | `{}`    |

**Returns**

| Type      | Description                  |
|-----------|------------------------------|
| `integer` | Task ID (await with `task.await()`) |

---

### `put(url, body?, headers?, opts?)`
Performs an HTTP PUT request.

**Parameters**  
Same as `post`.

**Returns**  
Task ID.

---

### `delete(url, headers?, opts?)`
Performs an HTTP DELETE request (currently uses `__http_delete__`).

**Parameters**  
Same as `get` (no body).

**Returns**  
Task ID.

---

### `await_get(url, headers?, opts?)`
Convenience: performs GET and awaits the result, wrapped in `HttpResponse`.

**Parameters**  
Same as `get`.

**Returns**

| Type           | Description                     |
|----------------|---------------------------------|
| `HttpResponse` | The response object             |

---

### `await_post(url, body?, headers?, opts?)`
Convenience: performs POST, awaits, and wraps in `HttpResponse`.

**Parameters**  
Same as `post`.

**Returns**  
`HttpResponse`

---

### `await_json_get(url, headers?, opts?)`
Performs GET, awaits, and returns the parsed JSON body (calls `.json()` on response).

**Parameters**  
Same as `get`.

**Returns**

| Type      | Description              |
|-----------|--------------------------|
| `hashmap` | Decoded JSON object      |

---

### `await_json_post(url, data, headers?, opts?)`
Performs POST with JSON body, awaits, and returns parsed JSON response.

**Parameters**

| Type      | Name      | Description                     |
|-----------|-----------|---------------------------------|
| `string`  | `url`     | Target URL                      |
| `hashmap` | `data`    | Data to encode as JSON          |
| `hashmap` | `headers` | Optional headers                |
| `hashmap` | `opts`    | Optional settings               |

**Returns**  
`hashmap` (decoded JSON)

---

### `await_download(url, filename, headers?, opts?)`
Downloads the response body to a file and returns whether it succeeded.

**Parameters**

| Type      | Name       | Description                     |
|-----------|------------|---------------------------------|
| `string`  | `url`      | Source URL                      |
| `string`  | `filename` | Destination path                |
| `hashmap` | `headers`  | Optional headers                |
| `hashmap` | `opts`     | Optional settings               |

**Returns**

| Type      | Description                          |
|-----------|--------------------------------------|
| `boolean` | `true` if download succeeded         |

---

### `response(raw)`
Constructs an `HttpResponse` from a raw response hashmap.

**Parameters**

| Type      | Name   | Description                     |
|-----------|--------|---------------------------------|
| `hashmap` | `raw`  | Raw response from `__http_*__`  |

**Returns**

| Type           | Description             |
|----------------|-------------------------|
| `HttpResponse` | Wrapped response object |

---

### `set_timeout(timeout_ms)`
Sets the global HTTP client timeout for all subsequent requests.

**Parameters**

| Type      | Name         | Description                    |
|-----------|--------------|--------------------------------|
| `integer` | `timeout_ms` | Timeout in milliseconds        |

---

### `url_decode(s, decode_plus?)`
Decodes a percent-encoded URL string (e.g. from a query string or form body).

**Parameters**

| Type      | Name           | Description                                          | Default |
|-----------|----------------|------------------------------------------------------|---------|
| `string`  | `s`            | The URL-encoded string to decode                     | —       |
| `boolean` | `decode_plus`  | If `true`, decodes `+` as a space                    | `true`  |

**Returns**

| Type     | Description          |
|----------|----------------------|
| `string` | The decoded string   |

**Example**
```kiwi
import "http"

println http::url_decode("hello%20world")     # hello world
println http::url_decode("a+b", true)         # a b
println http::url_decode("name%3DKiwi")       # name=Kiwi
```

---

### `parse(raw)`
Parses raw TCP socket bytes into a structured HTTP request hashmap. Useful for writing HTTP servers on top of the `socket` package.

**Parameters**

| Type    | Name  | Description                              |
|---------|-------|------------------------------------------|
| `bytes` | `raw` | Raw bytes received from a socket client  |

**Returns**

| Type              | Description                                                                                 |
|-------------------|---------------------------------------------------------------------------------------------|
| `hashmap` or `null` | A hashmap with keys `method`, `path`, `query`, `version`, `headers`, `body`, `form`, `raw`; or `null` if malformed/incomplete |

---

### `response_bytes(status, body?, headers?)`
Builds a complete raw HTTP/1.1 response as bytes, including auto-generated headers. Suitable for sending directly over a `socket`.

**Parameters**

| Type      | Name      | Description                                        | Default |
|-----------|-----------|----------------------------------------------------|---------|
| `integer` | `status`  | HTTP status code (e.g. `200`, `404`)               | —       |
| `any`     | `body`    | Response body — string or bytes                    | `""`    |
| `hashmap` | `headers` | Additional or override headers                     | `{}`    |

**Returns**

| Type    | Description                              |
|---------|------------------------------------------|
| `bytes` | Fully-formed HTTP response bytes         |

---

### `html_response(status?, body, headers?)`
Builds an HTTP response with `Content-Type: text/html`.

**Parameters**

| Type      | Name      | Description                    | Default |
|-----------|-----------|--------------------------------|---------|
| `integer` | `status`  | HTTP status code               | `200`   |
| `string`  | `body`    | HTML body                      | —       |
| `hashmap` | `headers` | Additional headers             | `{}`    |

**Returns** `bytes`

---

### `json_response(status?, data, headers?)`
Builds an HTTP response with `Content-Type: application/json`, serializing `data` automatically.

**Parameters**

| Type      | Name      | Description                    | Default |
|-----------|-----------|--------------------------------|---------|
| `integer` | `status`  | HTTP status code               | `200`   |
| `any`     | `data`    | Data to serialize as JSON      | —       |
| `hashmap` | `headers` | Additional headers             | `{}`    |

**Returns** `bytes`

**Example**
```kiwi
import "http"
import "socket"
import "task"

server = socket::tcpserver("127.0.0.1", 8080)

while true do
  client = task::await(socket::accept(server))
  raw    = task::await(socket::recv(client))
  req    = http::parse(raw)

  if req != null && req["path"] == "/api"
    resp = http::json_response(200, { ok: true, path: req["path"] })
  else
    resp = http::html_response(404, "<h1>Not Found</h1>")
  end

  task::await(socket::send(client, resp))
  socket::close(client)
end
```

---

## `HttpResponse`

A structured wrapper around HTTP responses from the low-level builtins.

### Constructor

```kiwi
HttpResponse.new(raw: hashmap)
```

Creates a new response object from the raw hashmap returned by `__http_*__`.

**Fields**
| Field     | Type      | Description                              |
|-----------|-----------|------------------------------------------|
| `status`  | `integer` | HTTP status code                         |
| `ok`      | `boolean` | `true` if status is 2xx                  |
| `headers` | `hashmap` | Response headers                         |
| `body`    | `any`     | Response body (string or bytes)          |
| `url`     | `string`  | Final URL (after redirects, if any)      |
| `raw`     | `hashmap` | Original raw response hashmap            |

### `get_raw()`
Returns the original raw response hashmap.

**Returns**

| Type      | Description                  |
|-----------|------------------------------|
| `hashmap` | The raw response data        |

---

### `json()`
Parses the body as JSON if possible.

**Returns**

| Type      | Description                  |
|-----------|------------------------------|
| `hashmap` | Decoded JSON object          |

**Throws**  
If body is not JSON-compatible.

---

### `text()`
Returns the body as a string.

**Returns**

| Type      | Description                  |
|-----------|------------------------------|
| `string`  | Body as text                 |

**Throws**  
If body cannot be converted to text.

---

### `pretty()`
Returns a pretty-printed JSON string of the parsed body.

**Returns**

| Type      | Description                  |
|-----------|------------------------------|
| `string`  | Pretty-printed JSON          |

---

### `save(filename)`
Saves the response body to a file.

**Parameters**

| Type     | Name       | Description             |
|----------|------------|-------------------------|
| `string` | `filename` | Path to save to         |

**Returns**

| Type      | Description                          |
|-----------|--------------------------------------|
| `boolean` | `true` if save succeeded, `false` otherwise |

---

## Examples

### Simple GET request

```kiwi
import "http"

resp = http::get("https://httpbin.org/get")
println resp.status    # 200
println resp.ok        # true
println resp.text()
```

### Fetch JSON directly

```kiwi
import "http"

data = http::await_json_get("https://httpbin.org/json")
println data.pretty()
```

### POST with a JSON body

```kiwi
import "http"

payload = { name: "Kiwi", version: 1 }
resp = http::await_post("https://httpbin.org/post", payload)

if resp.ok
  result = resp.json()
  println result["json"]
end
```

### Download a file

```kiwi
import "http"

ok = http::await_download("https://example.com/archive.zip", "/tmp/archive.zip")
println ok   # true on success
```

### Parallel async requests

```kiwi
import "http"
import "task"

# Fire both requests without waiting
t1 = http::__get__("https://httpbin.org/get")
t2 = http::__get__("https://httpbin.org/ip")

# Await each and wrap in HttpResponse
r1 = http::response(task::await(t1))
r2 = http::response(task::await(t2))

println r1.status   # 200
println r2.json()
```

### Build a response for a socket server

```kiwi
import "http"
import "socket"

# Construct a JSON response to send back over a raw TCP socket
bytes = http::json_response(200, { message: "Hello from Kiwi" })
socket::send(client, bytes)

# Or an HTML response
page = http::html_response(200, "<h1>Hello</h1>")
socket::send(client, page)
```

### Parse a raw HTTP request

```kiwi
import "http"
import "socket"

# Inside a TCP server handler, after receiving bytes from the client:
req = http::parse(raw_bytes)

if req != null
  println req["method"]           # "GET"
  println req["path"]             # "/api/users"
  println req["query"]["page"]    # query string param
  println req["headers"]["host"]  # request header
end
```

### Set a request timeout

```kiwi
import "http"

http::set_timeout(5000)   # 5-second timeout

resp = http::get("https://httpbin.org/delay/1")
println resp.ok
```
