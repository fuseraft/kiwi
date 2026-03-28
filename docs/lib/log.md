# `log`

The `log` package provides structured logging with severity levels, optional timestamps, and optional file output. Output goes to stderr; log lines below the configured minimum level are suppressed.

---

## Log Levels

Levels in ascending severity order: `DEBUG` → `INFO` → `WARN` → `ERROR` → `FATAL`

Only messages at or above the configured level are emitted. The default level is `INFO`.

---

## Package Functions

### `set_level(level)`

Sets the minimum log level. Messages below this level are silently dropped.

**Parameters**

| Type     | Name    | Description                                          |
|----------|---------|------------------------------------------------------|
| `string` | `level` | One of `"DEBUG"`, `"INFO"`, `"WARN"`, `"ERROR"`, `"FATAL"` |

---

### `set_file(path)`

Appends log output to a file in addition to stderr. Pass `null` to disable file output.

**Parameters**

| Type            | Name   | Description                        |
|-----------------|--------|------------------------------------|
| `string\|null`  | `path` | File path, or `null` to disable    |

---

### `timestamps(enabled)`

Enables or disables timestamps in log lines. Enabled by default.

**Parameters**

| Type      | Name      | Description                    |
|-----------|-----------|--------------------------------|
| `boolean` | `enabled` | `true` to include timestamps   |

---

### `debug(msg)`

Logs a `DEBUG` message.

---

### `info(msg)`

Logs an `INFO` message.

---

### `warn(msg)`

Logs a `WARN` message.

---

### `error(msg)`

Logs an `ERROR` message.

---

### `fatal(msg)`

Logs a `FATAL` message and calls `exit(1)`.

---

**All logging functions accept:**

| Type  | Name  | Description              |
|-------|-------|--------------------------|
| `any` | `msg` | The message to log       |

---

## Output Format

```
[LEVEL] [timestamp] message
```

Example:
```
[INFO] [2026-03-22 18:00:00Z] Server started
[WARN] [2026-03-22 18:01:05Z] Disk usage above 80%
[ERROR] [2026-03-22 18:02:11Z] Connection refused
```

---

## Examples

```kiwi
import "log"

log::info("Application starting")
log::debug("This won't appear — default level is INFO")

log::set_level("DEBUG")
log::debug("Now debug messages are visible")

log::warn("Low memory")
log::error("Failed to connect to database")
```

### File output

```kiwi
import "log"

log::set_file("/var/log/myapp.log")
log::set_level("WARN")

log::info("This is suppressed")
log::warn("This goes to stderr AND the file")
log::error("So does this")
```

### Disable timestamps

```kiwi
import "log"

log::timestamps(false)
log::info("Clean output without timestamps")
# [INFO] Clean output without timestamps
```

### Fatal error

```kiwi
import "log"

cfg = load_config()
if cfg == null
  log::fatal("Could not load configuration — aborting")
  # Process exits here with code 1
end
```
