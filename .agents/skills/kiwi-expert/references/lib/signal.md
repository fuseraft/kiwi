# signal

> Handle SIGINT, SIGTERM and SIGHUP.

The `signal` package runs your code when the process receives `SIGINT` (<kbd>Ctrl</kbd>+<kbd>C</kbd>), `SIGTERM` or `SIGHUP`. Handlers run on a background task.

```kiwi
signal::trap(signal::SIGINT, with () do
  println "Shutting down..."
  exit 0
end)

println "Running. Press Ctrl+C to stop."
while true do
  task::sleep(100)
end
```

Registering a handler replaces the default behavior for that signal: after trapping `SIGINT`, <kbd>Ctrl</kbd>+<kbd>C</kbd> no longer stops the program unless the handler calls `exit`.

| Function | Does |
| :-- | :-- |
| `trap(signal, handler)` | Runs `handler` when the signal arrives. |
| `reset(signal)` | Removes the handler and restores the default behavior. |
| `trapped(signal)` | Whether a handler is registered. |
| `raise_signal(signal)` | Runs the handler as if the signal had arrived; useful in tests. |

```kiwi
signal::trap(signal::SIGHUP, with () do
  println "reloading configuration"
end)

signal::raise_signal(signal::SIGHUP)
task::sleep(100)                       # give the background handler time to run

println signal::trapped(signal::SIGHUP)   # true
signal::reset(signal::SIGHUP)
```

`signal` is unavailable in [safe mode](../tools/cli.md#safe-mode). On Windows only `SIGINT` is delivered.

## Constants

| Name | Value | Description |
| :-- | :-- | :-- |
| `signal::SIGINT` | `"SIGINT"` |  |
| `signal::SIGTERM` | `"SIGTERM"` |  |
| `signal::SIGHUP` | `"SIGHUP"` |  |

## Functions

### `signal::trap`

```kiwi
signal::trap(sig, handler)
```

Registers a signal handler. The callback runs asynchronously in a background task.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `sig` | `string` |  | Signal name (e.g., signal::SIGINT, "SIGTERM"). |
| `handler` | `lambda` |  | Zero-argument lambda invoked when the signal fires. |

```kiwi
import signal
signal::trap(signal::SIGINT, fn()
  println "Shutting down..."
  exit(0)
end)
println "Running. Press Ctrl+C to stop."
while true do
  task::sleep(100)
end
```

### `signal::reset`

```kiwi
signal::reset(sig)
```

Removes a signal handler.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `sig` | `string` |  | Signal name. |

### `signal::trapped`

```kiwi
signal::trapped(sig)
```

Returns true if a signal handler is registered for the given signal.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `sig` | `string` |  | Signal name. |

**Returns** boolean

### `signal::raise_signal`

```kiwi
signal::raise_signal(sig)
```

Programmatically fires a signal (useful for testing handlers).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `sig` | `string` |  | Signal name. |
