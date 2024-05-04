# `@kiwi/http`

The `http` package contains functionality for working with HTTP requests.

## Table of Contents

- [Importing the Package](#importing-the-package)
- [Example GET Request](#example-get-request)
- [Package Functions](#package-functions)
  - [`delete_(_url, _path, _headers)`](#delete__url-_path-_headers)
  - [`get(_url, _path, _headers)`](#get_url-_path-_headers)
  - [`head(_url, _path, _headers)`](#head_url-_path-_headers)
  - [`options(_url, _path, _headers)`](#options_url-_path-_headers)
  - [`patch(_url, _path, _body, _content_type, _headers)`](#patch_url-_path-_body-_content_type-_headers)
  - [`post(_url, _path, _body, _content_type, _headers)`](#post_url-_path-_body-_content_type-_headers)
  - [`put(_url, _path, _body, _content_type, _headers)`](#put_url-_path-_body-_content_type-_headers)

## Importing the Package

To use the `http` package, import it at the beginning of your Kiwi script.

```ruby
import "@kiwi/http"
```

## Example GET Request

```ruby
import "@kiwi/http" as http

res = http.get("http://httpbin.org", "/get")

if res.status == 200
  println("${res.headers}${res.body}")
else
  println("An error occurred making GET request.")
end
```

## Package Functions

### `delete_(_url, _path, _headers)`

Performs an HTTP DELETE request to the specified URL.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_url` | The base URL. |
| `String` | `_path` | The path to request. |
| `Hash` | `_headers` | The headers. Defaults to `{}`. |

**Returns**
| Type | Description |
| :--- | :---|
| `Hash` | A response hash containing status, headers, and body. |

### `get(_url, _path, _headers)`

Performs an HTTP GET request to the specified URL.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_url` | The base URL. |
| `String` | `_path` | The path to request. |
| `Hash` | `_headers` | The headers. Defaults to `{}`. |

**Returns**
| Type | Description |
| :--- | :---|
| `Hash` | A response hash containing status, headers, and body. |

### `head(_url, _path, _headers)`

Performs an HTTP HEAD request to the specified URL.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_url` | The base URL. |
| `String` | `_path` | The path to request. |
| `Hash` | `_headers` | The headers. Defaults to `{}`. |

**Returns**
| Type | Description |
| :--- | :---|
| `Hash` | A response hash containing status, headers, and body. |

### `options(_url, _path, _headers)`

Performs an HTTP OPTIONS request to the specified URL.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_url` | The base URL. |
| `String` | `_path` | The path to request. |
| `Hash` | `_headers` | The headers. Defaults to `{}`. |

**Returns**
| Type | Description |
| :--- | :---|
| `Hash` | A response hash containing status, headers, and body. |

### `patch(_url, _path, _body, _content_type, _headers)`

Performs an HTTP PATCH request to the specified URL.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_url` | The base URL. |
| `String` | `_path` | The path to request. |
| `String` | `_body` | The request body. |
| `String` | `_content_type` | The content-type. Defaults to `"text/plain"`. |
| `Hash` | `_headers` | The headers. Defaults to `{}`. |

**Returns**
| Type | Description |
| :--- | :---|
| `Hash` | A response hash containing status, headers, and body. |

### `post(_url, _path, _body, _content_type, _headers)`

Performs an HTTP POST request to the specified URL.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_url` | The base URL. |
| `String` | `_path` | The path to request. |
| `String` | `_body` | The request body. |
| `String` | `_content_type` | The content-type. Defaults to `"text/plain"`. |
| `Hash` | `_headers` | The headers. Defaults to `{}`. |

**Returns**
| Type | Description |
| :--- | :---|
| `Hash` | A response hash containing status, headers, and body. |

### `put(_url, _path, _body, _content_type, _headers)`

Performs an HTTP PUT request to the specified URL.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_url` | The base URL. |
| `String` | `_path` | The path to request. |
| `String` | `_body` | The request body. |
| `String` | `_content_type` | The content-type. Defaults to `"text/plain"`. |
| `Hash` | `_headers` | The headers. Defaults to `{}`. |

**Returns**
| Type | Description |
| :--- | :---|
| `Hash` | A response hash containing status, headers, and body. |