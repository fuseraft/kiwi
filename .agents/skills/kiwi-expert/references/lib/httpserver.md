# `httpserver`

The `httpserver` package provides a production-grade HTTP server with routing, middleware pipelines, static file serving, cookies, CORS, and Tomcat-style configuration.

---

## Quick Start

```kiwi
import "httpserver"

server = httpserver::create(8080)

server.get("/", with (req, res) do
  res.text("Hello, World!")
end)

server.get("/greet/:name", with (req, res) do
  res.json({ "message": "Hello, " + req.param("name") + "!" })
end)

server.start()
```

---

## Factory Functions

### `create(port?, host?)`

Creates a new `Server` with default configuration.

**Parameters**

| Type      | Name   | Description                       | Default       |
|-----------|--------|-----------------------------------|---------------|
| `integer` | `port` | Port to listen on                 | `8080`        |
| `string`  | `host` | Bind address                      | `"localhost"` |

**Returns** `Server`

---

### `create_with_config(cfg)`

Creates a new `Server` from a `Config` object.

**Parameters**

| Type     | Name  | Description              |
|----------|-------|--------------------------|
| `Config` | `cfg` | Server configuration     |

**Returns** `Server`

---

### `config()`

Returns a new `Config` builder for fluent configuration.

**Returns** `Config`

**Example**
```kiwi
import "httpserver"

cfg = httpserver::config()
      .host("0.0.0.0")
      .port(9000)
      .workers(20)
      .logging("combined")
      .cors()

server = httpserver::create_with_config(cfg)
```

---

## `Config`

A fluent configuration builder. All setter methods return `self` for chaining.

### `Config.new()`

Creates a `Config` with defaults: `host="localhost"`, `port=8080`, `workers=10`, `timeout_ms=5000`, `log="combined"`, CORS disabled.

---

### Setters

| Method                  | Parameter Type    | Description                                              | Default      |
|-------------------------|-------------------|----------------------------------------------------------|--------------|
| `.host(v)`              | `string`          | Hostname or IP address to bind                           | `"localhost"`|
| `.port(v)`              | `integer`         | Port number                                              | `8080`       |
| `.workers(v)`           | `integer`         | Maximum concurrent request worker tasks                  | `10`         |
| `.timeout(v)`           | `integer`         | Accept-poll timeout in milliseconds                      | `5000`       |
| `.logging(v)`           | `string\|boolean` | `"combined"`, `"simple"`, `"none"`, or `false` to disable | `"combined"` |
| `.cors(origin?)`        | `string`          | Enables CORS; sets `Access-Control-Allow-Origin`         | `"*"`        |

---

## `Server`

The main server object. Returned by `httpserver::create()` or `httpserver::create_with_config()`.

### Route Registration

All route methods return `self` for chaining.

| Method                          | Description                                    |
|---------------------------------|------------------------------------------------|
| `.get(path, handler)`           | Register a `GET` route                         |
| `.post(path, handler)`          | Register a `POST` route                        |
| `.put(path, handler)`           | Register a `PUT` route                         |
| `.patch(path, handler)`         | Register a `PATCH` route                       |
| `.delete(path, handler)`        | Register a `DELETE` route                      |
| `.all_methods(path, handler)`   | Register a route matching any HTTP method      |

**`handler`** is a lambda with signature `with (req, res) do ... end`.

**Route patterns** support:
- Literal segments: `/users/profile`
- Named parameters: `/users/:id` — accessible via `req.param("id")`
- Wildcard suffix: `/static/*` — captures remaining path in `req.param("*")`

---

### `.use(mw)`

Adds a middleware function to the request pipeline. Middlewares run in registration order before the matched route handler.

**Parameters**

| Type     | Name | Description                                                        |
|----------|------|--------------------------------------------------------------------|
| `lambda` | `mw` | Middleware with signature `with (req, res, nxt) do ... nxt() end` |

**Returns** `Server`

> Call `nxt()` to pass control to the next middleware. Omit it to stop the chain (short-circuit).

---

### `.static_files(url_prefix, dir)`

Registers middleware to serve static files from a filesystem directory.

**Parameters**

| Type     | Name         | Description                                              |
|----------|--------------|----------------------------------------------------------|
| `string` | `url_prefix` | URL path prefix (e.g. `"/public"`)                       |
| `string` | `dir`        | Filesystem directory to serve from (e.g. `"./public"`)   |

**Returns** `Server`

MIME types are inferred from file extensions. Responds with `404` for missing files.

---

### `.group(prefix)`

Creates a `Router` that prepends `prefix` to all registered routes.

**Parameters**

| Type     | Name     | Description                    |
|----------|----------|--------------------------------|
| `string` | `prefix` | URL path prefix (e.g. `"/api"`) |

