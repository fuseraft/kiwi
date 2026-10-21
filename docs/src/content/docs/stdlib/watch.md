---
title: watch
description: Watch files and directories for created, modified and deleted entries.
---

The `watch` package watches a file or directory and calls your handlers when entries are created, modified or deleted. It polls on a background task, so it works the same on every platform.

```kiwi nocheck
watcher = watch::new("./data")
watcher.on_create(with (path) do println "created: ${path}" end)
watcher.on_modify(with (path) do println "modified: ${path}" end)
watcher.on_delete(with (path) do println "deleted: ${path}" end)
watcher.start()

# ... later
watcher.stop()
```

`on_change` registers one handler for every event, receiving the event name and the path. `watch::watch(path, handler)` creates, configures and starts a watcher in one call:

```kiwi nocheck
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

<!-- api: watch -->
