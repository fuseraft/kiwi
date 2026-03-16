# `ipc`

The `ipc` package enables **interprocess communication** between Kiwi programs (or between a Kiwi program and any other process) using **named pipes**.

Messages are framed with a 4-byte big-endian length prefix followed by a UTF-8 payload, so both sides always receive exactly one complete message per `recv` call — no fragmentation or partial reads.

The typical usage pattern:

1. **Server side** — call `ipc::create(name)`, which blocks until a client connects, then returns a handle.
2. **Client side** — call `ipc::connect(name)`, which connects to the waiting server, then returns a handle.
3. Both sides exchange messages with `ipc::send` and `ipc::recv`.
4. Both sides call `ipc::close` when finished.

---

## Package Functions

### `create(_name)`

Creates a named pipe **server** and blocks until one client connects.

**Parameters**

| Type     | Name    | Description       |
|----------|---------|-------------------|
| `string` | `_name` | The pipe name     |

**Returns**

| Type      | Description                              |
|-----------|------------------------------------------|
| `integer` | Handle ID for the connected pipe         |

**Throws**
- `SystemError` — if the pipe cannot be created (e.g., name conflict, OS error)

**Example**
```kiwi
handle = ipc::create("my_pipe")
println "Client connected. Handle: ${handle}"
```

---

### `connect(_name)`

Connects to an existing named pipe **server**.

**Parameters**

| Type     | Name    | Description       |
|----------|---------|-------------------|
| `string` | `_name` | The pipe name     |

**Returns**

| Type      | Description                              |
|-----------|------------------------------------------|
| `integer` | Handle ID for the connected pipe         |

**Throws**
- `SystemError` — if the connection fails (e.g., no server listening, OS error)

**Example**
```kiwi
handle = ipc::connect("my_pipe")
println "Connected to server. Handle: ${handle}"
```

---

### `send(_handle, _message)`

Sends a UTF-8 string message over an open pipe. The message is framed with a 4-byte big-endian length prefix so the receiver always gets the full message.

**Parameters**

| Type      | Name       | Description                                         |
|-----------|------------|-----------------------------------------------------|
| `integer` | `_handle`  | Pipe handle returned by `create` or `connect`       |
| `string`  | `_message` | The message to send                                 |

**Returns**
Nothing (`none`).

**Throws**
- `SystemError` — if the handle is invalid or the send fails

**Example**
```kiwi
ipc::send(handle, "hello from server")
```

---

### `recv(_handle)`

Reads a complete message from an open pipe. Blocks until a full message is available.

**Parameters**

| Type      | Name      | Description                                         |
|-----------|-----------|-----------------------------------------------------|
| `integer` | `_handle` | Pipe handle returned by `create` or `connect`       |

**Returns**

| Type     | Description           |
|----------|-----------------------|
| `string` | The received message  |

**Throws**
- `SystemError` — if the handle is invalid, the pipe is closed unexpectedly, or a read error occurs

**Example**
```kiwi
msg = ipc::recv(handle)
println "Received: ${msg}"
```

---

### `close(_handle)`

Closes an open pipe handle and releases the underlying OS resource.

**Parameters**

| Type      | Name      | Description            |
|-----------|-----------|------------------------|
| `integer` | `_handle` | Pipe handle to close   |

**Returns**
Nothing (`none`).

**Example**
```kiwi
ipc::close(handle)
```

---

## Full Example — Two Processes Communicating

The pattern below shows a **parent process** that spawns a **child process**, then communicates with it over a named pipe.

### `server.kiwi` (parent / server side)

```kiwi
# server.kiwi — creates the pipe, spawns the client, talks to it

# Spawn the client process first (it will call ipc::connect)
pid = sys::spawn("kiwi", ["client.kiwi"])

# Create the server end — blocks until client connects
handle = ipc::create("kiwi_demo_pipe")
println "Client connected (pid=${pid})"

# Exchange messages
ipc::send(handle, "ping")
reply = ipc::recv(handle)
println "Server got: ${reply}"

ipc::send(handle, "quit")
ipc::close(handle)
sys::wait(pid)
println "Done."
```

### `client.kiwi` (child / client side)

```kiwi
# client.kiwi — connects to the pipe and responds

handle = ipc::connect("kiwi_demo_pipe")

loop
  msg = ipc::recv(handle)
  if msg == "quit"
    break
  end
  ipc::send(handle, "pong")
end

ipc::close(handle)
```

**Running the example:**

```
kiwi server.kiwi
```

Expected output:

```
Client connected (pid=12345)
Server got: pong
Done.
```

---

## Notes

- Named pipe names are **OS-scoped**. On Linux/macOS they map to `/tmp/CoreFxPipe_<name>`; on Windows they map to `\\.\pipe\<name>`.
- Only **one client** can connect per `create` call. To serve multiple clients, call `create` again after the first client disconnects.
- Both `recv` and `create` are **blocking** — use `task::spawn` if you need non-blocking behaviour.
- Always call `ipc::close` when finished to avoid resource leaks.
