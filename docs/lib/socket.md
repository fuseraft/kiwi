# `socket` / `tls`

The `socket` package provides low-level asynchronous TCP networking functionality in Kiwi. It allows creating TCP servers and clients, accepting connections, and performing non-blocking send/receive operations that integrate with the [`task`](./task.md) package via awaitable task IDs.

The `tls` package provides the same interface and is designed to work with TLS.

## Table of Contents

- [`socket` Package Functions](#socket-package-functions)
  - [`socket::tcpserver(host, port, backlog)`](#tcpserverhost-port-backlog)
  - [`socket::tcpconnect(host, port)`](#tcpconnecthost-port)
  - [`socket::accept(sock_id)`](#acceptsock_id)
  - [`socket::close(sock_id)`](#closesock_id)
  - [`socket::recv(sock_id, max_bytes)`](#recvsock_id-max_bytes)
  - [`socket::send(sock_id, data)`](#sendsock_id-data)
- [`tls` Package Functions](#tls-package-functions)
  - [`tls::tcpconnect(host, port?, sni?)`](#tlstcpconnecthost-port-sni)
  - [`tls::close(sock_id)`](#tlsclosesock_id)
  - [`tls::recv(sock_id, max_bytes?)`](#tlsrecvsock_id-max_bytes)
  - [`tls::send(sock_id, data)`](#tlssendsock_id-data)


## `socket` Package Functions

### `tcpserver(host, port, backlog)`
Creates a TCP server socket and binds it to the specified host and port.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `host` | The host address to bind to (e.g., `"127.0.0.1"`. |
| `integer` | `port` | The port number to listen on. |
| `integer` | `backlog` | The maximum length of the pending connections queue (optional, default `128`). |

**Returns**
| Type | Description |
| :--- | :--- |
| `integer` | The socket identifier for the server socket. |

---

### `tcpconnect(host, port)`
Creates a TCP client socket and connects to the remote host and port.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `host` | The remote host address to connect to. |
| `integer` | `port` | The remote port number to connect to. |

**Returns**
| Type | Description |
| :--- | :--- |
| `integer` | The socket identifier for the client socket. |

---

### `accept(sock_id)`
Enqueues an asynchronous accept operation on a server socket. The operation completes when a new client connection is available.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `sock_id` | The server socket identifier. |

**Returns**
| Type | Description |
| :--- | :--- |
| `integer` | An awaitable task identifier. Use `task::await()` on this ID to obtain the new client socket identifier (`integer`). |

---

### `close(sock_id)`
Closes the socket and cancels/faults any pending asynchronous operations on it.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `sock_id` | The socket identifier to close. |

**Returns**
_None_

---

### `recv(sock_id, max_bytes)`
Enqueues an asynchronous receive operation on a client socket. The operation completes when data is available or the connection is closed.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `sock_id` | The client socket identifier. |
| `integer` | `max_bytes` | The maximum number of bytes to receive (optional, default `4096`). |

**Returns**
| Type | Description |
| :--- | :--- |
| `integer` | An awaitable task identifier. Use `task::await()` on this ID to obtain the received data (`bytes`). An empty `bytes` value indicates the connection was closed. |

---

### `send(sock_id, data)`
Enqueues an asynchronous send operation on a client socket. The operation completes when the data has been fully sent (or the connection is closed).

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `sock_id` | The client socket identifier. |
| `bytes` | `data` | The data to send. |

**Returns**
| Type | Description |
| :--- | :--- |
| `integer` | An awaitable task identifier. Use `task::await()` on this ID to obtain the number of bytes sent (`integer`). |

Here is clean, well-structured Markdown documentation for your `tls` package, styled exactly like the `task` and `http` examples you provided.

# `tls` Package Functions

### `tls::tcpconnect(host, port?, sni?)`
Establishes a TLS-secured TCP connection to the specified host and port.  
Returns a socket ID that can be used with `recv`, `send`, and `close`.

**Parameters**
| Type      | Name   | Description                                                                 | Default |
|-----------|--------|-----------------------------------------------------------------------------|---------|
| `string`  | `host` | The hostname or IP address to connect to                                   | —       |
| `integer` | `port` | The port number (typically 443 for HTTPS)                                   | `443`   |
| `string`  | `sni`  | Server Name Indication (SNI) hostname for virtual hosting; defaults to `host` | `""` (uses `host`) |

**Returns**
| Type      | Description                          |
|-----------|--------------------------------------|
| `integer` | Socket ID (or negative on failure)   |

---

### `tls::close(sock_id)`
Closes the TLS socket and faults any pending send/receive tasks associated with it.

**Parameters**
| Type      | Name      | Description                  |
|-----------|-----------|------------------------------|
| `integer` | `sock_id` | The socket ID to close       |

**Returns**  
_None_

**Throws / faults**  
Any pending tasks on this socket will be faulted with a socket error.

---

### `tls::recv(sock_id, max_bytes?)`
Enqueues an asynchronous receive operation on the TLS socket.  
Returns a task ID that you can `await` to get the received bytes.

**Parameters**
| Type      | Name         | Description                                      | Default |
|-----------|--------------|--------------------------------------------------|---------|
| `integer` | `sock_id`    | The socket ID                                    | —       |
| `integer` | `max_bytes`  | Maximum number of bytes to receive in one call   | `4096`  |

**Returns**
| Type      | Description                          |
|-----------|--------------------------------------|
| `integer` | Task ID (await with `task.await()`)  |

---

### `tls::send(sock_id, data)`
Enqueues an asynchronous send operation on the TLS socket.  
Returns a task ID that you can `await` to confirm the send completed.

**Parameters**
| Type      | Name      | Description                          |
|-----------|-----------|--------------------------------------|
| `integer` | `sock_id` | The socket ID                        |
| `bytes`   | `data`    | The data to send                     |

**Returns**
| Type      | Description                          |
|-----------|--------------------------------------|
| `integer` | Task ID (await with `task.await()`)  |
