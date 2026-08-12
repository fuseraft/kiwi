# `udp`

The `udp` package provides UDP (User Datagram Protocol) socket communication for sending and receiving datagrams.

---

## Package Functions

### `socket()`

Creates a new `Socket` instance.

**Returns** `Socket`

---

### `send(host, port, data)`

Sends a single datagram and immediately closes the socket. Convenience function for fire-and-forget sends.

**Parameters**

| Type              | Name   | Description           |
|-------------------|--------|-----------------------|
| `string`          | `host` | Destination hostname  |
| `integer`         | `port` | Destination port      |
| `string\|bytes`   | `data` | Data to send          |

---

## `Socket`

A UDP socket returned by `udp::socket()`.

### `Socket.new()`

Direct constructor (prefer `udp::socket()` factory function).

---

### `.bind(port, host?)`

Binds the socket to a local port for receiving datagrams.

**Parameters**

| Type      | Name   | Description                     | Default     |
|-----------|--------|---------------------------------|-------------|
| `integer` | `port` | Port number to bind             | —           |
| `string`  | `host` | Local address to bind           | `"0.0.0.0"` |

**Returns** `Socket` (self)

---

### `.send(host, port, data)`

Sends a datagram to a remote host.

**Parameters**

| Type            | Name   | Description          |
|-----------------|--------|----------------------|
| `string`        | `host` | Destination hostname |
| `integer`       | `port` | Destination port     |
| `string\|bytes` | `data` | Data to send         |

**Returns**

| Type      | Description              |
|-----------|--------------------------|
| `integer` | Number of bytes sent     |

---

### `.recv(buffer_size?)`

Receives a datagram. Returns the payload as a string.

**Parameters**

| Type      | Name          | Description                     | Default  |
|-----------|---------------|---------------------------------|----------|
| `integer` | `buffer_size` | Maximum bytes to receive        | `65535`  |

**Returns**

| Type      | Description                                                              |
|-----------|--------------------------------------------------------------------------|
| `hashmap` | `{ "data": string, "from_host": string, "from_port": integer }`          |

---

### `.recv_bytes(buffer_size?)`

Receives a datagram. Returns the payload as raw bytes.

**Parameters**

| Type      | Name          | Description                     | Default  |
|-----------|---------------|---------------------------------|----------|
| `integer` | `buffer_size` | Maximum bytes to receive        | `65535`  |

**Returns**

| Type      | Description                                                              |
|-----------|--------------------------------------------------------------------------|
| `hashmap` | `{ "data": bytes, "from_host": string, "from_port": integer }`           |

---

### `.timeout(ms)`

Sets a receive timeout. After the timeout, `.recv()` returns an empty result rather than blocking indefinitely.

**Parameters**

| Type      | Name | Description              |
|-----------|------|--------------------------|
| `integer` | `ms` | Timeout in milliseconds  |

**Returns** `Socket` (self)

---

### `.close()`

Closes the socket and releases its resources.

---

## Examples

### Simple send

```kiwi
import "udp"

# Fire and forget — no socket management needed
udp::send("127.0.0.1", 9999, "hello")
```

### Echo server

```kiwi
import "udp"

server = udp::socket().bind(9999)
println "UDP server listening on :9999"

while true do
  msg = server.recv()
  println "From ${msg["from_host"]}:${msg["from_port"]}: ${msg["data"]}"
  server.send(msg["from_host"], msg["from_port"], "echo: " + msg["data"])
end
```

### Echo client

```kiwi
import "udp"

sock = udp::socket().timeout(3000)
sock.send("127.0.0.1", 9999, "ping")

reply = sock.recv()
println reply["data"]   # echo: ping

sock.close()
```

### Receiving raw bytes

```kiwi
import "udp"

sock = udp::socket().bind(5005)
pkt = sock.recv_bytes()

println "Received ${pkt["data"].size()} bytes from ${pkt["from_host"]}"
sock.close()
```

### Log drain (syslog-style receiver)

```kiwi
import "udp"
import "fio"

drain = udp::socket().bind(514)
log_file = "/var/log/kiwi-drain.log"

while true do
  entry = drain.recv()
  line = "[${entry["from_host"]}] ${entry["data"]}"
  println line
  fio::append(log_file, line + "\n")
end
```
