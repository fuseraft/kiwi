# `socket` / `tls`

The `socket` package provides low-level asynchronous TCP networking functionality in Kiwi. It allows creating TCP servers and clients, accepting connections, and performing non-blocking send/receive operations that integrate with the [`task`](./task.md) package via awaitable task IDs.

The `tls` package provides the same interface and is designed to work with TLS.

---

## `socket` Package Functions

### `socket::tcpserver(host, port, backlog)`
Creates a TCP server socket and binds it to the specified host and port.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `host` | The host address to bind to (e.g., `"127.0.0.1"`). |
| `integer` | `port` | The port number to listen on. |
| `integer` | `backlog` | The maximum length of the pending connections queue (optional, default `128`). |

**Returns**

| Type | Description |
| :--- | :--- |
| `integer` | The socket identifier for the server socket. |

---

### `socket::tcpconnect(host, port)`
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

### `socket::accept(sock_id)`
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

### `socket::close(sock_id)`
Closes the socket and cancels/faults any pending asynchronous operations on it.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `sock_id` | The socket identifier to close. |

**Returns**
_None_

---

### `socket::recv(sock_id, max_bytes)`
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

### `socket::send(sock_id, data)`
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

**Example: Echo server**

A minimal TCP echo server that accepts one connection, reads data, and echoes it back.

```kiwi
import "socket"
import "task"

# Bind and listen on localhost:8080
server = socket::tcpserver("127.0.0.1", 8080)
println "Server listening on 127.0.0.1:8080"

# Block until a client connects
client_id = task::await(socket::accept(server))
println "Client connected: ${client_id}"

# Receive up to 4096 bytes
data = task::await(socket::recv(client_id))
println "Received: ${data.to_list().join("")}"

# Echo back
task::await(socket::send(client_id, data))

socket::close(client_id)
socket::close(server)
println "Done."
```

**Example: TCP client**

Connect to a server, send a message, and read the response.

```kiwi
import "socket"
import "task"

conn = socket::tcpconnect("127.0.0.1", 8080)
println "Connected to server"

task::await(socket::send(conn, "Hello".to_bytes()))

response = task::await(socket::recv(conn))
println "Server replied: ${response.to_list().join("")}"

socket::close(conn)
```

**Example: Persistent echo server (multi-client loop)**

Accept and handle clients in a loop using `task::spawn`.

```kiwi
import "socket"
import "task"

fn handle_client(client_id)
  data = task::await(socket::recv(client_id))
  if data.size() > 0
    task::await(socket::send(client_id, data))
  end
  socket::close(client_id)
end

server = socket::tcpserver("127.0.0.1", 9000)
println "Echo server running on port 9000"

# Accept connections indefinitely
while true do
  client_id = task::await(socket::accept(server))
  task::spawn(with (id) do handle_client(id) end, [client_id])
end
```

**Example: Concurrent port scanner**

Probe a range of ports in parallel. Each probe connects, then polls `task::status` on a `recv` task to detect whether the remote sent a TCP RST (closed) or is waiting for client data (open). A configurable timeout declares the port open if neither happens within the window.

```kiwi
import "socket"
import "task"

/#
  probe(host, port, ch, timeout_ms)

  Detection strategy:
    - RST within timeout  -> task faults  -> port closed -> sends 0 on ch
    - Banner received     -> task completes -> port open  -> sends port on ch
    - Timeout with no RST -> port open (service silent) -> sends port on ch
#/
fn probe(host: string, port: integer, ch: Channel, timeout_ms: integer)
  try
    sock     = socket::tcpconnect(host, port)
    recv_tid = socket::recv(sock, 1)

    elapsed = 0
    open    = false

    while elapsed < timeout_ms do
      s = task::status(recv_tid)

      if s == "Completed"
        open = true   # banner received — definitely open
        break
      elsif s == "Faulted"
        open = false  # TCP RST — port closed
        break
      end

      task::sleep(10)
      elapsed += 10
    end

    socket::close(sock) # faults recv_tid if still running
    open = true when elapsed >= timeout_ms

    ch.send(open ? port : 0)
  catch (err)
    ch.send(0) # connection refused or other error -> closed
  end
end

host       = "127.0.0.1"
port_start = 1
port_end   = 1024
timeout_ms = 500

ch         = task::channel(0)
total      = port_end - port_start + 1
open_ports = []

for port in [port_start to port_end] do
  task::spawn(with (h, p, c, t) do
    probe(h, p, c, t)
  end, [host, port, ch, timeout_ms])
end

received = 0
while received < total do
  result = ch.recv()
  received += 1
  if result > 0
    open_ports.push(result)
    println "  [open]  port ${result}"
  end
end

open_ports.sort()
println "Open ports: " + open_ports.join(", ")
```

> **Note:** `task::status(id)` is non-blocking and does not throw for faulted tasks — use it for polling. `task::await(id)` blocks and throws a catchable `SystemError` for faulted tasks. See the [`task` docs](./task.md) for details.

---

## `tls` Package Functions

### `tls::tcpserver(host, port, backlog)`
Creates a TCP server socket and binds it to the specified host and port.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `host` | The host address to bind to (e.g., `"127.0.0.1"`). |
| `integer` | `port` | The port number to listen on. |
| `integer` | `backlog` | The maximum length of the pending connections queue (optional, default `128`). |

**Returns**

| Type | Description |
| :--- | :--- |
| `integer` | The socket identifier for the server socket. |

---

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

### `tls::accept(sock_id)`
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

**Example: HTTPS request over TLS**

Connect to a remote HTTPS server and issue a raw HTTP/1.1 GET request.

```kiwi
import "socket"
import "task"

# Connect with TLS — port defaults to 443
conn = tls::tcpconnect("example.com", 443)
println "TLS connection established"

# Build a minimal HTTP/1.1 request
request = "GET / HTTP/1.1\r\nHost: example.com\r\nConnection: close\r\n\r\n"
task::await(tls::send(conn, request.to_bytes()))

# Read the response (may need multiple recv calls for large responses)
response = task::await(tls::recv(conn))
println response.to_list().join("")

tls::close(conn)
```

**Example: TLS client with SNI**

When connecting to a server that hosts multiple domains, pass the SNI hostname explicitly.

```kiwi
import "socket"
import "task"

# SNI lets the server select the correct certificate for "api.example.com"
conn = tls::tcpconnect("203.0.113.10", 443, "api.example.com")

request = "GET /status HTTP/1.1\r\nHost: api.example.com\r\nConnection: close\r\n\r\n"
task::await(tls::send(conn, request.to_bytes()))

data = task::await(tls::recv(conn))
println "Response length: ${data.size()} bytes"

tls::close(conn)
```
