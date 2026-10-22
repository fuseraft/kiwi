---
title: socket
description: Asynchronous TCP and TLS sockets.
---

The `socket` package provides TCP sockets, and the `tls` package, defined in the same file, provides the same functions over TLS. Sockets are identified by integer ids. Operations that wait on the network (`accept`, `recv`, `send`) start a background [task](/kiwi/stdlib/task/) and return its id; `task::await` waits for the result.

## An echo server and client

```kiwi
server = socket::tcpserver("127.0.0.1", 18093)

worker = task::spawn(do ()
  client = task::await(socket::accept(server))
  data = task::await(socket::recv(client))
  task::await(socket::send(client, ("echo: " + data.to_string()).to_bytes()))
  socket::close(client)
end)

conn = socket::tcpconnect("127.0.0.1", 18093)
task::await(socket::send(conn, "hello".to_bytes()))
println task::await(socket::recv(conn)).to_string()   # echo: hello

socket::close(conn)
task::await(worker)
socket::close(server)
```

| Function | Returns |
| :-- | :-- |
| `tcpserver(host, port, backlog)` | A listening socket id. |
| `tcpconnect(host, port)` | A connected socket id. |
| `accept(server)` | A task that resolves to the next client's socket id. |
| `recv(socket, max_bytes)` | A task that resolves to the bytes received (up to 4096 by default). |
| `send(socket, bytes)` | A task that resolves when the bytes have been sent. |
| `close(socket)` | Closes the socket and fails any pending tasks on it. |

`send` takes bytes; convert text with `.to_bytes()`. `recv` returns bytes; convert with `.to_string()`. A `recv` that returns fewer bytes than were sent is normal for TCP, so protocols built on sockets need their own framing.

To serve many clients, loop on `accept` and spawn a task per client. For HTTP, [`http::parse`](/kiwi/stdlib/http/#parsing-requests-and-building-responses) turns received bytes into a request, or use [`httpserver`](/kiwi/stdlib/httpserver/) and skip sockets altogether.

## TLS

The `tls` functions mirror the `socket` ones, with one difference: `tls::tcpconnect(host, port, sni)` performs the TLS handshake in the background and returns a task id. Await it to get the socket id. The port defaults to 443, and `sni` sets the server name sent during the handshake. A handshake that doesn't finish within 30 seconds fails, and so does `await`ing it.

```kiwi nocheck
conn = task::await(tls::tcpconnect("example.com", 443))
request = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n"
task::await(tls::send(conn, request.to_bytes()))
println task::await(tls::recv(conn)).to_string().lines()[0]   # HTTP/1.1 200 OK
tls::close(conn)
```

Sockets are unavailable in [safe mode](/kiwi/tools/cli/#safe-mode). For UDP, see [`udp`](/kiwi/stdlib/udp/).

<!-- api: socket -->
