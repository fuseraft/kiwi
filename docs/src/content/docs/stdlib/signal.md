---
title: signal
description: Handle SIGINT, SIGTERM and SIGHUP.
---

The `signal` package runs your code when the process receives `SIGINT` (<kbd>Ctrl</kbd>+<kbd>C</kbd>), `SIGTERM` or `SIGHUP`. Handlers run on a background task.

```kiwi nocheck
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

`signal` is unavailable in [safe mode](/kiwi/tools/cli/#safe-mode). On Windows only `SIGINT` is delivered.

<!-- api: signal -->
