# httpserver

> An HTTP server with routing, middleware, route groups, static files, cookies and CORS.

The `httpserver` package runs an HTTP server. Register handlers for routes, add middleware, and start it. Handlers receive a `Request` and a `Response`.

```kiwi
server = httpserver::create(18087)

server.get("/", with (req, res) do
  res.text("Hello, World!")
end)

server.get("/greet/:name", with (req, res) do
  res.json({message: "Hello, ${req.param("name")}!"})
end)

server.start()            # serves on a background task
task::sleep(200)

println http::get("http://localhost:18087/greet/ada").json()   # {"message": "Hello, ada!"}
server.stop()
```

`start()` returns immediately and serves requests in the background, which suits scripts that do other work. A program whose only job is serving should call `server.start(true)`, which blocks until `server.stop()` is called.

The server logs each request to stderr; set `.logging("none")` in the [configuration](#configuration) to turn that off. httpserver is unavailable in [safe mode](../tools/cli.md#safe-mode).

## Routes

`get`, `post`, `put`, `patch` and `delete` register a handler for a method and path; `all_methods` matches any method. Each returns the server, so registrations can be chained. Paths can contain:

- literal segments: `/users/profile`
- named parameters: `/users/:id`, read with `req.param("id")`
- a trailing wildcard: `/files/*`, read with `req.param("*")`

`group(prefix)` returns a router whose routes all start with `prefix`:

```kiwi
api = server.group("/api/v1")

api.get("/users", with (req, res) do
  res.json([{id: 1, name: "Alice"}])
end)

api.post("/users", with (req, res) do
  res.json({created: req.json()}, 201)
end)
```

Requests that match no route get a `404 Not Found`.

## Requests

| Method | Returns |
| :-- | :-- |
| `method()`, `path()` | The HTTP method and path |
| `param(name)`, `params()` | Route parameters |
| `query()`, `query_param(name)` | Query string values |
| `header(name)`, `headers()` | Request headers (names are case-insensitive) |
| `body()` | The raw body as a string |
| `json()`, `is_json()` | The body parsed as JSON |
| `form()`, `is_form()` | Form fields from a URL-encoded body |
| `cookie(name)`, `cookies()` | Request cookies |
| `content_type()`, `remote_addr()` | The content type and client address |

## Responses

| Method | Sends |
| :-- | :-- |
| `text(body, status)` | Plain text |
| `html(body, status)` | HTML |
| `json(value, status)` | JSON |
| `file(path)` | A file, with a content type based on its extension |
| `redirect(url, status)` | A redirect |
| `send(body, status)` | A raw body |
| `not_found()`, `bad_request()`, `unauthorized()`, `forbidden()`, `server_error()` | An error status with an optional message |

`status_code(code)`, `header(name, value)`, `cookie(name, value, path, max_age)`, `clear_cookie(name)` and `no_cache()` set response details before sending. Each response is sent once; check `sent()` to see whether a handler already responded.

## Middleware

`use` adds a function that runs before every route handler. It receives the request, the response and `nxt`; call `nxt()` to continue to the next middleware or the handler, or send a response and return without calling it to stop the request there.

```kiwi
server.use(with (req, res, nxt) do
  started = time::ticks()
  nxt()
  eprintln "${req.method()} ${req.path()} took ${time::ticksms(time::ticks() - started)}ms"
end)

server.use(with (req, res, nxt) do
  if req.header("authorization") != "Bearer secret"
    res.unauthorized("Invalid token")
    return
  end
  nxt()
end)
```

## Static files

```kiwi
server.static_files("/public", "./public")   # serves ./public/* at /public/*

server.get("/", with (req, res) do
  res.file("./public/index.html")
end)
```

## Configuration

`httpserver::config()` builds a configuration fluently; pass it to `create_with_config`:

```kiwi
cfg = httpserver::config()
  .host("0.0.0.0")
  .port(9000)
  .workers(8)
  .timeout(30000)
  .cors("https://myapp.example.com")
  .logging("simple")   # "combined", "simple" or "none"

server = httpserver::create_with_config(cfg)
```

## Lifecycle and errors

```kiwi
server.on_start(with () do
  eprintln "listening"
end)

server.on_stop(with () do
  eprintln "shutting down"
end)

server.on_error(with (err, req, res) do
  eprintln "error on ${req.path()}: ${err}"
  res.json({error: "internal error"}, 500)
end)
```

Without an error hook, a handler that throws produces a `500` response.

To stop cleanly on <kbd>Ctrl</kbd>+<kbd>C</kbd>, combine `start(true)` with a [`signal`](signal.md) handler that calls `server.stop()`.

## Functions

### `httpserver::create`

```kiwi
httpserver::create(port = 8080, host = "localhost")
```

Creates a new HTTP server with default configuration.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `port` | `integer` | `8080` | Port to listen on (default 8080). |
| `host` | `string` | `"localhost"` | Bind address (default "localhost"). |

**Returns** Server

### `httpserver::create_with_config`

```kiwi
httpserver::create_with_config(cfg)
```

Creates a new HTTP server from a Config object.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `cfg` | `Config` |  | Server configuration. |

**Returns** Server

### `httpserver::config`

```kiwi
httpserver::config()
```

Returns a new Config builder.

**Returns** Config

## `Config` struct

Server configuration. All setter methods return self for fluent chaining.

### `Config.new`

```kiwi
Config.new()
```

### `.host`

```kiwi
host(v)
```

Sets the bind host.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `v` | `string` |  | Hostname or IP address. |

**Returns** `Config` — self

### `.port`

```kiwi
port(v)
```

Sets the listening port.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `v` | `integer` |  | Port number. |

**Returns** `Config` — self

### `.workers`

```kiwi
workers(v)
```

Sets max concurrent request worker tasks.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `v` | `integer` |  | Worker count. |

**Returns** `Config` — self

### `.timeout`

```kiwi
timeout(v)
```

Sets the accept-poll timeout in milliseconds.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `v` | `integer` |  | Milliseconds to wait per accept call. |

**Returns** `Config` — self

### `.logging`

```kiwi
logging(v)
```

Sets the access log format.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `v` | `string\|boolean` |  | "combined", "simple", "none", or false to disable. |

**Returns** `Config` — self

### `.cors`

```kiwi
cors(origin = "*")
```

Enables CORS with an optional allowed origin.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `origin` | `string` | `"*"` | Allowed origin (default "*"). |

**Returns** `Config` — self

### `.get_host`

```kiwi
get_host()
```

### `.get_port`

```kiwi
get_port()
```

### `.get_workers`

```kiwi
get_workers()
```

### `.get_timeout`

```kiwi
get_timeout()
```

### `.get_log`

```kiwi
get_log()
```

### `.get_cors`

```kiwi
get_cors()
```

### `.get_cors_origin`

```kiwi
get_cors_origin()
```

## `Request` struct

Represents an incoming HTTP request.

### `Request.new`

```kiwi
Request.new(raw, params)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `raw` | `any` |  |  |
| `params` | `any` |  |  |

### `.method`

```kiwi
method()
```

Returns the HTTP method (uppercase).

**Returns** string

### `.path`

```kiwi
path()
```

Returns the URL path.

**Returns** string

### `.query`

```kiwi
query()
```

Returns the query string parameters as a hashmap.

**Returns** hashmap

### `.query_param`

```kiwi
query_param(name, default_val = null)
```

Returns a single query parameter, or default if absent.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | Parameter name. |
| `default_val` | `any` | `null` | Default value (default null). |

**Returns** any

### `.headers`

```kiwi
headers()
```

Returns all request headers as a hashmap (keys lowercased).

**Returns** hashmap

### `.header`

```kiwi
header(name)
```

Returns a header value by name (case-insensitive), or null.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | Header name. |

**Returns** string|null

### `.body`

```kiwi
body()
```

Returns the raw request body string.

**Returns** string

### `.remote_addr`

```kiwi
remote_addr()
```

Returns the client IP address.

**Returns** string

### `.id`

```kiwi
id()
```

Returns the internal request ID (used for responding).

**Returns** integer

### `.param`

```kiwi
param(name)
```

Returns a route parameter by name, or null if not present.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | Parameter name (from the pattern, e.g. "id" for /:id). |

**Returns** string|null

### `.params`

```kiwi
params()
```

Returns all route parameters.

**Returns** hashmap

### `.content_type`

```kiwi
content_type()
```

Returns the Content-Type header value.

**Returns** string|null

### `.is_json`

```kiwi
is_json()
```

Returns true if the Content-Type indicates JSON.

**Returns** boolean

### `.is_form`

```kiwi
is_form()
```

Returns true if the Content-Type indicates form-encoded data.

**Returns** boolean

### `.json`

```kiwi
json()
```

Parses and returns the request body as a JSON value (cached).

**Returns** any

### `.form`

```kiwi
form()
```

Parses the body as URL-encoded form data (cached).

**Returns** hashmap

### `.cookie`

```kiwi
cookie(name)
```

Returns a cookie value by name, or null (cookies parsed lazily).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | Cookie name. |

**Returns** string|null

### `.cookies`

```kiwi
cookies()
```

Returns all cookies as a hashmap (parsed lazily).

**Returns** hashmap

## `Response` struct

Represents an HTTP response being built and sent.

### `Response.new`

```kiwi
Response.new(server_id, request_id)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `server_id` | `any` |  |  |
| `request_id` | `any` |  |  |

### `.sent`

```kiwi
sent()
```

Returns true if the response has already been sent.

**Returns** boolean

### `.status_code`

```kiwi
status_code()
```

Returns the HTTP status code that was (or will be) sent.

**Returns** integer

### `.header`

```kiwi
header(key, value)
```

Sets a response header.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `key` | `string` |  | Header name. |
| `value` | `string` |  | Header value. |

**Returns** `Response` — self

### `.send`

```kiwi
send(status, body)
```

Sends the response with a status code and body.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `status` | `integer` |  | HTTP status code. |
| `body` | `string\|bytes` |  | Response body. |

### `.file`

```kiwi
file(fs_path, status = 200)
```

Sends a file's contents as the response body.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `fs_path` | `string` |  | Filesystem path to the file. |
| `status` | `integer` | `200` | HTTP status code (default 200). |

### `.text`

```kiwi
text(body, status = 200)
```

Sends a plain-text response.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `body` | `string` |  | Response body. |
| `status` | `integer` | `200` | HTTP status code (default 200). |

### `.html`

```kiwi
html(body, status = 200)
```

Sends an HTML response.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `body` | `string` |  | HTML body. |
| `status` | `integer` | `200` | HTTP status code (default 200). |

### `.json`

```kiwi
json(body, status = 200)
```

Sends a JSON response.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `body` | `any` |  | Value to serialize (or a pre-serialized string). |
| `status` | `integer` | `200` | HTTP status code (default 200). |

### `.redirect`

```kiwi
redirect(location, status = 302)
```

Sends a redirect response.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `location` | `string` |  | Target URL. |
| `status` | `integer` | `302` | HTTP status code (default 302). |

### `.cookie`

```kiwi
cookie(name, value, path = "/", max_age = null, http_only = true, secure = false, same_site = "Lax")
```

Appends a Set-Cookie header to the response.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | Cookie name. |
| `value` | `string` |  | Cookie value. |
| `path` | `string` | `"/"` | Cookie path (default "/"). |
| `max_age` | `integer` | `null` | Max-Age in seconds (default null = session cookie). |
| `http_only` | `boolean` | `true` | HttpOnly flag (default true). |
| `secure` | `boolean` | `false` | Secure flag (default false). |
| `same_site` | `string` | `"Lax"` | SameSite policy: "Strict", "Lax", or "None" (default "Lax"). |

**Returns** `Response` — self

### `.clear_cookie`

```kiwi
clear_cookie(name, path = "/")
```

Clears a cookie by setting its expiry to the past.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | Cookie name. |
| `path` | `string` | `"/"` | Cookie path (default "/"). |

**Returns** `Response` — self

### `.no_cache`

```kiwi
no_cache()
```

Sets Cache-Control headers to prevent caching.

**Returns** `Response` — self

### `.not_found`

```kiwi
not_found(msg = "Not Found")
```

Sends a 404 Not Found response.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `"Not Found"` | Response body (default "Not Found"). |

### `.bad_request`

```kiwi
bad_request(msg = "Bad Request")
```

Sends a 400 Bad Request response.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `"Bad Request"` | Response body (default "Bad Request"). |

### `.unauthorized`

```kiwi
unauthorized(msg = "Unauthorized")
```

Sends a 401 Unauthorized response.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `"Unauthorized"` | Response body. |

### `.forbidden`

```kiwi
forbidden(msg = "Forbidden")
```

Sends a 403 Forbidden response.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `"Forbidden"` | Response body. |

### `.server_error`

```kiwi
server_error(msg = "Internal Server Error")
```

Sends a 500 Internal Server Error response.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `"Internal Server Error"` | Response body. |

## `Router` struct

A route group that delegates to a Server with a path prefix applied.

### `Router.new`

```kiwi
Router.new(server, prefix)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `server` | `any` |  |  |
| `prefix` | `any` |  |  |

### `.get`

```kiwi
get(path, handler)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path` | `any` |  |  |
| `handler` | `any` |  |  |

### `.post`

```kiwi
post(path, handler)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path` | `any` |  |  |
| `handler` | `any` |  |  |

### `.put`

```kiwi
put(path, handler)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path` | `any` |  |  |
| `handler` | `any` |  |  |

### `.patch`

```kiwi
patch(path, handler)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path` | `any` |  |  |
| `handler` | `any` |  |  |

### `.delete`

```kiwi
delete(path, handler)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path` | `any` |  |  |
| `handler` | `any` |  |  |

### `.all_methods`

```kiwi
all_methods(path, handler)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path` | `any` |  |  |
| `handler` | `any` |  |  |

### `.group`

```kiwi
group(sub_prefix)
```

Creates a nested sub-router under this group's prefix.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `sub_prefix` | `string` |  | Additional prefix for the sub-group. |

**Returns** `Router` — New router with combined prefix.

## `Server` struct

Production-grade HTTP server with routing, middleware, and static serving.

### `Server.new`

```kiwi
Server.new(cfg)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `cfg` | `any` |  |  |

### `.get`

```kiwi
get(path, handler)
```

Registers a GET route handler.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path` | `string` |  | URL pattern (supports :param and * wildcard). |
| `handler` | `lambda` |  | fn(req, res) handler. |

**Returns** `Server` — self

### `.post`

```kiwi
post(path, handler)
```

Registers a POST route handler.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path` | `any` |  |  |
| `handler` | `any` |  |  |

### `.put`

```kiwi
put(path, handler)
```

Registers a PUT route handler.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path` | `any` |  |  |
| `handler` | `any` |  |  |

### `.patch`

```kiwi
patch(path, handler)
```

Registers a PATCH route handler.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path` | `any` |  |  |
| `handler` | `any` |  |  |

### `.delete`

```kiwi
delete(path, handler)
```

Registers a DELETE route handler.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path` | `any` |  |  |
| `handler` | `any` |  |  |

### `.all_methods`

```kiwi
all_methods(path, handler)
```

Registers a handler for all HTTP methods.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path` | `any` |  |  |
| `handler` | `any` |  |  |

### `.use`

```kiwi
use(mw)
```

Adds a middleware function to the pipeline.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `mw` | `lambda` |  | fn(req, res, next) middleware.  Call next() to continue. |

**Returns** `Server` — self

### `.static_files`

```kiwi
static_files(url_prefix, dir)
```

Registers middleware to serve static files from a directory.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `url_prefix` | `string` |  | URL path prefix (e.g. "/public"). |
| `dir` | `string` |  | Filesystem directory to serve from. |

**Returns** `Server` — self

### `.on_error`

```kiwi
on_error(hook)
```

Registers an error handler: fn(err, req, res).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `hook` | `lambda` |  | Error handler lambda. |

**Returns** `Server` — self

### `.on_start`

```kiwi
on_start(hook)
```

Registers a startup hook: fn() called after server starts listening.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `hook` | `lambda` |  | Startup hook lambda. |

**Returns** `Server` — self

### `.on_stop`

```kiwi
on_stop(hook)
```

Registers a shutdown hook: fn() called before server stops.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `hook` | `lambda` |  | Shutdown hook lambda. |

**Returns** `Server` — self

### `.group`

```kiwi
group(prefix)
```

Creates a sub-router for grouping routes under a common prefix.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `prefix` | `string` |  | URL path prefix. |

**Returns** Router

### `.start`

```kiwi
start(block = false)
```

Starts the server.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `block` | `boolean` | `false` | If true, block the caller (serve in the calling thread). |

**Returns** `Server` — self

### `.stop`

```kiwi
stop()
```

Stops the server.

**Returns** `Server` — self

### `.id`

```kiwi
id()
```

### `.running`

```kiwi
running()
```

### `.cfg`

```kiwi
cfg()
```

### `.routes`

```kiwi
routes()
```

### `.middlewares`

```kiwi
middlewares()
```

### `.error_hook`

```kiwi
error_hook()
```
