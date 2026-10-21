# log

> Leveled logging to stderr and, optionally, a file.

The `log` package writes leveled log messages to stderr, and optionally to a file as well. Messages below the current level are dropped.

```kiwi
log::info("service starting")
log::debug("hidden: the default level is INFO")

log::set_level("DEBUG")
log::debug("now visible")
log::warn("disk usage above 80%")
log::error("could not reach database")
```

```text
[INFO] [3/22/2026 6:00:00 PM] service starting
[DEBUG] [3/22/2026 6:00:00 PM] now visible
[WARN] [3/22/2026 6:00:00 PM] disk usage above 80%
[ERROR] [3/22/2026 6:00:00 PM] could not reach database
```

## Levels

From least to most severe: `DEBUG`, `INFO`, `WARN`, `ERROR`, `FATAL`. `set_level` shows messages at that level and above; the default is `INFO`.

`log::fatal(message)` logs the message and then ends the program with exit code `1`.

## Options

```kiwi
log::set_file("/var/log/myapp.log")   # also append every message to this file
log::timestamps(false)                # drop the timestamp: "[INFO] message"
```

## Functions

### `log::set_level`

```kiwi
log::set_level(lv)
```

Sets the minimum log level. Messages below this level are suppressed.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `lv` | `any` |  |  |

### `log::set_file`

```kiwi
log::set_file(path)
```

Sets a file path to append log output to.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path` | `string` |  | File path, or null to disable file output. |

### `log::timestamps`

```kiwi
log::timestamps(enabled)
```

Enables or disables timestamps in log output.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `enabled` | `boolean` |  | Whether to include timestamps. |

### `log::debug`

```kiwi
log::debug(msg)
```

Logs a DEBUG message.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `any` |  | The message to log. |

### `log::info`

```kiwi
log::info(msg)
```

Logs an INFO message.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `any` |  | The message to log. |

### `log::warn`

```kiwi
log::warn(msg)
```

Logs a WARN message.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `any` |  | The message to log. |

### `log::error`

```kiwi
log::error(msg)
```

Logs an ERROR message.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `any` |  | The message to log. |

### `log::fatal`

```kiwi
log::fatal(msg)
```

Logs a FATAL message and exits with code 1.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `any` |  | The message to log. |
