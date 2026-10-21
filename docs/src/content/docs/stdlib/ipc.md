---
title: ipc
description: Send messages between processes over named pipes.
---

The `ipc` package connects two processes with a named pipe and exchanges text messages between them. Each message is framed with a length prefix, so every `recv` returns exactly one complete message.

1. The server calls `ipc::create(name)`, which waits for a client and returns a handle.
2. The client calls `ipc::connect(name)` with the same name.
3. Both sides `send` and `recv` messages.
4. Both sides `close` the handle when they are done.

## Example

`server.kiwi` starts the client process, then waits for it to connect:

```kiwi nocheck
pid = sys::spawn("kiwi", ["client.kiwi"])

handle = ipc::create("kiwi_demo_pipe")   # blocks until the client connects
ipc::send(handle, "ping")
println "server got: ${ipc::recv(handle)}"

ipc::send(handle, "quit")
ipc::close(handle)
sys::wait(pid)
```

`client.kiwi` answers until told to stop:

```kiwi nocheck
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
- `create` and `recv` block. Run them on a [task](/kiwi/stdlib/task/) if the process has other work to do.
- The other end doesn't have to be Kiwi: any program that speaks the framing (a 4-byte big-endian length, then UTF-8 bytes) can talk to it.
- `ipc` is unavailable in [safe mode](/kiwi/tools/cli/#safe-mode).

<!-- api: ipc -->
