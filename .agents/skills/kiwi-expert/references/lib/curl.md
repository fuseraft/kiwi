# curl

> Make HTTP requests by running the system curl command.

The `curl` package makes HTTP requests by running the `curl` program, which must be installed and on your `PATH`. It is a simple, synchronous alternative to the [`http`](http.md) package, convenient for quick scripts.

```kiwi
body = curl::get("https://httpbin.org/get")
println body

headers = {"Content-Type": "application/json"}
println curl::post("https://httpbin.org/post", headers, '{"lang": "kiwi"}')
println curl::put("https://httpbin.org/put", headers, '{"status": "ok"}')
println curl::delete("https://httpbin.org/delete")
```

Each function returns the response body as a string. Pass `false` as the last argument (`capture_output`) to get `curl`'s exit code instead:

```kiwi
code = curl::get("https://example.com", {}, false)
println code == 0 ? "reachable" : "failed"
```

Notes:

- Requests run with `curl -s`, so progress output is suppressed.
- A request body is only sent for `POST` and `PUT`.
- For form data, set `Content-Type` to `application/x-www-form-urlencoded` and pass `key=value&key2=value2` as the body.
- For response status codes and headers, or concurrent requests, use [`http`](http.md).

## Functions

### `curl::request`

```kiwi
curl::request(url: string, method: string = "GET", headers: hashmap = {}, data: string = "", capture_output: boolean = true)
```

Executes a curl command to perform an HTTP request.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `url` | `string` |  | The URL to which the request will be sent. |
| `method` | `string` | `"GET"` | The HTTP method (e.g., `"GET"`, `"POST"`) (default `"GET"`). |
| `headers` | `hashmap` | `{}` | A hashmap of headers to include in the request (default `{}`). |
| `data` | `string` | `""` | Optional data to send with the request, used for POST and PUT (default `""`). |
| `capture_output` | `boolean` | `true` | If `true`, returns the response body as a string; otherwise returns the exit code (default `true`). |

**Returns** `string|integer` — The response body if `capture_output` is `true`, or the exit code otherwise.

### `curl::get`

```kiwi
curl::get(url: string, headers: hashmap = {}, capture_output: boolean = true)
```

Sends a GET request to the specified URL.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `url` | `string` |  | The URL to request. |
| `headers` | `hashmap` | `{}` | Optional headers (default `{}`). |
| `capture_output` | `boolean` | `true` | If `true`, returns the response body (default `true`). |

**Returns** `string|integer` — The response body or exit code.

### `curl::post`

```kiwi
curl::post(url: string, headers: hashmap = {}, data: string = "", capture_output: boolean = true)
```

Sends a POST request to the specified URL.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `url` | `string` |  | The URL to request. |
| `headers` | `hashmap` | `{}` | Optional headers (default `{}`). |
| `data` | `string` | `""` | The request body (default `""`). |
| `capture_output` | `boolean` | `true` | If `true`, returns the response body (default `true`). |

**Returns** `string|integer` — The response body or exit code.

### `curl::put`

```kiwi
curl::put(url: string, headers: hashmap = {}, data: string = "", capture_output: boolean = true)
```

Sends a PUT request to the specified URL.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `url` | `string` |  | The URL to request. |
| `headers` | `hashmap` | `{}` | Optional headers (default `{}`). |
| `data` | `string` | `""` | The request body (default `""`). |
| `capture_output` | `boolean` | `true` | If `true`, returns the response body (default `true`). |

**Returns** `string|integer` — The response body or exit code.

### `curl::delete`

```kiwi
curl::delete(url: string, headers: hashmap = {}, capture_output: boolean = true)
```

Sends a DELETE request to the specified URL.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `url` | `string` |  | The URL to request. |
| `headers` | `hashmap` | `{}` | Optional headers (default `{}`). |
| `capture_output` | `boolean` | `true` | If `true`, returns the response body (default `true`). |

**Returns** `string|integer` — The response body or exit code.
