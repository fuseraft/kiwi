# watch

> Watch files and directories for created, modified and deleted entries.

The `watch` package watches a file or directory and calls your handlers when entries are created, modified or deleted. It polls on a background task, so it works the same on every platform.

```kiwi
watcher = watch::new("./data")
watcher.on_create(with (path) do println "created: ${path}" end)
watcher.on_modify(with (path) do println "modified: ${path}" end)
watcher.on_delete(with (path) do println "deleted: ${path}" end)
watcher.start()

# ... later
watcher.stop()
```

`on_change` registers one handler for every event, receiving the event name and the path. `watch::watch(path, handler)` creates, configures and starts a watcher in one call:

```kiwi
w = watch::watch("./src", with (event, path) do
  println "[${event}] ${path}"
end)

while w.is_running() do
  task::sleep(1000)
end
```

## Options

`watch::new(path, recursive, interval_ms, ignore)`:

| Argument | Default | Meaning |
| :-- | :-- | :-- |
| `recursive` | `true` | Also watch subdirectories. |
| `interval_ms` | `300` | How often to check for changes. |
| `ignore` | `[]` | Path fragments to skip, such as `".git"` or `"node_modules"`. |

## Polling by hand

Instead of `start()`, call `poll()` yourself. The first call records the current state; each later call fires events for whatever changed since the previous one.

```kiwi
dir = fio::combine([fio::tmpdir(), "watch-demo"])
fio::mkdirp(dir)
file = fio::combine([dir, "a.txt"])

events = []
w = watch::new(dir)
w.on_change(with (event, path) do
  events.push("${event} ${fio::filename(path)}")
end)

w.poll()                       # baseline
fio::write(file, "1")
w.poll()
task::sleep(20)
fio::append(file, "22")
w.poll()
fio::remove(file)
w.poll()

println events   # ["create a.txt", "modify a.txt", "delete a.txt"]
fio::rmdirf(dir)
```

## How changes are detected

Each check lists the watched entries and compares every file's last-write time and size with the previous check. As a result, several writes between two checks are reported as one `modify`, a change is noticed up to `interval_ms` after it happens, and directories are reported when created or deleted but never as modified.

## Functions

### `watch::new`

```kiwi
watch::new(_path: string, _recursive: boolean = true, _interval_ms: integer = 300, _ignore: list = []): Watcher
```

Creates a new Watcher without starting it.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The file or directory path to watch. |
| `_recursive` | `boolean` | `true` | Watch subdirectories recursively. Defaults to `true`. |
| `_interval_ms` | `integer` | `300` | Poll interval in milliseconds. Defaults to `300`. |
| `_ignore` | `list` | `[]` | Entries whose path contains any of these substrings are skipped. Defaults to `[]`. |

**Returns** `Watcher` — Watcher

### `watch::watch`

```kiwi
watch::watch(_path: string, callback: lambda, _recursive: boolean = true, _interval_ms: integer = 300): Watcher
```

Creates a Watcher, registers a single callback for all change events, and starts it.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The file or directory path to watch. |
| `callback` | `lambda` |  | Invoked with `(event, path)` for every create/modify/delete. |
| `_recursive` | `boolean` | `true` | Watch subdirectories recursively. Defaults to `true`. |
| `_interval_ms` | `integer` | `300` | Poll interval in milliseconds. Defaults to `300`. |

**Returns** `Watcher` — The running watcher. Call `.stop()` to end it.

## `Watcher` struct

A background poller that reports created, modified, and deleted entries under a path.

```kiwi
import watch

w = watch::new("./data")
w.on_create(with (path) do println "created: ${path}" end)
w.on_modify(with (path) do println "modified: ${path}" end)
w.on_delete(with (path) do println "deleted: ${path}" end)
w.start()

# ... later ...
w.stop()
```

### `Watcher.new`

```kiwi
Watcher.new(_path: string, _recursive: boolean = true, _interval_ms: integer = 300, _ignore: list = [])
```

Creates a new Watcher. Call `.start()` to begin watching.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The file or directory path to watch. |
| `_recursive` | `boolean` | `true` | Watch subdirectories recursively. Defaults to `true`. |
| `_interval_ms` | `integer` | `300` | Poll interval in milliseconds. Defaults to `300`. |
| `_ignore` | `list` | `[]` | Entries whose path contains any of these substrings are skipped. Defaults to `[]`. |

### `.on_create`

```kiwi
on_create(callback: lambda): Watcher
```

Registers a callback fired when a new entry appears.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `callback` | `lambda` |  | Invoked with `(path)`. |

**Returns** `Watcher` — self, for chaining.

### `.on_modify`

```kiwi
on_modify(callback: lambda): Watcher
```

Registers a callback fired when an entry's contents or size change.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `callback` | `lambda` |  | Invoked with `(path)`. |

**Returns** `Watcher` — self, for chaining.

### `.on_delete`

```kiwi
on_delete(callback: lambda): Watcher
```

Registers a callback fired when an entry disappears.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `callback` | `lambda` |  | Invoked with `(path)`. |

**Returns** `Watcher` — self, for chaining.

### `.on_change`

```kiwi
on_change(callback: lambda): Watcher
```

Registers a callback fired for every create, modify, or delete.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `callback` | `lambda` |  | Invoked with `(event, path)`. |

**Returns** `Watcher` — self, for chaining.

### `.start`

```kiwi
start(): Watcher
```

Takes an initial snapshot and starts polling in a background task. Calling `.start()` while already running has no effect.

**Returns** `Watcher` — self.

### `.stop`

```kiwi
stop(): Watcher
```

Stops watching. The background task exits on its next tick.

**Returns** `Watcher` — self.

### `.is_running`

```kiwi
is_running(): boolean
```

Checks whether the watcher is currently running.

**Returns** `boolean` — boolean

### `.interval_ms`

```kiwi
interval_ms(): integer
```

Gets the poll interval in milliseconds.

**Returns** `integer` — integer

### `.poll`

```kiwi
poll(): Watcher
```

Immediately scans and fires any pending events synchronously, without waiting for the next background tick. The first call (before `.start()`, or on a freshly created Watcher) only establishes the baseline snapshot and fires no events; later calls diff against the previous snapshot as usual. Useful for tests or manual polling loops instead of `.start()`.

**Returns** `Watcher` — self.
