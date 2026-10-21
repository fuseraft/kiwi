# socket

> Asynchronous TCP and TLS sockets.

The `socket` package provides TCP sockets, and the `tls` package, defined in the same file, provides the same functions over TLS. Sockets are identified by integer ids. Operations that wait on the network (`accept`, `recv`, `send`) start a background [task](task.md) and return its id; `task::await` waits for the result.

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

To serve many clients, loop on `accept` and spawn a task per client. For HTTP, [`http::parse`](http.md#parsing-requests-and-building-responses) turns received bytes into a request, or use [`httpserver`](httpserver.md) and skip sockets altogether.

## TLS

The `tls` functions mirror the `socket` ones, with one difference: `tls::tcpconnect(host, port, sni)` performs the TLS handshake in the background and returns a task id. Await it to get the socket id. The port defaults to 443, and `sni` sets the server name sent during the handshake.

```kiwi
conn = task::await(tls::tcpconnect("example.com", 443))
request = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n"
task::await(tls::send(conn, request.to_bytes()))
println task::await(tls::recv(conn)).to_string().lines()[0]   # HTTP/1.1 200 OK
tls::close(conn)
```

Sockets are unavailable in [safe mode](../tools/cli.md#safe-mode). For UDP, see [`udp`](udp.md).

## `socket` package

A package for asynchronous TCP socket operations.

### Functions

#### `socket::tcpserver`

```kiwi
socket::tcpserver(host: string, port: integer, backlog: integer = 128): integer
```

Creates a TCP server socket bound to the given host and port.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `host` | `string` |  | The host address to bind to. |
| `port` | `integer` |  | The port number to listen on. |
| `backlog` | `integer` | `128` | Maximum number of queued connections (default 128). |

**Returns** `integer` — The socket ID.

#### `socket::tcpconnect`

```kiwi
socket::tcpconnect(host: string, port: integer): integer
```

Creates a TCP client socket connected to the given host and port.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `host` | `string` |  | The host to connect to. |
| `port` | `integer` |  | The port to connect to. |

**Returns** `integer` — The socket ID.

#### `socket::accept`

```kiwi
socket::accept(sock_id: integer): integer
```

Enqueues an accept task on a server socket.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `sock_id` | `integer` |  | The server socket ID returned by `tcpserver()`. |

**Returns** `integer` — An awaitable task ID that resolves to a client socket ID.

#### `socket::close`

```kiwi
socket::close(sock_id: integer)
```

Closes a socket and faults any pending tasks on it.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `sock_id` | `integer` |  | The socket ID to close. |

#### `socket::recv`

```kiwi
socket::recv(sock_id: integer, max_bytes: integer = 4096): integer
```

Enqueues a receive task on a client socket.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `sock_id` | `integer` |  | The client socket ID. |
| `max_bytes` | `integer` | `4096` | Maximum number of bytes to receive (default 4096). |

**Returns** `integer` — An awaitable task ID that resolves to the received bytes.

#### `socket::send`

```kiwi
socket::send(sock_id: integer, data: bytes): integer
```

Enqueues a send task on a client socket.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `sock_id` | `integer` |  | The client socket ID. |
| `data` | `bytes` |  | The data to send. |

**Returns** `integer` — An awaitable task ID.

## `tls` package

A package for asynchronous TLS-encrypted TCP socket operations.

### Functions

#### `tls::tcpserver`

```kiwi
tls::tcpserver(host: string, port: integer, backlog: integer = 128): integer
```

Creates a TLS server socket bound to the given host and port.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `host` | `string` |  | The host address to bind to. |
| `port` | `integer` |  | The port number to listen on. |
| `backlog` | `integer` | `128` | Maximum number of queued connections (default 128). |

**Returns** `integer` — The socket ID.

#### `tls::tcpconnect`

```kiwi
tls::tcpconnect(host: string, port: integer = 443, sni: string = ""): integer
```

Creates a TLS client socket connected to the given host and port.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `host` | `string` |  | The host to connect to. |
| `port` | `integer` | `443` | The port to connect to (default 443). |
| `sni` | `string` | `""` | The Server Name Indication hostname (default empty). |

**Returns** `integer` — The socket ID.

#### `tls::accept`

```kiwi
tls::accept(sock_id: integer): integer
```

Enqueues an accept task on a TLS server socket.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `sock_id` | `integer` |  | The server socket ID returned by `tcpserver()`. |

**Returns** `integer` — An awaitable task ID that resolves to a client socket ID.

#### `tls::close`

```kiwi
tls::close(sock_id: integer)
```

Closes a TLS socket and faults any pending tasks on it.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `sock_id` | `integer` |  | The socket ID to close. |

#### `tls::recv`

```kiwi
tls::recv(sock_id: integer, max_bytes: integer = 4096): integer
```

Enqueues a receive task on a TLS client socket.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `sock_id` | `integer` |  | The client socket ID. |
| `max_bytes` | `integer` | `4096` | Maximum number of bytes to receive (default 4096). |

**Returns** `integer` — An awaitable task ID that resolves to the received bytes.

#### `tls::send`

```kiwi
tls::send(sock_id: integer, data: bytes): integer
```

Enqueues a send task on a TLS client socket.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `sock_id` | `integer` |  | The client socket ID. |
| `data` | `bytes` |  | The data to send. |

**Returns** `integer` — An awaitable task ID.
