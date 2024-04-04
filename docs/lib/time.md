# `@astral/time`

The `time` module contains functionality for working with time and date information.

## Table of Contents

- [Importing the Module](#importing-the-module)
- [Module Classes](#module-classes)
  - [`DateTime`](#datetime)
- [Module Functions](#module-functions)
  - [`ampm()`](#ampm)
  - [`day()`](#day)
  - [`delay(_ms)`](#delay_ms)
  - [`epochms()`](#epochms)
  - [`hour()`](#hour)
  - [`isdst()`](#isdst)
  - [`leapyear(_year)`](#leapyear_year)
  - [`monthdays(_year, _month)`](#monthdays_year-_month)
  - [`minute()`](#minute)
  - [`month()`](#month)
  - [`second()`](#second)
  - [`ticks()`](#ticks)
  - [`ticksms(_ticks)`](#ticksms_ticks)
  - [`timestamp()`](#timestamp)
  - [`formatted(_format)`](#timestamp_formatted_format)
  - [`dtformat(_datetime, _format)`](#dtformat_datetime-_format)
  - [`weekday()`](#weekday)
  - [`year()`](#year)
  - [`yearday()`](#yearday)

## Importing the Module
To use the `time` module, import it at the beginning of your Astral script.

```ruby
import "@astral/time" as time
```

## Module Classes

### `DateTime`

The `DateTime` class is still a WIP, but functional. It offers some convenient functionality for working with date and time data in Astral.

**Methods**
| Type | Returns | Method | Description |
| :--- | :--- | :--- | :--- |
| Constructor | `DateTime` | `initialize(year, month, day, hour, minute, second)` | Initialize a new `DateTime`. Defaults to the epoch. |
| Static | `String` | `format(_datetime, _format = "%Y-%m-%d %H:%M:%S")` | Format the value with a given format string. |
| Static | `DateTime` | `now()` | Returns the current date and time as a `DateTime` object. |
| Instance | `String` | `to_string()` | Returns a string representation of the `DateTime` object. |
| instance | `List` | `to_number()` | Returns the date and time parts as a list of integers. |
| instance | `Boolean` | `equal(another_date)` | Returns `true` if the values are equal. |
| instance | `Boolean` | `after(another_date)` | Returns `true` if the value is greater than another `DateTime`. |
| instance | `Boolean` | `before(another_date)` | Returns `true` if the value is less than another `DateTime`. |
| instance | `DateTime` | `add_days(n)` | Adds `n` number of days. |
| instance | `DateTime` | `add_months(n)` | Adds `n` number of months. |
| instance | `DateTime` | `add_years(n)` | Adds `n` number of years. |
| instance | `Integer` | `get_year()` | Gets the year. |
| instance | `Integer` | `get_month()` | Gets the month. |
| instance | `Integer` | `get_day()` | Gets the day. |
| instance | `Integer` | `get_hour()` | Gets the hour. |
| instance | `Integer` | `get_minute()` | Gets the minute. |
| instance | `Integer` | `get_second()` | Gets the second. |

**Example**
```ruby
import "@astral/time"

println DateTime.format(DateTime.now().add_years(1), "%Y-%m-%d")
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

### `dtformat(_datetime, _format)`

Format a `DateTime` object as a string using a given format.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `DateTime` | `_datetime` | The `DateTime` object. |
| `String` | `_format` | The format string. |

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The formatted value from a `DateTime` object. |


### `timestamp()`

Retrieves the current timestamp formatted as "YYYY/mm/dd hh:mm:ss tt".

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The current timestamp formatted as "YYYY/mmdd hh:mm:ss tt". |

### `timestamp_formatted(_format)`

Get current timestamp with a specific format.

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
| `String` | `_format` | The format. |

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The current timestamp formatted as specified. |


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

### `leapyear(_year)`

Check if a given year is a leap year.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `Integer` | `_year` | The year to check. |

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | Returns `true` if the given year is a leap year. |


### `monthdays(_year, _month)`

Get the number of days in a given month. Requires the year for leap year checks.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `Integer` | `_year` | The year to check. |
| `Integer` | `_month` | The month to check. |

**Returns**
| Type | Description |
| :--- | :---|
| `Integer` | Returns the number of days in a given month. |

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
