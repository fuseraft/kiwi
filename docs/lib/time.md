# `time`

The `time` package contains functionality for working with time and date information.

## Table of Contents

- [Package Functions](#package-functions)
  - [`ampm()`](#ampm)
  - [`day()`](#day)
  - [`hour()`](#hour)
  - [`isdst()`](#isdst)
  - [`leapyear(_year)`](#leapyear_year)
  - [`monthdays(_year, _month)`](#monthdays_year-_month)
  - [`millisecond()`](#millisecond)
  - [`minute()`](#minute)
  - [`month()`](#month)
  - [`second()`](#second)
  - [`ticks()`](#ticks)
  - [`ticksms(_ticks)`](#ticksms_ticks)
  - [`timestamp()`](#timestamp)
  - [`weekday()`](#weekday)
  - [`year()`](#year)
  - [`yearday()`](#yearday)

## Package Functions

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

### `millisecond()`

Retrieves the current millisecond of the second (0 to 1000).

**Returns**
| Type | Description |
| :--- | :---|
| `Integer` | The current millisecond. |

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
| `String` | The current timestamp formatted as "YYYY/mm/dd hh:mm:ss tt". |

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

### `ticks()`

Retrieves the number of nanoseconds since January 1st, 1970.

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The nanoseconds since the epoch. |

### `ticksms(_ticks)`

Converts ticks to milliseconds.

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The ticks in milliseconds. |
