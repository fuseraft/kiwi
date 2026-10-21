# udp

> Send and receive UDP datagrams.

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

`recv` blocks, so a server usually runs its receive loop on a [task](task.md) or in a `while true` loop of its own:

```kiwi
server = udp::socket().bind(5140)
while true do
  entry = server.recv()
  println "[${entry["from_host"]}] ${entry["data"]}"
end
```

UDP doesn't guarantee delivery or ordering. `udp` is unavailable in [safe mode](../tools/cli.md#safe-mode). For TCP, see [`socket`](socket.md).

## Functions

### `udp::socket`

```kiwi
udp::socket()
```

Creates a new UDP socket.

**Returns** Socket

### `udp::send`

```kiwi
udp::send(host, port, data)
```

Sends a single datagram and closes the socket immediately.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `host` | `string` |  | Destination host. |
| `port` | `integer` |  | Destination port. |
| `data` | `string\|bytes` |  | Data to send. |

## `Socket` struct

A UDP socket supporting send and receive operations.

### `Socket.new`

```kiwi
Socket.new()
```

### `.bind`

```kiwi
bind(port, host = "0.0.0.0")
```

Binds the socket to a local port.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `port` | `integer` |  | Port number to bind. |
| `host` | `string` | `"0.0.0.0"` | Host address (default "0.0.0.0"). |

**Returns** `Socket` — self

### `.send`

```kiwi
send(host, port, data)
```

Sends data to a remote host.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `host` | `string` |  | Destination host. |
| `port` | `integer` |  | Destination port. |
| `data` | `string\|bytes` |  | Data to send. |

**Returns** `integer` — Number of bytes sent.

### `.recv`

```kiwi
recv(buffer_size = 65535)
```

Receives a datagram. Returns a hashmap with "data" (string), "from_host", "from_port".

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `buffer_size` | `integer` | `65535` | Max bytes to receive (default 65535). |

**Returns** `hashmap` — { "data": string, "from_host": string, "from_port": integer }

### `.recv_bytes`

```kiwi
recv_bytes(buffer_size = 65535)
```

Receives a datagram. Returns raw bytes in "data" field.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `buffer_size` | `integer` | `65535` | Max bytes to receive (default 65535). |

**Returns** `hashmap` — { "data": bytes, "from_host": string, "from_port": integer }

### `.timeout`

```kiwi
timeout(ms)
```

Sets the receive timeout.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `ms` | `integer` |  | Timeout in milliseconds. |

**Returns** `Socket` — self

### `.close`

```kiwi
close()
```

Closes the socket.