**Returns** `Router`

---

### Lifecycle Hooks

| Method             | Handler Signature          | Description                                    |
|--------------------|----------------------------|------------------------------------------------|
| `.on_start(hook)`  | `with () do ... end`       | Called after the server starts listening       |
| `.on_stop(hook)`   | `with () do ... end`       | Called before the server stops                 |
| `.on_error(hook)`  | `with (err, req, res) do`  | Called when a route handler throws an error    |

All hook methods return `self`.

---

### `.start(block?)`

Starts the server.

**Parameters**

| Type      | Name    | Description                                                 | Default |
|-----------|---------|-------------------------------------------------------------|---------|
| `boolean` | `block` | If `true`, blocks the calling thread until `stop()` is called | `false` |

**Returns** `Server`

When `block=false` (default), the accept loop runs in a background task.

---

### `.stop()`

Stops the server, calling the `on_stop` hook if registered.

**Returns** `Server`

---

## `Router`

A route group that delegates to a `Server` with a common path prefix. Returned by `server.group(prefix)`.

### Methods

| Method                        | Description                                           |
|-------------------------------|-------------------------------------------------------|
| `.get(path, handler)`         | Registers `GET prefix+path`                           |
| `.post(path, handler)`        | Registers `POST prefix+path`                          |
| `.put(path, handler)`         | Registers `PUT prefix+path`                           |
| `.patch(path, handler)`       | Registers `PATCH prefix+path`                         |
| `.delete(path, handler)`      | Registers `DELETE prefix+path`                        |
| `.all_methods(path, handler)` | Registers for all methods under `prefix+path`         |
| `.group(sub_prefix)`          | Creates a nested `Router` with combined prefix        |

---

## `Request`

Passed as the first argument to every route handler and middleware.

### Methods

| Method                        | Returns           | Description                                                        |
|-------------------------------|-------------------|--------------------------------------------------------------------|
| `.method()`                   | `string`          | HTTP method in uppercase (`"GET"`, `"POST"`, …)                    |
| `.path()`                     | `string`          | URL path (e.g. `"/users/42"`)                                      |
| `.query()`                    | `hashmap`         | All query-string parameters                                        |
| `.query_param(name, default?)` | `string\|any`    | Single query param; returns `default` if absent (`null`)           |
| `.headers()`                  | `hashmap`         | All request headers (keys are lowercased)                          |
| `.header(name)`               | `string\|null`    | Single header by name (case-insensitive)                           |
| `.body()`                     | `string`          | Raw request body                                                   |
| `.remote_addr()`              | `string`          | Client IP address                                                  |
| `.id()`                       | `integer`         | Internal request ID                                                |
| `.param(name)`                | `string\|null`    | Route parameter by name (e.g. `"id"` for `/:id`)                  |
| `.params()`                   | `hashmap`         | All route parameters                                               |
| `.content_type()`             | `string\|null`    | Value of the `Content-Type` header                                 |
| `.is_json()`                  | `boolean`         | `true` if `Content-Type` starts with `application/json`            |
| `.is_form()`                  | `boolean`         | `true` if `Content-Type` is `application/x-www-form-urlencoded`   |
| `.json()`                     | `any`             | Parses and returns the body as JSON (cached)                       |
| `.form()`                     | `hashmap`         | Parses URL-encoded form body (cached)                              |
| `.cookie(name)`               | `string\|null`    | Returns a cookie value by name                                     |
| `.cookies()`                  | `hashmap`         | All cookies parsed from the `Cookie` header (cached)               |

---

## `Response`

Passed as the second argument to every route handler and middleware.

### Sending Responses

| Method                             | Description                                                    |
|------------------------------------|----------------------------------------------------------------|
| `.send(status, body)`              | Send with explicit status code; `body` is a string or bytes    |
| `.text(body, status?)`             | Send `text/plain` (default 200)                                |
| `.html(body, status?)`             | Send `text/html`                                               |
| `.json(body, status?)`             | Send `application/json`; auto-serializes non-string values     |
| `.file(fs_path, status?)`          | Send file contents; MIME inferred from extension               |
| `.redirect(location, status?)`     | Send redirect (default 302)                                    |

### Convenience Error Responses

| Method                     | Status | Default body               |
|----------------------------|--------|----------------------------|
| `.not_found(msg?)`         | 404    | `"Not Found"`              |
| `.bad_request(msg?)`       | 400    | `"Bad Request"`            |
| `.unauthorized(msg?)`      | 401    | `"Unauthorized"`           |
| `.forbidden(msg?)`         | 403    | `"Forbidden"`              |
| `.server_error(msg?)`      | 500    | `"Internal Server Error"`  |

### Headers and Cookies

