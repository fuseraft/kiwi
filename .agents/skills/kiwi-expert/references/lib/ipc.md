# ipc

> Send messages between processes over named pipes.

The `ipc` package connects two processes with a named pipe and exchanges text messages between them. Each message is framed with a length prefix, so every `recv` returns exactly one complete message.

1. The server calls `ipc::create(name)`, which waits for a client and returns a handle.
2. The client calls `ipc::connect(name)` with the same name.
3. Both sides `send` and `recv` messages.
4. Both sides `close` the handle when they are done.

## Example

`server.kiwi` starts the client process, then waits for it to connect:

```kiwi
pid = sys::spawn("kiwi", ["client.kiwi"])

handle = ipc::create("kiwi_demo_pipe")   # blocks until the client connects
ipc::send(handle, "ping")
println "server got: ${ipc::recv(handle)}"

ipc::send(handle, "quit")
ipc::close(handle)
sys::wait(pid)
```

`client.kiwi` answers until told to stop:

```kiwi
handle = ipc::connect("kiwi_demo_pipe")

while true do
  message = ipc::recv(handle)
  break when message == "quit"
  ipc::send(handle, "pong")
end

ipc::close(handle)
```

```text
$ kiwi server.kiwi
server got: pong
```

## Notes

- Pipe names are system-wide. On Linux and macOS they live under `/tmp`; on Windows they are `\\.\pipe\<name>`.
- One client connects per `create` call. Call `create` again to accept another client.
- `create` and `recv` block. Run them on a [task](task.md) if the process has other work to do.
- The other end doesn't have to be Kiwi: any program that speaks the framing (a 4-byte big-endian length, then UTF-8 bytes) can talk to it.
- `ipc` is unavailable in [safe mode](../tools/cli.md#safe-mode).

## Functions

### `ipc::create`

```kiwi
ipc::create(_name: string): integer
```

Create a named pipe server and wait for a client to connect.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_name` | `string` |  | The pipe name. |

**Returns** `integer` — A handle ID for the connected pipe.

**Throws** `SystemError` — When the pipe cannot be created.

### `ipc::connect`

```kiwi
ipc::connect(_name: string): integer
```

Connect to a named pipe server.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_name` | `string` |  | The pipe name. |

**Returns** `integer` — A handle ID for the connected pipe.

**Throws** `SystemError` — When the connection fails.

### `ipc::send`

```kiwi
ipc::send(_handle: integer, _message: string)
```

Send a string message over an open pipe.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_handle` | `integer` |  | The pipe handle returned by `create` or `connect`. |
| `_message` | `string` |  | The message to send. |

**Throws** `SystemError` — When the send fails.

### `ipc::recv`

```kiwi
ipc::recv(_handle: integer): string
```

Receive a string message from an open pipe (blocking).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_handle` | `integer` |  | The pipe handle returned by `create` or `connect`. |

**Returns** `string` — The received message.

**Throws** `SystemError` — When the receive fails or the pipe is closed.

### `ipc::close`

```kiwi
ipc::close(_handle: integer)
```

Close an open pipe handle.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_handle` | `integer` |  | The pipe handle to close. |
