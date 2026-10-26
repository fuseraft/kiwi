# httpserver

> An HTTP server with routing, middleware, route groups, static files, cookies and CORS.

The `httpserver` package runs an HTTP server. Register handlers for routes, add middleware, and start it. Handlers receive a `Request` and a `Response`. A server is built on the [`web`](web.md) interface, so it can be mounted beside other web apps, wrapped in web middleware and tested without a network; see [Working with `web`](#working-with-web).

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

Requests that match no route get a `404 Not Found`. A `HEAD` request is answered by the matching `GET` route, without the body.

## Requests

| Method | Returns |
| :-- | :-- |
| `method()`, `path()` | The HTTP method and path |
| `param(name)`, `params()` | Route parameters |
| `query()`, `query_param(name)`, `query_all(name)` | Query string values; when a key repeats, `query` keeps the last value and `query_all` returns every one |
| `header(name)`, `headers()` | Request headers (names are case-insensitive) |
| `body()` | The raw body as a string |
| `body_bytes()` | The raw body as bytes, for binary uploads |
| `json()`, `is_json()` | The body parsed as JSON |
| `form()`, `is_form()` | Form fields from a URL-encoded body |
| `cookie(name)`, `cookies()` | Request cookies |
| `content_type()`, `remote_addr()` | The content type and client address |
| `id()` | The request id from `web::request_id()`, if the server is [wrapped](#working-with-web) with it |
| `env()` | The request's [`web` environment](web.md#the-environment) |

## Responses

| Method | Sends |
| :-- | :-- |
| `text(body, status)` | Plain text |
| `html(body, status)` | HTML |
| `json(value, status)` | JSON |
| `file(path)` | A file, with a content type based on its extension |
| `redirect(url, status)` | A redirect |
| `send(status, body)` | A raw body, as a string or bytes |
| `stream(body, status)` | A body sent in chunks as it is produced; see [Streaming](#streaming) |
| `not_found()`, `bad_request()`, `unauthorized()`, `forbidden()`, `server_error()` | An error status with an optional message |

`header(name, value)`, `cookie(name, value, path, max_age)`, `clear_cookie(name)` and `no_cache()` set response details before sending. Header names are sent lowercase, as HTTP/2 requires and the `web` interface expects; HTTP treats names case-insensitively, so clients are unaffected unless they compare names exactly. Each response is sent once; check `sent()` to see whether a handler already responded, and `status_code()` for the status it sent.

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
  .max_body_size(1024 * 1024)
  .request_timeout(10000)
  .cors("https://myapp.example.com")
  .logging("simple")   # "combined", "simple" or "none"

server = httpserver::create_with_config(cfg)
```

## Limits

The server is built on [Kestrel](https://learn.microsoft.com/aspnet/core/fundamentals/servers/kestrel), the HTTP server behind ASP.NET Core, which parses requests and manages connections and keep-alive. A few limits protect it from slow or oversized requests and from overload, and each answers the client with a status code instead of reaching a handler:

| Limit | Default | Set with | Response |
| :-- | :-- | :-- | :-- |
| Handlers running at once | 64 | `workers(n)` | `503` if no slot frees up within five seconds |
| Request body size | 8 MiB | `max_body_size(bytes)` | `413` |
| Time for a handler to respond | 30 seconds | `request_timeout(ms)` | `504`; the handler's late response is discarded |
| Time for a client to send its headers | 10 seconds | | `408` |
| Malformed request | | | `400` |

`server.stop()` answers requests still waiting on a handler with `503`.

## Streaming

`res.stream(body)` sends the headers at once and then each chunk as the body produces it, which suits large downloads and server-sent events. The body is a generator that yields chunks, or a lambda that receives a `write(chunk)` function. `write` returns `false` once the client has gone away:

```kiwi
server = httpserver::create(18088)

server.get("/events", with (req, res) do
  res.header("content-type", "text/event-stream")
  res.stream(with (write) do
    for i in [1 to 3] do
      break when !write("data: ${i}\n\n")
      task::sleep(100)
    end
  end)
end)

server.start()
task::sleep(200)
println http::get("http://localhost:18088/events").body.size()   # 27
server.stop()
```

A streaming response keeps its worker until it ends, so raise `workers` in the [configuration](#configuration) for many long-lived streams.

## Working with `web`

A server is a [`web`](web.md) app. `to_app()` returns it with its logging, CORS and wrappers applied, `wrap` adds web middleware around the whole server, and `web::request` calls it without starting a listener, which makes handlers quick to test:

```kiwi
server = httpserver::create_with_config(httpserver::config().logging("none"))
server.wrap(web::request_id())
server.get("/users/:id", with (req, res) do
  res.json({"id": req.param("id"), "request": req.id().size()})
end)

r = web::request(server.to_app(), "GET", "/users/7")
println r.json()                 # {"id": "7", "request": 36}

site = web::builder()
  .map("/api", server.to_app())
  .run(with (env) do web::text_response("home") end)
println web::request(site, "GET", "/api/users/8").json()["id"]   # 8
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

Sets how many requests can run handlers at once (default 64). Further requests wait up to five seconds for a free slot, then get `503 Service Unavailable`.

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

### `.max_body_size`

```kiwi
max_body_size(v)
```

Sets the largest request body accepted, in bytes (default 8 MiB). A larger body gets `413 Content Too Large` without reaching a handler.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `v` | `integer` |  | Maximum body size in bytes. |

**Returns** `Config` — self

### `.request_timeout`

```kiwi
request_timeout(v)
```

Sets how long a handler has to respond, in milliseconds (default 30000). Past that, the client gets `504 Gateway Timeout` and the handler's late response is discarded.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `v` | `integer` |  | Milliseconds. |

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

### `.get_max_body_size`

```kiwi
get_max_body_size()
```

### `.get_request_timeout`

```kiwi
get_request_timeout()
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

Represents an incoming HTTP request. It wraps a `web::Request`, adding route parameters.

### `Request.new`

```kiwi
Request.new(env, params)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `env` | `any` |  |  |
| `params` | `any` |  |  |

### `.env`

```kiwi
env()
```

Returns the request's `web` environment.

**Returns** hashmap

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

Returns the URL path, percent-decoded.

**Returns** string

### `.query`

```kiwi
query()
```

Returns the query string parameters as a hashmap. When a key repeats, the last value wins; use `query_all` for every value.

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

### `.query_all`

```kiwi
query_all(name)
```

Returns every value of a query parameter, in order.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | Parameter name. |

**Returns** list

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

### `.body_bytes`

```kiwi
body_bytes()
```

Returns the raw request body as bytes, for binary uploads that `body()` would corrupt by decoding as UTF-8.

**Returns** bytes

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

Returns the request ID set by the `web::request_id()` middleware (see `Server.wrap`), or null.

**Returns** string|null

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

Collects the response a handler sends. Each response is sent once; the first `send` (or helper such as `text`) wins.

### `Response.new`

```kiwi
Response.new()
```

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

Sets a response header. Names are case-insensitive and sent lowercase. A non-string value is sent as its text, and a list sends one header line per element.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `key` | `string` |  | Header name. |
| `value` | `any` |  | Header value. |

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

### `.stream`

```kiwi
stream(body, status = 200)
```

Streams the response body: chunks reach the client as they are produced, so the response can be long-lived (such as server-sent events). The body is a generator that yields chunks, or a lambda that receives a `write(chunk)` function; `write` returns false once the client has gone away.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `body` | `generator\|lambda` |  | The chunk producer. |
| `status` | `integer` | `200` | HTTP status code (default 200). |

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

### `.finish`

```kiwi
finish()
```

Returns the collected response as `[status, headers, body]`.

**Returns** list

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

An HTTP server with routing, middleware, and static serving.

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

### `.wrap`

```kiwi
wrap(middleware)
```

Adds a `web` middleware (a function from app to app, such as `web::request_id()`) around the whole server. Wrappers run before the server's own middleware, in the order added.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `middleware` | `lambda` |  | The `web` middleware. |

**Returns** `Server` — self

### `.to_app`

```kiwi
to_app()
```

Returns the server as a `web` app, with its logging, CORS and wrappers applied, for mounting in a `web::Builder` or testing with `web::request`.

**Returns** `lambda` — The app.

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
