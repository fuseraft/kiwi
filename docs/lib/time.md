# `@astral/time`

The `time` module contains functionality for working with time and date information.

## Table of Contents

- [Importing the Module](#importing-the-module)
- [Module Functions](#module-functions)
  - [`ampm()`](#ampm)
  - [`day()`](#day)
  - [`delay(_ms)`](#delay_ms)
  - [`epochms()`](#epochms)
  - [`hour()`](#hour)
  - [`isdst()`](#isdst)
  - [`minute()`](#minute)
  - [`month()`](#month)
  - [`second()`](#second)
  - [`ticks()`](#ticks)
  - [`ticksms(_ticks)`](#ticksms_ticks)
  - [`timestamp()`](#timestamp)
  - [`weekday()`](#weekday)
  - [`year()`](#year)
  - [`yearday()`](#yearday)

## Importing the Module
To use the `time` module, import it at the beginning of your Astral script.

```ruby
import "@astral/time" as time
```

## Module Functions

### `hour()`

Retrieves the current hour of the day (0 to 23).

**Returns**
| Type | Description |
| :--- | :---|
| `Integer` | The current hour. |

### `minute()`

Retrieves the current minute of the hour (0 to 59).

**Returns**
| Type | Description |
| :--- | :---|
| `Integer` | The current minute. |

### `second()`

Retrieves the current second of the minute (0 to 60).

**Returns**
| Type | Description |
| :--- | :---|
| `Integer` | The current second. |

### `ampm()`

Retrieves the time of day designation for 12-hour clocks as "AM" or "PM".

**Returns**
| Type | Description |
| :--- | :---|
| `String` | "AM" or "PM". |

### `timestamp()`

Retrieves the current timestamp formatted as "YYYY/mm/dd hh:mm:ss tt".

**Returns**
| Type | Description |
| :--- | :---|
| `Integer` | The current timestamp formatted as "YYYY/mmdd hh:mm:ss tt". |

### `day()`

Retrieves the day of the month (1 to 31).

**Returns**
| Type | Description |
| :--- | :---|
| `Integer` | The current day of the month. |

### `month()`

Retrieves the current month of the year (1 to 12).

**Returns**
| Type | Description |
| :--- | :---|
| `Integer` | The current month. |

### `year()`

Retrieves the current year.

**Returns**
| Type | Description |
| :--- | :---|
| `Integer` | The current year. |

### `weekday()`

Retrieves the day of the week (1 to 7, where 1 is Sunday and 7 is Saturday).

**Returns**
| Type | Description |
| :--- | :---|
| `Integer` | The day of the week. |

### `yearday()`

Retrieves the day of the year (0 to 365). December 31st is 364 on leap years.

**Returns**
| Type | Description |
| :--- | :---|
| `Integer` | The day of the year. |

### `isdst()`

Checks whether it is currently Daylight Savings Time.

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | Indicates DST status. |

### `epochms()`

Retrieves the number of milliseconds since January 1st, 1970.

**Returns**
| Type | Description |
| :--- | :---|
| `Double` | The milliseconds since the epoch. |

### `ticks()`

Retrieves the number of nanoseconds since January 1st, 1970.

**Returns**
| Type | Description |
| :--- | :---|
| `Double` | The nanoseconds since the epoch. |

### `ticksms(_ticks)`

Converts ticks to milliseconds.

**Returns**
| Type | Description |
| :--- | :---|
| `Double` | The ticks in milliseconds. |

### `delay(_ms)`

Pauses execution for a specified duration in milliseconds.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `Integer` | `_ms` | The duration to sleep in milliseconds. |

**Returns**
| Type | Description |
| :--- | :---|
| `Double` | The duration slept in milliseconds. |
