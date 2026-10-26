# web

> The interface between web applications and the server, with a builder, middleware, streaming and a test client.

The `web` package defines how a Kiwi web application talks to the server that runs it, in the spirit of Ruby's Rack. Because every app follows the same small contract, apps, middleware and servers can be combined freely: a router from one place, a logger from another, mounted side by side under different paths.

The contract has three parts:

- An **app** is anything you can call with an environment hashmap: a lambda, a named function, or an instance of a struct that defines `call` (see [callable instances](../language/structs.md#callable-instances)).
- The app returns a **response**: a list `[status, headers, body]`.
- **Middleware** is a function that takes an app and returns a new app that wraps it.

```kiwi
app = with (env) do
  [200, {"content-type": "text/plain"}, "Hello from ${env["path_info"]}"]
end

r = web::request(app, "GET", "/kiwi")   # call the app directly, no network
println r.status()                       # 200
println r.body()                         # Hello from /kiwi
```

`web::request` is the test client: it builds an environment, calls the app and collects the response, so apps can be tested without a server. To serve the same app over HTTP, pass it to `web::serve(app, {"port": 8080})`; see [Serving](#serving).

## The environment

Each request gets its own hashmap:

| Key | Value |
| :-- | :-- |
| `method` | The HTTP method, uppercase. |
| `script_name` | The path the app is mounted at, or `""` at the root. |
| `path_info` | The rest of the path, percent-decoded, starting with `/` (or `""` when the request is for the mount point itself). |
| `query_string` | The raw query string, without `?`. |
| `headers` | Request headers, with lowercase names. |
| `body` | The request body, as bytes. |
| `remote_addr`, `scheme`, `protocol` | The client's address, `"http"` or `"https"`, and the protocol, such as `"HTTP/1.1"`. |
| `server_name`, `server_port` | Where the server listens. |

Middleware may add its own keys, prefixed with `web.` by convention: `web::request_id()` adds `web.request_id`. `web::Request` wraps an environment with parsing helpers, and caches what it parses in the environment so every layer shares it:

```kiwi
app = with (env) do
  req = web::Request.new(env)
  web::json_response({
    "path": req.path(),
    "q": req.query_param("q"),
    "tags": req.query_all("tag"),
    "theme": req.cookie("theme"),
    "body": req.json()
  })
end

r = web::request(app, "POST", "/search?q=kiwi&tag=a&tag=b", {
  "headers": {"Cookie": "theme=dark"},
  "body": {"page": 2}
})
println r.json()   # {"path": "/search", "q": "kiwi", "tags": ["a", "b"], "theme": "dark", "body": {"page": 2}}
```

## Responses

A response is `[status, headers, body]`. Header names are lowercase. A header value is a string, or a list of strings to send the header more than once (as `set-cookie` needs). The body can be:

| Body | Sent as |
| :-- | :-- |
| a string or bytes | the whole body |
| a list of strings and bytes | the pieces joined |
| a generator | each yielded chunk, as it is produced |
| a lambda | whatever it passes to the `write` function it receives |
| `null` | an empty body |

`text_response`, `html_response`, `json_response`, `redirect` and `status_response` build common responses; `cookie_header` and `add_header` help with headers.

```kiwi
println web::json_response({"ok": true}, 201)   # [201, {"content-type": "application/json"}, "{"ok": true}"]
println web::redirect("/login")                  # [302, {"location": "/login"}, ""]
println web::status_response(404)                # [404, {"content-type": "text/plain; charset=utf-8"}, "Not Found"]
```

### Streaming

A generator or lambda body is streamed: the server sends the headers at once, then each chunk as soon as it is produced. This suits large downloads and server-sent events. A lambda body receives `write`, which returns `false` once the client has gone away, so the producer knows to stop:

```kiwi
fn countdown(n)
  while n > 0 do
    yield "data: ${n}\n\n"
    n -= 1
  end
end

events = with (env) do
  [200, {"content-type": "text/event-stream"}, countdown(3)]
end

ticker = with (env) do
  [200, {"content-type": "text/plain"}, with (write) do
    i = 0
    while i < 3 && write("tick ${i}\n") do
      i += 1
    end
  end]
end

println web::request(events, "GET", "/").body().size()   # 27
println web::request(ticker, "GET", "/").body()          # tick 0, tick 1 and tick 2, one per line
```

If the app throws while streaming, the server resets the connection, so the client sees an incomplete response rather than a complete one with missing data. A streamed response holds one of the server's [workers](#serving) until it finishes, so raise `workers` for long-lived streams.

## Composing apps

`web::builder()` composes middleware and apps. `use` adds a middleware, `run` sets the app at the end of the chain, and `map` mounts an app under a path prefix. Middleware runs in the order it was added, so the first `use` sees each request first. A builder is itself an app.

```kiwi
api = web::builder()
  .use(web::request_id())
  .run(with (env) do
    web::json_response({"mounted_at": env["script_name"], "path": env["path_info"]})
  end)

app = web::builder()
  .use(web::cors())
  .map("/api", api)
  .run(with (env) do web::text_response("home") end)

println web::request(app, "GET", "/api/users").json()   # {"mounted_at": "/api", "path": "/users"}
println web::request(app, "GET", "/").body()            # home
```

A mounted app sees its prefix moved from `path_info` to `script_name`, so it can be written as if it were at the root. The longest matching prefix wins, and prefixes only match whole path segments: `/api` doesn't match `/apix`. Without `run`, a request that no mapping claims gets `404`.

An [`httpserver`](httpserver.md) server is an app too: `server.to_app()` returns it, ready to mount.

### Writing middleware

Middleware takes the next app and returns a new one. It can change the environment on the way in, the response on the way out, or answer without calling the app at all:

```kiwi
fn require_token(token)
  with (app) do
    with (env) do
      return web::status_response(401) when env["headers"].get("authorization") != "Bearer ${token}"
      response = app(env)
      response[1]["x-checked"] = "yes"
      response
    end
  end
end

app = web::builder()
  .use(require_token("s3cret"))
  .run(with (env) do web::text_response("secret stuff") end)

println web::request(app, "GET", "/").status()                                              # 401
println web::request(app, "GET", "/", {"headers": {"Authorization": "Bearer s3cret"}}).body()   # secret stuff
```

### Stock middleware

| Middleware | Does |
| :-- | :-- |
| `logger(format)` | Logs each request to stderr: `"combined"` or `"simple"`. |
| `static_files(prefix, dir, options)` | Serves files for `GET` and `HEAD` under a prefix; `index` and `cache_control` options. Requests that would leave the directory are passed on. |
| `cors(origin, options)` | Adds CORS headers and answers preflight requests with `204`. |
| `request_id(header)` | Gives each request an id in `env["web.request_id"]` and a response header, keeping a well-formed one from the client. |
| `rescue(handler)` | Turns an error thrown by the app into a response: `handler(err, env)`. |
| `lint()` | Checks requests and responses against the contract and throws a descriptive error. Use it in development and tests. |

```kiwi
app = web::builder()
  .use(web::lint())
  .run(with (env) do [200, {"Content-Type": "text/plain"}, "hi"] end)

try
  web::request(app, "GET", "/")
catch e
  println e["message"]   # web::lint: header names must be lowercase, got "Content-Type"
end
```

The server itself checks every response with `web::check_response` and answers a malformed one with `500`, logging the reason, so a mistake never leaves a client waiting.

## Serving

`web::serve(app, options)` runs an app on an HTTP server and blocks until it stops. `web::Server.new(app, options).start()` serves on a background task instead, and `stop()` ends it. The server is [Kestrel](https://learn.microsoft.com/aspnet/core/fundamentals/servers/kestrel); each request runs on its own task with its own environment.

```kiwi
app = web::builder()
  .use(web::logger("simple"))
  .run(with (env) do web::text_response("Hello over HTTP") end)

server = web::Server.new(app, {"port": 18091}).start()
task::sleep(200)

println http::get("http://localhost:18091/").body   # Hello over HTTP
server.stop()
```

| Option | Default | Meaning |
| :-- | :-- | :-- |
| `host` | `"localhost"` | The address to bind: `localhost`, an IP address, or `*` for every interface. |
| `port` | `8080` | The port. |
| `workers` | `64` | Requests whose app is running at once. More wait up to five seconds for a slot, then get `503`. |
| `max_body_size` | 8 MiB | The largest request body; larger ones get `413`. |
| `request_timeout` | `30000` | Milliseconds the app has to return a response; after that the client gets `504`. Streaming bodies can then run as long as they need. |

The server also answers malformed requests with `400`, clients that take more than ten seconds to send their headers with `408`, and requests still waiting when `stop()` is called with `503`. The server is unavailable in [safe mode](../tools/cli.md#safe-mode); the test client and everything else in the package still work.

Requests run in parallel, so state shared between them (a hashmap of sessions, a counter) needs care: each hashmap or list operation is safe on its own, but a read followed by a write, such as `hits += 1`, can lose updates. Guard such updates with a [`task::Mutex`](task.md).

## Constants

| Name | Value | Description |
| :-- | :-- | :-- |
| `web::VERSION` | `"1.0"` |  |

## Functions

### `web::mime_type`

```kiwi
web::mime_type(ext)
```

Returns the MIME type for a file extension, with or without its leading dot.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `ext` | `string` |  | The extension, such as `"png"` or `".png"`. |

**Returns** `string` — The MIME type, or `"application/octet-stream"` when unknown.

### `web::status_text`

```kiwi
web::status_text(code)
```

Returns the reason phrase for an HTTP status code.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `code` | `integer` |  | The status code. |

**Returns** `string` — The phrase, such as `"Not Found"`, or `""` when unknown.

### `web::parse_query`

```kiwi
web::parse_query(text)
```

Parses `application/x-www-form-urlencoded` text, as found in query strings and form bodies. A `+` decodes to a space. When a key repeats, the last value wins.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `text` | `string` |  | The encoded text, without a leading `?`. |

**Returns** `hashmap` — The decoded keys and values.

### `web::parse_query_pairs`

```kiwi
web::parse_query_pairs(text)
```

Parses `application/x-www-form-urlencoded` text into a list of `[key, value]` pairs, keeping every value of a repeated key in order.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `text` | `string` |  | The encoded text, without a leading `?`. |

**Returns** `list` — The decoded pairs.

### `web::parse_cookies`

```kiwi
web::parse_cookies(header)
```

Parses a `Cookie` request header into a hashmap of names to values.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `header` | `string` |  | The header value, such as `"a=1; b=2"`. |

**Returns** `hashmap` — The cookies.

### `web::cookie_header`

```kiwi
web::cookie_header(name, value, options = {})
```

Builds a `Set-Cookie` header value.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | The cookie name. |
| `value` | `string` |  | The cookie value. |
| `options` | `hashmap` | `{}` | Any of `path` (default `"/"`), `max_age` (seconds), `domain`, `http_only` (default `true`), `secure` (default `false`) and `same_site` (default `"Lax"`). |

**Returns** `string` — The header value.

### `web::add_header`

```kiwi
web::add_header(headers, name, value)
```

Adds a header to a response's headers, keeping any value already there: a second value turns the entry into a list, which sends one header line per value (as `set-cookie` needs).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `headers` | `hashmap` |  | The response headers to change. |
| `name` | `string` |  | The header name. Use lowercase. |
| `value` | `string` |  | The value to add. |

**Returns** `hashmap` — `headers`, for chaining.

### `web::text_response`

```kiwi
web::text_response(body, status = 200, headers = {})
```

Builds a plain-text response.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `body` | `string` |  | The text. |
| `status` | `integer` | `200` | The status code (default `200`). |
| `headers` | `hashmap` | `{}` | Extra headers (default `{}`). |

**Returns** `list` — `[status, headers, body]`.

### `web::html_response`

```kiwi
web::html_response(body, status = 200, headers = {})
```

Builds an HTML response.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `body` | `string` |  | The HTML. |
| `status` | `integer` | `200` | The status code (default `200`). |
| `headers` | `hashmap` | `{}` | Extra headers (default `{}`). |

**Returns** `list` — `[status, headers, body]`.

### `web::json_response`

```kiwi
web::json_response(value, status = 200, headers = {})
```

Builds a JSON response.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `value` | `any` |  | The value to serialize. |
| `status` | `integer` | `200` | The status code (default `200`). |
| `headers` | `hashmap` | `{}` | Extra headers (default `{}`). |

**Returns** `list` — `[status, headers, body]`.

### `web::redirect`

```kiwi
web::redirect(location, status = 302)
```

Builds a redirect response.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `location` | `string` |  | Where to send the client. |
| `status` | `integer` | `302` | The status code (default `302`; use `301`, `303`, `307` or `308` as needed). |

**Returns** `list` — `[status, headers, body]`.

### `web::status_response`

```kiwi
web::status_response(status, message = null)
```

Builds a plain-text error response whose body is the status's reason phrase.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `status` | `integer` |  | The status code. |
| `message` | `string` | `null` | The body (default: the reason phrase, such as `"Not Found"`). |

**Returns** `list` — `[status, headers, body]`.

### `web::builder`

```kiwi
web::builder()
```

Creates a `Builder`.

**Returns** Builder

### `web::logger`

```kiwi
web::logger(format = "combined")
```

Logs each request to stderr after it is answered.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `format` | `string` | `"combined"` | `"combined"` (client, time, request line, status, duration) or `"simple"` (time, request line, status, duration). |

**Returns** `lambda` — The middleware.

### `web::static_files`

```kiwi
web::static_files(prefix, dir, options = {})
```

Serves files from a directory for GET and HEAD requests under a path prefix, passing everything else through. Requests that would leave the directory are passed through as well.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `prefix` | `string` |  | The URL prefix, such as `"/assets"`. |
| `dir` | `string` |  | The directory to serve. |
| `options` | `hashmap` | `{}` | `index` names a file to serve for a directory (such as `"index.html"`); `cache_control` sets that header on every file served. |

**Returns** `lambda` — The middleware.

### `web::cors`

```kiwi
web::cors(origin = "*", options = {})
```

Adds CORS headers to every response and answers preflight (`OPTIONS`) requests with `204`.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `origin` | `string` | `"*"` | The allowed origin (default `"*"`). |
| `options` | `hashmap` | `{}` | `methods` and `headers` override the allowed methods and request headers; `max_age` sets how long a preflight is cached, in seconds. |

**Returns** `lambda` — The middleware.

### `web::request_id`

```kiwi
web::request_id(header = "x-request-id")
```

Gives each request an identifier, stored as `env["web.request_id"]` and sent back in a response header. A well-formed identifier sent by the client (or a proxy) is kept.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `header` | `string` | `"x-request-id"` | The header to read and write (default `"x-request-id"`). |

**Returns** `lambda` — The middleware.

### `web::rescue`

```kiwi
web::rescue(handler)
```

Catches errors thrown by the app and turns them into a response. Errors thrown while a streaming body is producing chunks happen after the response has started, so they are not caught here.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `handler` | `lambda` |  | Called as `handler(err, env)`; returns a response. |

**Returns** `lambda` — The middleware.

### `web::lint`

```kiwi
web::lint()
```

Checks that requests and responses follow the web interface, and throws a descriptive error when they don't. Use it in development and tests, between the app and everything else.

**Returns** `lambda` — The middleware.

### `web::check_response`

```kiwi
web::check_response(response)
```

Checks a value against the response format: `[status, headers, body]` with an integer status from 100 to 999, a hashmap of lowercase header names to strings (or lists of strings), and a body that is a string, bytes, a list of strings or bytes, a generator, or a lambda. Status 1xx, 204 and 304 responses must have an empty body.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `response` | `any` |  | The value an app returned. |

**Returns** `string` — A description of the first problem, or `null` when it is valid.

### `web::serve`

```kiwi
web::serve(app, options = {})
```

Runs an app on an HTTP server and blocks until it stops.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `app` | `any` |  | The app. |
| `options` | `hashmap` | `{}` | The server options; see `Server.new`. |

**Returns** `Server` — The stopped server.

### `web::mock_env`

```kiwi
web::mock_env(method, path, options = {})
```

Builds an environment for calling an app directly, without a server.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `method` | `string` |  | The HTTP method. |
| `path` | `string` |  | The path, optionally with a query string (`"/search?q=kiwi"`). |
| `options` | `hashmap` | `{}` | `headers` (a hashmap), `body` (a string, bytes, or a hashmap sent as JSON), `remote_addr` and `script_name`. |

**Returns** `hashmap` — The environment.

### `web::request`

```kiwi
web::request(app, method, path, options = {})
```

Calls an app with a mock request and collects its response, streaming bodies included. Errors thrown by the app propagate, so tests see them.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `app` | `any` |  | The app. |
| `method` | `string` |  | The HTTP method. |
| `path` | `string` |  | The path, optionally with a query string. |
| `options` | `hashmap` | `{}` | As for `mock_env`. |

**Returns** `MockResponse` — The response.

## `Request` struct

A read-only view of an environment with parsing helpers. Parsed values are cached in the environment, so every `Request` made from the same environment shares them.

### `Request.new`

```kiwi
Request.new(env)
```

Wraps an environment.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `env` | `hashmap` |  | The environment passed to the app. |

### `.env`

```kiwi
env()
```

Returns the environment.

**Returns** hashmap

### `.method`

```kiwi
method()
```

Returns the HTTP method, uppercase.

**Returns** string

### `.path`

```kiwi
path()
```

Returns the full path: `script_name` followed by `path_info`.

**Returns** string

### `.path_info`

```kiwi
path_info()
```

Returns the part of the path after the point where the app is mounted.

**Returns** string

### `.script_name`

```kiwi
script_name()
```

Returns the path where the app is mounted, or `""` at the root.

**Returns** string

### `.query_string`

```kiwi
query_string()
```

Returns the raw query string, without the `?`.

**Returns** string

### `.query`

```kiwi
query()
```

Returns the query string parsed into a hashmap. When a key repeats, the last value wins; use `query_all` for every value.

**Returns** hashmap

### `.query_param`

```kiwi
query_param(name, fallback = null)
```

Returns one query parameter.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | The parameter name. |
| `fallback` | `any` | `null` | The value when it is absent (default `null`). |

**Returns** any

### `.query_all`

```kiwi
query_all(name)
```

Returns every value of a query parameter, in order.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | The parameter name. |

**Returns** list

### `.headers`

```kiwi
headers()
```

Returns the request headers. Names are lowercase.

**Returns** hashmap

### `.header`

```kiwi
header(name)
```

Returns a request header, or `null`. The name is case-insensitive.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | The header name. |

**Returns** string

### `.content_type`

```kiwi
content_type()
```

Returns the `Content-Type` header, or `""`.

**Returns** string

### `.is_json`

```kiwi
is_json()
```

Checks whether the body is JSON.

**Returns** boolean

### `.is_form`

```kiwi
is_form()
```

Checks whether the body is a URL-encoded form.

**Returns** boolean

### `.body_bytes`

```kiwi
body_bytes()
```

Returns the body as bytes.

**Returns** bytes

### `.body`

```kiwi
body()
```

Returns the body decoded as UTF-8 text.

**Returns** string

### `.json`

```kiwi
json()
```

Returns the body parsed as JSON, or `null` when it is empty.

**Returns** any

### `.form`

```kiwi
form()
```

Returns the fields of a URL-encoded form body.

**Returns** hashmap

### `.cookies`

```kiwi
cookies()
```

Returns the request cookies.

**Returns** hashmap

### `.cookie`

```kiwi
cookie(name)
```

Returns a cookie's value, or `null`.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | The cookie name. |

**Returns** string

### `.remote_addr`

```kiwi
remote_addr()
```

Returns the client's IP address.

**Returns** string

### `.scheme`

```kiwi
scheme()
```

Returns `"http"` or `"https"`.

**Returns** string

### `.host`

```kiwi
host()
```

Returns the host the client asked for: the `Host` header, or the server name and port.

**Returns** string

### `.url`

```kiwi
url()
```

Returns the full URL of the request.

**Returns** string

### `.get`

```kiwi
get(key, fallback = null)
```

Returns any environment value, such as one a middleware added.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `key` | `string` |  | The key, such as `"web.request_id"`. |
| `fallback` | `any` | `null` | The value when it is absent (default `null`). |

**Returns** any

## `Builder` struct

Composes middleware and apps into one app. Middleware runs in the order it was added, so the first `use` sees each request first. A builder is itself an app.

### `Builder.new`

```kiwi
Builder.new()
```

### `.use`

```kiwi
use(middleware)
```

Adds a middleware: a function that takes the next app and returns a new app.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `middleware` | `lambda` |  | The middleware. |

**Returns** `Builder` — self

### `.run`

```kiwi
run(app)
```

Sets the app that handles requests no mapping claims.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `app` | `any` |  | The app. |

**Returns** `Builder` — self

### `.map`

```kiwi
map(prefix, app)
```

Mounts an app under a path prefix. The mounted app sees the prefix moved from `path_info` to `script_name`. The longest matching prefix wins, and a prefix only matches whole path segments.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `prefix` | `string` |  | The prefix, such as `"/api"`. |
| `app` | `any` |  | The app to mount (another builder works too). |

**Returns** `Builder` — self

### `.to_app`

```kiwi
to_app()
```

Builds the composed app. Calling the builder directly does this once, on the first request.

**Returns** `lambda` — The app.

### `.call`

```kiwi
call(env)
```

Handles a request, which makes a builder usable as an app.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `env` | `hashmap` |  | The environment. |

**Returns** `list` — `[status, headers, body]`.

## `Server` struct

Runs an app on an HTTP server (Kestrel). Each request gets its own task and its own environment.

### `Server.new`

```kiwi
Server.new(app, options = {})
```

Creates a server for an app. Nothing listens until `start`.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `app` | `any` |  | The app. |
| `options` | `hashmap` | `{}` | `host` (default `"localhost"`), `port` (default `8080`), `workers` (handlers running at once, default `64`), `max_body_size` (bytes, default 8 MiB) and `request_timeout` (milliseconds to start a response, default `30000`). |

### `.start`

```kiwi
start(block = false)
```

Starts listening.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `block` | `boolean` | `false` | If `true`, serves on the calling task until `stop()`; otherwise serves on a background task and returns at once. |

**Returns** `Server` — self

### `.stop`

```kiwi
stop()
```

Stops listening. Requests still waiting on the app are answered with `503`.

**Returns** `Server` — self

### `.running`

```kiwi
running()
```

Checks whether the server is listening.

**Returns** boolean

### `.id`

```kiwi
id()
```

Returns the server's internal identifier.

**Returns** integer

### `.poll_ms`

```kiwi
poll_ms()
```

Returns the accept-poll interval in milliseconds.

**Returns** integer

## `MockResponse` struct

The response collected by `web::request`.

### `MockResponse.new`

```kiwi
MockResponse.new(status, headers, body)
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `status` | `any` |  |  |
| `headers` | `any` |  |  |
| `body` | `any` |  |  |

### `.status`

```kiwi
status()
```

Returns the status code.

**Returns** integer

### `.headers`

```kiwi
headers()
```

Returns the response headers.

**Returns** hashmap

### `.header`

```kiwi
header(name)
```

Returns a response header (case-insensitive), or `null`. A header sent more than once is a list.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | The header name. |

**Returns** any

### `.body`

```kiwi
body()
```

Returns the body as text.

**Returns** string

### `.body_bytes`

```kiwi
body_bytes()
```

Returns the body as bytes.

**Returns** bytes

### `.json`

```kiwi
json()
```

Returns the body parsed as JSON.

**Returns** any
