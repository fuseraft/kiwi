# `curl`

The `curl` package provides a thin wrapper around the system `curl` command for making HTTP requests from Kiwi programs. It requires `curl` to be installed and available on the system `PATH`.

```kiwi
import "curl"
```

All functions return the response body as a `string` when `capture_output` is `true` (the default), or the `curl` process exit code as an `integer` when `capture_output` is `false`.

---

## Package Functions

### `curl::request(url, method?, headers?, data?, capture_output?)`

Executes a `curl` command to perform an arbitrary HTTP request. This is the underlying function used by all other helpers in this package.

**Parameters**

| Type | Name | Description | Default |
| :--- | :--- | :--- | :--- |
| `string` | `url` | The URL to request. | — |
| `string` | `method` | The HTTP method (`"GET"`, `"POST"`, `"PUT"`, `"DELETE"`, etc.). | `"GET"` |
| `hashmap` | `headers` | Request headers as a hashmap of name -> value pairs. | `{}` |
| `string` | `data` | Request body. Only sent for `POST` and `PUT` requests. | `""` |
| `boolean` | `capture_output` | If `true`, return the response body. If `false`, return the exit code. | `true` |

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | The response body when `capture_output` is `true`. |
| `integer` | The `curl` process exit code when `capture_output` is `false`. |

**Example**

```kiwi
import "curl"

response = curl::request("https://httpbin.org/get", "GET", {"Accept": "application/json"})
println response
```

---

### `curl::get(url, headers?, capture_output?)`

Sends an HTTP `GET` request to the specified URL.

**Parameters**

| Type | Name | Description | Default |
| :--- | :--- | :--- | :--- |
| `string` | `url` | The URL to request. | — |
| `hashmap` | `headers` | Optional request headers. | `{}` |
| `boolean` | `capture_output` | If `true`, return the response body. If `false`, return the exit code. | `true` |

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | The response body when `capture_output` is `true`. |
| `integer` | The `curl` process exit code when `capture_output` is `false`. |

**Example**

```kiwi
import "curl"

# Simple GET request
body = curl::get("https://httpbin.org/get")
println body

# GET with custom headers
body = curl::get("https://api.example.com/data", {"Authorization": "Bearer token123"})
println body

# Check only that the request succeeded (exit code 0 = success)
code = curl::get("https://example.com", {}, false)
println code  # prints: 0
```

---

### `curl::post(url, headers?, data?, capture_output?)`

Sends an HTTP `POST` request with an optional body.

**Parameters**

| Type | Name | Description | Default |
| :--- | :--- | :--- | :--- |
| `string` | `url` | The URL to request. | — |
| `hashmap` | `headers` | Optional request headers. | `{}` |
| `string` | `data` | The request body to send. | `""` |
| `boolean` | `capture_output` | If `true`, return the response body. If `false`, return the exit code. | `true` |

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | The response body when `capture_output` is `true`. |
| `integer` | The `curl` process exit code when `capture_output` is `false`. |

**Example**

```kiwi
import "curl"

headers = {"Content-Type": "application/json"}
payload = '{"name": "kiwi", "version": "1.0"}'

response = curl::post("https://httpbin.org/post", headers, payload)
println response
```

---

### `curl::put(url, headers?, data?, capture_output?)`

Sends an HTTP `PUT` request with an optional body.

**Parameters**

| Type | Name | Description | Default |
| :--- | :--- | :--- | :--- |
| `string` | `url` | The URL to request. | — |
| `hashmap` | `headers` | Optional request headers. | `{}` |
| `string` | `data` | The request body to send. | `""` |
| `boolean` | `capture_output` | If `true`, return the response body. If `false`, return the exit code. | `true` |

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | The response body when `capture_output` is `true`. |
| `integer` | The `curl` process exit code when `capture_output` is `false`. |

**Example**

```kiwi
import "curl"

headers = {"Content-Type": "application/json", "Authorization": "Bearer mytoken"}
payload = '{"status": "active"}'

response = curl::put("https://api.example.com/resource/42", headers, payload)
println response
```

---

### `curl::delete(url, headers?, capture_output?)`

Sends an HTTP `DELETE` request to the specified URL.

**Parameters**

| Type | Name | Description | Default |
| :--- | :--- | :--- | :--- |
| `string` | `url` | The URL to request. | — |
| `hashmap` | `headers` | Optional request headers. | `{}` |
| `boolean` | `capture_output` | If `true`, return the response body. If `false`, return the exit code. | `true` |

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | The response body when `capture_output` is `true`. |
| `integer` | The `curl` process exit code when `capture_output` is `false`. |

**Example**

```kiwi
import "curl"

headers = {"Authorization": "Bearer mytoken"}
response = curl::delete("https://api.example.com/resource/42", headers)
println response
```

---

## Complete Example

```kiwi
import "curl"

base = "https://httpbin.org"

# GET request
println "-- GET --"
println curl::get(base + "/get")

# POST JSON
println "-- POST --"
headers = {"Content-Type": "application/json"}
body = '{"lang": "kiwi"}'
println curl::post(base + "/post", headers, body)

# PUT
println "-- PUT --"
println curl::put(base + "/put", headers, '{"status": "ok"}')

# DELETE
println "-- DELETE --"
println curl::delete(base + "/delete")

# Check exit code only (no output capture)
code = curl::get(base + "/status/200", {}, false)
println "Exit code: " + code  # prints: Exit code: 0
```

---

## Notes

- All requests are made silently (`-s` flag) — `curl`'s progress meter and error output are suppressed.
- The `data` parameter is only sent when the method is `POST` or `PUT`. For `GET` and `DELETE` requests, any `data` argument is ignored.
- To send form-encoded data, set the `Content-Type` header to `"application/x-www-form-urlencoded"` and format `data` accordingly (e.g., `"key1=val1&key2=val2"`).
- For non-blocking or async HTTP, see the [`http`](http.md) package.
