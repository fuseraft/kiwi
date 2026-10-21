---
title: udp
description: Send and receive UDP datagrams.
---

The `udp` package sends and receives UDP datagrams. `udp::send(host, port, text)` fires off a single message; `udp::socket()` creates a socket for anything more.

```kiwi
server = udp::socket().bind(19999, "127.0.0.1")

listener = task::spawn(do (s)
  msg = s.recv()
  s.send(msg["from_host"], msg["from_port"], "echo: " + msg["data"])
end, [server])

client = udp::socket().timeout(2000)
client.send("127.0.0.1", 19999, "ping")
println client.recv()["data"]   # echo: ping

task::await(listener)
client.close()
server.close()
```

| Method | Does |
| :-- | :-- |
| `bind(port, host)` | Listens on a port; `host` defaults to all interfaces. Returns the socket. |
| `timeout(ms)` | Makes `recv` give up after `ms` milliseconds. Returns the socket. |
| `send(host, port, data)` | Sends a string or bytes. |
| `recv(buffer_size)` | Waits for a datagram and returns `{data, from_host, from_port}`, with `data` as a string. |
| `recv_bytes(buffer_size)` | Like `recv`, with `data` as bytes. |
| `close()` | Closes the socket. |

`recv` blocks, so a server usually runs its receive loop on a [task](/kiwi/stdlib/task/) or in a `while true` loop of its own:

```kiwi nocheck
server = udp::socket().bind(5140)
while true do
  entry = server.recv()
  println "[${entry["from_host"]}] ${entry["data"]}"
end
```

UDP doesn't guarantee delivery or ordering. `udp` is unavailable in [safe mode](/kiwi/tools/cli/#safe-mode). For TCP, see [`socket`](/kiwi/stdlib/socket/).

<!-- api: udp -->
