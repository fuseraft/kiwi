# `@kiwi/web`

The `web` module contains functionality for building web applications in Kiwi.

*Note: this module is likely to evolve over time.*

## Importing the Module

To use the `web` module, import it at the beginning of your Kiwi script.

```ruby
import "@kiwi/web"
```

## Module Functions

### `ok(_content, _content_type, _status = 200)`
- **Summary**: Returns an OK response.
- **Parameters**:
  - `_content`: The content to return.
  - `_content_type`: The content-type.
  - `_status`: The status code. Defaults to 200.
- **Returns**: Hash with content, content-type, and status.

### `bad(_content, _content_type, _status = 500)`
- **Summary**: Returns a BAD response.
- **Parameters**:
  - `_content`: The content to return.
  - `_content_type`: The content-type.
  - `_status`: The status code. Defaults to 200.
- **Returns**: Hash with content, content-type, and status.

### `get(_endpoint, _handler)`
- **Summary**: Registers a GET endpoint.
- **Parameters**:
  - `_endpoint`: The endpoint to register.
  - `_handler`: A request handler lambda.

### `post(_endpoint, _handler)`
- **Summary**: Registers a POST endpoint.
- **Parameters**:
  - `_endpoint`: The endpoint to register.
  - `_handler`: A request handler lambda.

### `listen(_ipaddr = "0.0.0.0", _port = 8080)`
- **Summary**: Instructs the web server to listen for HTTP requests.
- **Parameters**:
  - `_ipaddr`: The host. Defaults to 0.0.0.0.
  - `_port`: The port. Defaults to 8080.
