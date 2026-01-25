# `http`

The `http` package provides a high-level, idiomatic HTTP client for Kiwi.  

It builds on the low-level `__http_get__`, `__http_post__`, and `__http_download__` builtins, adding conveniences such as automatic JSON handling, response wrapping, and utility functions for common use cases.

## Table of Contents

- [Package Functions](#package-functions)
  - [`get(url, headers?, opts?)`](#geturl-headers-opts)
  - [`post(url, body?, headers?, opts?)`](#posturl-body-headers-opts)
  - [`put(url, body?, headers?, opts?)`](#puturl-body-headers-opts)
  - [`delete(url, headers?, opts?)`](#deleteurl-headers-opts)
  - [`await_get(url, headers?, opts?)`](#await_geturl-headers-opts)
  - [`await_post(url, body?, headers?, opts?)`](#await_posturl-body-headers-opts)
  - [`await_json_get(url, headers?, opts?)`](#await_json_geturl-headers-opts)
  - [`await_json_post(url, data, headers?, opts?)`](#await_json_posturl-data-headers-opts)
  - [`await_download(url, filename, headers?, opts?)`](#await_downloadurl-filename-headers-opts)
  - [`response(raw)`](#responsraw)
- [`HttpResponse` struct](#httpresponse)
  - [Constructor](#constructor)
  - [`get_raw()`](#get_raw)
  - [`json()`](#json)
  - [`text()`](#text)
  - [`pretty()`](#pretty)
  - [`save(filename)`](#savefilename)

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
Performs an HTTP PUT request (currently uses `__http_put__` – typo in your code; fix to `__http_put__` when implemented).

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
