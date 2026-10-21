---
title: httpserver
description: An HTTP server with routing, middleware, route groups, static files, cookies and CORS.
---

The `httpserver` package runs an HTTP server. Register handlers for routes, add middleware, and start it. Handlers receive a `Request` and a `Response`.

```kiwi stderr
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

The server logs each request to stderr; set `.logging("none")` in the [configuration](#configuration) to turn that off. httpserver is unavailable in [safe mode](/kiwi/tools/cli/#safe-mode).

## Routes

`get`, `post`, `put`, `patch` and `delete` register a handler for a method and path; `all_methods` matches any method. Each returns the server, so registrations can be chained. Paths can contain:

- literal segments: `/users/profile`
- named parameters: `/users/:id`, read with `req.param("id")`
- a trailing wildcard: `/files/*`, read with `req.param("*")`

`group(prefix)` returns a router whose routes all start with `prefix`:

```kiwi nocheck
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

```kiwi nocheck
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

```kiwi nocheck
server.static_files("/public", "./public")   # serves ./public/* at /public/*

server.get("/", with (req, res) do
  res.file("./public/index.html")
end)
```

## Configuration

`httpserver::config()` builds a configuration fluently; pass it to `create_with_config`:

```kiwi nocheck
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

```kiwi nocheck
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

To stop cleanly on <kbd>Ctrl</kbd>+<kbd>C</kbd>, combine `start(true)` with a [`signal`](/kiwi/stdlib/signal/) handler that calls `server.stop()`.

<!-- api: httpserver -->
