---
title: curl
description: Make HTTP requests by running the system curl command.
---

The `curl` package makes HTTP requests by running the `curl` program, which must be installed and on your `PATH`. It is a simple, synchronous alternative to the [`http`](/kiwi/stdlib/http/) package, convenient for quick scripts.

```kiwi nocheck
body = curl::get("https://httpbin.org/get")
println body

headers = {"Content-Type": "application/json"}
println curl::post("https://httpbin.org/post", headers, '{"lang": "kiwi"}')
println curl::put("https://httpbin.org/put", headers, '{"status": "ok"}')
println curl::delete("https://httpbin.org/delete")
```

Each function returns the response body as a string. Pass `false` as the last argument (`capture_output`) to get `curl`'s exit code instead:

```kiwi nocheck
code = curl::get("https://example.com", {}, false)
println code == 0 ? "reachable" : "failed"
```

Notes:

- Requests run with `curl -s`, so progress output is suppressed.
- A request body is only sent for `POST` and `PUT`.
- For form data, set `Content-Type` to `application/x-www-form-urlencoded` and pass `key=value&key2=value2` as the body.
- For response status codes and headers, or concurrent requests, use [`http`](/kiwi/stdlib/http/).

<!-- api: curl -->
