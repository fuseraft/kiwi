# `signal`

The `signal` package provides OS signal handling for `SIGINT`, `SIGTERM`, and `SIGHUP`. Handlers are registered with `trap()` and run asynchronously in a background task.

---

## Constants

| Constant          | Value       | Description                          |
|-------------------|-------------|--------------------------------------|
| `signal::SIGINT`  | `"SIGINT"`  | Interrupt (Ctrl+C)                   |
| `signal::SIGTERM` | `"SIGTERM"` | Termination request                  |
| `signal::SIGHUP`  | `"SIGHUP"`  | Hang-up / configuration reload       |

---

## Package Functions

### `trap(sig, handler)`

Registers a handler for the given signal. The handler is invoked asynchronously in a background task when the signal fires.

**Parameters**

| Type     | Name      | Description                                        |
|----------|-----------|----------------------------------------------------|
| `string` | `sig`     | Signal name — use the constants or a string literal |
| `lambda` | `handler` | Zero-argument lambda: `with () do ... end`         |

---

### `reset(sig)`

Removes the handler for the given signal.

**Parameters**

| Type     | Name  | Description     |
|----------|-------|-----------------|
| `string` | `sig` | Signal name     |

---

### `trapped(sig)`

Returns `true` if a handler is currently registered for the given signal.

**Parameters**

| Type     | Name  | Description  |
|----------|-------|--------------|
| `string` | `sig` | Signal name  |

**Returns**

| Type      | Description                                |
|-----------|--------------------------------------------|
| `boolean` | `true` if a handler is registered          |

---

### `raise_signal(sig)`

Programmatically fires a signal. Useful for testing signal handlers.

**Parameters**

| Type     | Name  | Description  |
|----------|-------|--------------|
| `string` | `sig` | Signal name  |

---

## Examples

### Graceful shutdown on Ctrl+C

```kiwi
import "signal"
import "task"

signal::trap(signal::SIGINT, with () do
  println "Shutting down gracefully..."
  exit(0)
end)

println "Running. Press Ctrl+C to stop."

while true do
  task::sleep(100)
end
```

### Reload configuration on SIGHUP

```kiwi
import "signal"
import "task"

signal::trap(signal::SIGHUP, with () do
  println "Reloading configuration..."
  # reload_config()
end)

println "PID: ${sys::pid()}"

while true do
  task::sleep(500)
end
```

### Cleanup on SIGTERM

```kiwi
import "signal"
import "task"

signal::trap(signal::SIGTERM, with () do
  println "SIGTERM received — cleaning up"
  # close connections, flush buffers, etc.
  exit(0)
end)

while true do
  task::sleep(100)
end
```

### Testing a handler

```kiwi
import "signal"

signal::trap(signal::SIGINT, with () do
  println "Handler fired!"
end)

signal::raise_signal(signal::SIGINT)   # Fires the handler immediately
task::sleep(100)                       # Give the background task time to run
```
