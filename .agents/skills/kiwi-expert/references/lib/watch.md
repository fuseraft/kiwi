# `watch`

The `watch` package provides a `Watcher` struct that polls a file or directory
on a background task and reports entries that are created, modified, or
deleted. It's built on `fio` and `task`, so it works anywhere Kiwi runs
without relying on OS-specific file system event APIs.

---

## Quick Start

```kiwi
import "watch"

w = watch::new("./data")
w.on_create(with (path) do println "created: ${path}" end)
w.on_modify(with (path) do println "modified: ${path}" end)
w.on_delete(with (path) do println "deleted: ${path}" end)
w.start()

# ... later ...
w.stop()
```

---

## Package Functions

### `watch::new(path, recursive = true, interval_ms = 300, ignore = [])`

Creates a new `Watcher` without starting it.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `path` | The file or directory path to watch. |
| `boolean` | `recursive` | Watch subdirectories recursively. Defaults to `true`. |
| `integer` | `interval_ms` | Poll interval in milliseconds. Defaults to `300`. |
| `list` | `ignore` | Entries whose path contains any of these substrings are skipped. Defaults to `[]`. |

**Returns**

| Type | Description |
| :--- | :--- |
| `Watcher` | An idle watcher. Call `.start()` to begin watching. |

---

### `watch::watch(path, callback, recursive = true, interval_ms = 300)`

Creates a `Watcher`, registers `callback` for every change event, and starts it immediately.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `path` | The file or directory path to watch. |
| `lambda` | `callback` | Invoked with `(event, path)` for every create/modify/delete. |
| `boolean` | `recursive` | Watch subdirectories recursively. Defaults to `true`. |
| `integer` | `interval_ms` | Poll interval in milliseconds. Defaults to `300`. |

**Returns**

| Type | Description |
| :--- | :--- |
| `Watcher` | The running watcher. Call `.stop()` to end it. |

---

## `Watcher` Methods

### `.on_create(callback)`
Registers a callback fired when a new entry appears. Invoked with `(path)`.

### `.on_modify(callback)`
Registers a callback fired when an entry's contents or size change. Invoked with `(path)`.

### `.on_delete(callback)`
Registers a callback fired when an entry disappears. Invoked with `(path)`.

### `.on_change(callback)`
Registers a callback fired for every create, modify, or delete. Invoked with `(event, path)`, where `event` is `"create"`, `"modify"`, or `"delete"`.

Multiple callbacks can be registered per event; each one runs on every matching change.

---

### `.start()`
Takes an initial snapshot and starts polling in a background task. Calling `.start()` while already running has no effect.

**Returns** `Watcher` — self.

---

### `.stop()`
Stops watching. The background task exits on its next tick (within `interval_ms`).

**Returns** `Watcher` — self.

---

### `.is_running()`
**Returns** `boolean` — `true` while the watcher's background task is active.

---

### `.interval_ms()`
**Returns** `integer` — the configured poll interval.

---

### `.poll()`
Immediately scans and fires any pending events synchronously, without waiting for the next background tick. The **first** call (on a freshly created `Watcher`, before `.start()`) only establishes the baseline snapshot and fires no events; subsequent calls diff against the previous snapshot as usual. Useful for tests or a manual polling loop instead of `.start()`.

**Returns** `Watcher` — self.

---

## How Change Detection Works

On each tick, `watch` lists the entries under `path` (via `fio::listdir`) and
builds a signature for each file from its last-write time and size (via
`fio::fileinfo` / `fio::filesize`). Comparing the new signature map against
the previous one produces the create/modify/delete events. Directories are
tracked for creation and deletion but never reported as "modified".

Because detection is signature-based polling rather than a live OS event
feed, very fast back-to-back writes between two polls are collapsed into a
single `modify` event, and changes are only observed after up to
`interval_ms` has elapsed. Lower `interval_ms` for snappier detection at the
cost of more frequent directory scans.

---

## Examples

### Watching a single file

```kiwi
import "watch"

w = watch::new("./config.json")
w.on_modify(with (path) do
  println "config changed, reloading..."
end)
w.start()
```

### Ignoring noisy paths

```kiwi
import "watch"

w = watch::new("./project", true, 500, [".git", "node_modules", ".log"])
w.on_change(with (event, path) do
  println "${event}: ${path}"
end)
w.start()
```

### One-liner with `watch::watch()`

```kiwi
import "watch"
import "task"

w = watch::watch("./src", with (event, path) do
  println "[${event}] ${path}"
end)

while w.is_running() do
  task::sleep(1000)
end
```

### Manual polling instead of a background task

```kiwi
import "watch"

w = watch::new("./data")
w.on_create(with (path) do println "created: ${path}" end)

w.poll() # establishes the baseline; no events fire yet

# ... time passes, files change ...

w.poll() # fires events for anything that changed since the last poll
```
