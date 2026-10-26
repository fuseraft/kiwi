---
title: httpserver
description: An HTTP server with routing, middleware, route groups, static files, cookies and CORS.
---

The `httpserver` package runs an HTTP server. Register handlers for routes, add middleware, and start it. Handlers receive a `Request` and a `Response`. A server is built on the [`web`](/kiwi/stdlib/web/) interface, so it can be mounted beside other web apps, wrapped in web middleware and tested without a network; see [Working with `web`](#working-with-web).

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
| `env()` | The request's [`web` environment](/kiwi/stdlib/web/#the-environment) |

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

```kiwi stderr
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

A server is a [`web`](/kiwi/stdlib/web/) app. `to_app()` returns it with its logging, CORS and wrappers applied, `wrap` adds web middleware around the whole server, and `web::request` calls it without starting a listener, which makes handlers quick to test:

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
