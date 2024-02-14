# `@kiwi/http`

The `http` module contains functionality for working with HTTP requests.

*Note: this module is likely to evolve over time.*

## Importing the Module

To use the `http` module, import it at the beginning of your Kiwi script.

```ruby
import "@kiwi/http"
```

## Example GET Request

```ruby
import "@kiwi/http"

@response = http::get("http://httpbin.org/get", [])

if @response["status"] == 200
  @body = @response["body"]
  @headers = @response["headers"]
  println "${@headers}${@body}"
else
  println "An error occurred making GET request."
end
```

## Module Functions

### `delete_(@_url, @_headers)`
- **Summary**: Performs an HTTP DELETE request to the specified URL.
- **Parameters**:
  - `@_url`: The URL to request.
  - `@_headers`: The list of headers.
- **Returns**: Hash with the response status code, headers, and body.

### `get(@_url, @_headers)`
- **Summary**: Performs an HTTP GET request to the specified URL.
- **Parameters**:
  - `@_url`: The URL to request.
  - `@_headers`: The list of headers.
- **Returns**: Hash with the response status code, headers, and body.

### `head(@_url, @_headers)`
- **Summary**: Performs an HTTP HEAD request to the specified URL.
- **Parameters**:
  - `@_url`: The URL to request.
  - `@_headers`: The list of headers.
- **Returns**: Hash with the response status code, headers, and body.

### `options(@_url, @_headers)`
- **Summary**: Performs an HTTP OPTIONS request to the specified URL.
- **Parameters**:
  - `@_url`: The URL to request.
  - `@_headers`: The list of headers.
- **Returns**: Hash with the response status code, headers, and body.

### `patch(@_url, @_body, @_headers)`
- **Summary**: Performs an HTTP PATCH request to the specified URL.
- **Paramaters**:
    - `@_url`: The URL to request.
    - `@_body`: The body to send.
    - `@_headers`: The list of headers.
- **Returns**: Hash with the response status code, headers, and body.

### `post(@_url, @_body, @_headers)`
- **Summary**: Performs an HTTP POST request to the specified URL.
- **Paramaters**:
    - `@_url`: The URL to request.
    - `@_body`: The body to send.
    - `@_headers`: The list of headers.
- **Returns**: Hash with the response status code, headers, and body.

### `put(@_url, @_body, @_headers)`
- **Summary**: Performs an HTTP PUT request to the specified URL.
- **Paramaters**:
    - `@_url`: The URL to request.
    - `@_body`: The body to send.
    - `@_headers`: The list of headers.
- **Returns**: Hash with the response status code, headers, and body.