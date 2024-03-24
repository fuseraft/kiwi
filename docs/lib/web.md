# `@astral/web`

The `web` module contains functionality for building web applications in Astral.

*Note: this module is likely to evolve over time.*

## Table of Contents

- [Importing the Module](#importing-the-module)
- [Module Functions](#module-functions)
  - [`ok(_content, _content_type)`](#ok_content-_content_type-_status--200)
  - [`bad(_content, _content_type)`](#bad_content-_content_type-_status--500)
  - [`redirect(_url)`](#redirect_url-_status--302)
  - [`get(_endpoint, _handler)`](#get_endpoint-_handler)
  - [`post(_endpoint, _handler)`](#post_endpoint-_handler)
  - [`listen(_ipaddr, _port)`](#listen_ipaddr--0000-_port--8080)
  - [`public(_public_endpoint, _public_path)`](#public_public_endpoint-_public_path)


## Importing the Module

To use the `web` module, import it at the beginning of your Astral script.

```ruby
import "@astral/web" as web
```

## Module Functions

### `ok(_content, _content_type, _status = 200)`

Returns an OK response.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_content` | The content to return. |
| `String` | `_content_type` | The content-type. |
| `Integer` | `_status` | The status code. Defaults to 200. |

**Returns**
| Type | Description |
| :--- | :---|
| `Hash` | Contains content, content-type, and status. |


### `bad(_content, _content_type, _status = 500)`

Returns a BAD response.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_content` | The content to return. |
| `String` | `_content_type` | The content-type. |
| `Integer` | `_status` | The status code. Defaults to 500. |

**Returns**
| Type | Description |
| :--- | :---|
| `Hash` | Contains content, content-type, and status. |

### `redirect(_url, _status = 302)`

Redirect a request.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_url` | The URL to redirect to. |
| `Integer` | `_status` | The status code. Defaults to 302 |

**Returns**
| Type | Description |
| :--- | :---|
| `Hash` | Contains redirect and status. |

### `get(_endpoint, _handler)`

Registers a GET endpoint.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_endpoint` | The endpoint to register. |
| `Lambda` | `_handler` | A request handler. |

### `post(_endpoint, _handler)`

Registers a POST endpoint.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_endpoint` | The endpoint to register. |
| `Lambda` | `_handler` | A request handler. |

### `listen(_ipaddr = "0.0.0.0", _port = 8080)`

Instructs the web server to listen for HTTP requests.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_ipaddr` | The host. Defaults to 0.0.0.0. |
| `Integer` | `_port` | The port. Defaults to 8080. |

### `public(_public_endpoint, _public_path)`

Instructs the web server to serve static content.
  
**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_public_endpoint` | The endpoint at which static content is served. |
| `String` | `_public_path` | The server-side path containing static content to be served. |

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The standard output of an external process. |