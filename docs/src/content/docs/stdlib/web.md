---
title: web
description: The interface between web applications and the server, with a builder, middleware, streaming and a test client.
---

The `web` package defines how a Kiwi web application talks to the server that runs it, in the spirit of Ruby's Rack. Because every app follows the same small contract, apps, middleware and servers can be combined freely: a router from one place, a logger from another, mounted side by side under different paths.

The contract has three parts:

- An **app** is anything you can call with an environment hashmap: a lambda, a named function, or an instance of a struct that defines `call` (see [callable instances](/kiwi/language/structs/#callable-instances)).
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

An [`httpserver`](/kiwi/stdlib/httpserver/) server is an app too: `server.to_app()` returns it, ready to mount.

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

```kiwi stderr
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

The server also answers malformed requests with `400`, clients that take more than ten seconds to send their headers with `408`, and requests still waiting when `stop()` is called with `503`. The server is unavailable in [safe mode](/kiwi/tools/cli/#safe-mode); the test client and everything else in the package still work.

Requests run in parallel, so state shared between them (a hashmap of sessions, a counter) needs care: each hashmap or list operation is safe on its own, but a read followed by a write, such as `hits += 1`, can lose updates. Guard such updates with a [`task::Mutex`](/kiwi/stdlib/task/).

<!-- api: web -->