| Method                                                      | Description                                                  |
|-------------------------------------------------------------|--------------------------------------------------------------|
| `.header(key, value)`                                       | Set a response header; returns `self`                        |
| `.cookie(name, value, path?, max_age?, http_only?, secure?, same_site?)` | Append a `Set-Cookie` header; returns `self` |
| `.clear_cookie(name, path?)`                                | Expire a cookie immediately; returns `self`                  |
| `.no_cache()`                                               | Set `Cache-Control: no-store` headers; returns `self`        |

**`cookie()` defaults:** `path="/"`, `max_age=null` (session), `http_only=true`, `secure=false`, `same_site="Lax"`.

### State

| Method           | Returns   | Description                                  |
|------------------|-----------|----------------------------------------------|
| `.sent()`        | `boolean` | `true` if a response has already been sent   |
| `.status_code()` | `integer` | The status code that was (or will be) sent   |

---

## Examples

### Basic server

```kiwi
import "httpserver"

server = httpserver::create(8080)

server.get("/", with (req, res) do
  res.html("<h1>Hello from Kiwi</h1>")
end)

server.get("/api/greet/:name", with (req, res) do
  name = req.param("name")
  res.json({ "greeting": "Hello, " + name + "!" })
end)

server.post("/api/echo", with (req, res) do
  res.text("You sent: " + req.body())
end)

server.start()
```

---

### Middleware

```kiwi
import "httpserver"
import "time"

server = httpserver::create(8080)

# Timing middleware
server.use(with (req, res, nxt) do
  t0 = time::ticks()
  nxt()
  elapsed = (time::ticks() - t0) * 1000
  eprintln "handled in ${elapsed}ms"
end)

# Auth guard
server.use(with (req, res, nxt) do
  token = req.header("authorization")
  if token == null || token != "Bearer secret"
    res.unauthorized("Invalid token")
    return    # Do NOT call nxt() — short-circuit the chain
  end
  nxt()
end)

server.get("/secure", with (req, res) do
  res.text("You're in!")
end)

server.start()
```

---

### Route groups

```kiwi
import "httpserver"

server = httpserver::create(8080)
api    = server.group("/api/v1")

api.get("/users", with (req, res) do
  res.json([{ "id": 1, "name": "Alice" }])
end)

api.get("/users/:id", with (req, res) do
  res.json({ "id": req.param("id") })
end)

api.post("/users", with (req, res) do
  data = req.json()
  res.json({ "created": data }, 201)
end)

server.start()
```

---

### Static file serving

```kiwi
import "httpserver"

server = httpserver::create(8080)

# Serve ./public/ at /public/*
server.static_files("/public", "./public")

server.get("/", with (req, res) do
  res.file("./public/index.html")
end)

server.start()
```

---

### CORS and configuration

```kiwi
import "httpserver"

cfg = httpserver::config()
      .host("0.0.0.0")
      .port(9000)
      .cors("https://myapp.example.com")
      .logging("simple")

server = httpserver::create_with_config(cfg)

server.get("/api/data", with (req, res) do
  res.json({ "items": [1, 2, 3] })
end)

server.start()
```

---

### Cookies

```kiwi
import "httpserver"

server = httpserver::create(8080)

server.post("/login", with (req, res) do
  body = req.json()
  if body["username"] == "admin" && body["password"] == "secret"
    res.cookie("session", "tok_abc123", "/", 3600)
    res.json({ "ok": true })
  else
    res.unauthorized("Bad credentials")
  end
end)

server.post("/logout", with (req, res) do
  res.clear_cookie("session")
  res.json({ "ok": true })
end)

server.get("/profile", with (req, res) do
  sid = req.cookie("session")
  if sid == null
    res.unauthorized()
    return
  end
  res.json({ "user": "admin" })
end)

server.start()
```

---

### Lifecycle hooks and error handling

```kiwi
import "httpserver"

server = httpserver::create(8080)

server.on_start(with () do
  eprintln "Server is up!"
end)

server.on_stop(with () do
  eprintln "Server is going down..."
end)

server.on_error(with (err, req, res) do
  eprintln "Error on ${req.path()}: ${err}"
  res.json({ "error": err.to_string() }, 500)
end)

server.get("/risky", with (req, res) do
  error::raise("something went wrong")
end)

server.start()
```

---

### Blocking mode (serve from main thread)

```kiwi
import "httpserver"
import "task"

server = httpserver::create(8080)

server.get("/", with (req, res) do
  res.text("Hello!")
end)

# Spawn a background task that stops the server after 10 seconds
task::spawn(with () do
  task::sleep(10000)
  server.stop()
end, [])

# Block the main thread — returns when stop() is called
server.start(true)

println "Server stopped."
```
