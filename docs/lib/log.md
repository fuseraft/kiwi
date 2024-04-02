# `@astral/log`

The `log` module contains functionality for working with the Astral logger.

## Table of Contents

- [Importing the Module](#importing-the-module)
- [Module Functions](#module-functions)
  - [`get()`](#get)
  - [`opt(_key)`](#opt_key)

## Importing the Module

To use the `log` module, import it at the beginning of your Astral script.

```ruby
import "@astral/log" as logger
```

## Module Functions

### `set_file(_file_path)`

Set the log file path.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_file_path` | The log file path. |

### `set_mode(_mode = "CONSOLE")`

Sets the log mode.

Currently available modes:
- `FILE`
- `CONSOLE`

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_mode` | The log mode. Defaults to `CONSOLE`. |

### `logformat(_format)`

Sets the log entry format.

Currently available format specifiers:
- `%timestamp`: The current timestamp.
- `%level`: The log level: WARN, DEBUG, INFO, ERROR
- `%source`: The log source.
- `%message`: The log message.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_format` | The log entry format. |

### `tsformat(_format)`

Sets the log timestamp format.

Currently available format specifiers:
- `%Y`: Year with century as a decimal number (e.g., `2023`).
- `%m`: Month as a zero-padded decimal number (e.g., `01` to `12`).
- `%d`: Day of the month as a zero-padded decimal number (e.g., `01` to `31`).
- `%H`: Hour (24-hour clock) as a zero-padded decimal number (e.g., `00` to `23`).
- `%M`: Minute as a zero-padded decimal number (e.g., `00` to `59`).
- `%S`: Second as a zero-padded decimal number (e.g., `00` to `60`).

Additional specifiers:
- `%y`: Year without century as a zero-padded decimal number (e.g., `00` to `99`).
- `%b` or `%h`: Abbreviated month name (e.g., `Jan`, `Feb`, `Mar`, ...).
- `%B`: Full month name (e.g., `January`, `February`, `March`, ...).
- `%a`: Abbreviated weekday name (e.g., `Sun`, `Mon`, `Tue`, ...).
- `%A`: Full weekday name (e.g., `Sunday`, `Monday`, `Tuesday`, ...).
- `%I`: Hour (12-hour clock) as a zero-padded decimal number (e.g., `01` to `12`).
- `%p`: Locale's equivalent of either AM or PM.
- `%j`: Day of the year as a zero-padded decimal number (e.g., `001` to `366`).
- `%U`: Week number of the year (Sunday as the first day of the week) as a zero-padded decimal number (e.g., `00` to `53`).
- `%W`: Week number of the year (Monday as the first day of the week) as a zero-padded decimal number (e.g., `00` to `53`).
- `%c`: Locale's appropriate date and time representation.
- `%x`: Locale's appropriate date representation.
- `%X`: Locale's appropriate time representation.
- `%Z`: Time zone name or abbreviation (no characters if no time zone exists).
- `%z`: UTC offset in the form `±HHMM` (e.g., `-0400`, `+0230`).

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_format` | The log entry format. |

### `minlevel(_level = "INFO")`

Sets the minimum log level. Defaults to `INFO`.

Currently log levels:
- `SILENT`: No log entries will be written.
- `DEBUG`: For debugging.
- `WARNING`: For warnings.
- `INFO`: For informational messages.
- `ERROR`: For errors.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_level` | The log level. Defaults to `INFO`. |

### `debug(_message, _source = "")`

Writes a log entry at the DEBUG level.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_message` | The message. |
| `String` | `_source` | The source. Optional. |


### `warn(_message, _source = "")`

Writes a log entry at the WARNING level.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_message` | The message. |
| `String` | `_source` | The source. Optional. |


### `info(_message, _source = "")`

Writes a log entry at the INFO level.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_message` | The message. |
| `String` | `_source` | The source. Optional. |

### `error(_message, _source = "")`

Writes a log entry at the ERROR level.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_message` | The message. |
| `String` | `_source` | The source. Optional. |