# `time`

The `time` package provides comprehensive support for working with dates, times, durations, and calendar calculations in Kiwi.

It includes:
- A `date` type (built-in) with parsing, formatting, and arithmetic
- Functions for adding/subtracting time units
- Component extraction (year, month, day, hour, etc.)
- Calendar utilities (leap years, month lengths, day of week/year)
- Current time / timestamp helpers
- Low-level tick-based timing

## Date arithmetic

All `add_*` functions return a **new** `date` object — they do **not** mutate the input.

### `add_days(dt, n)`
Adds/subtracts whole days (handles month/year rollover).

**Parameters**  

| Type    | Name | Description          |
|---------|------|----------------------|
| `date`  | `dt` | Base date            |
| `integer`| `n`  | Days (±)             |

**Returns** `date`

### `add_months(dt, n)`
Adds/subtracts months (preserves day-of-month when possible).

**Parameters**  

| Type    | Name | Description          |
|---------|------|----------------------|
| `date`  | `dt` | Base date            |
| `integer`| `n`  | Months (±)           |

**Returns** `date`

### `add_years(dt, n)`
Adds/subtracts whole years.

**Parameters**

| Type    | Name | Description          |
|---------|------|----------------------|
| `date`  | `dt` | Base date            |
| `integer`| `n`  | Years (±)            |

**Returns** `date`

### `add_hours(dt, n)`, `add_minutes(dt, n)`, `add_seconds(dt, n)`, `add_milliseconds(dt, n)`
Add/subtract smaller time units (may cross day boundaries).

**Returns** `date`

## Current time & components

Most component functions accept an optional `date` argument.  
If omitted (`dt = null`), they return the value for **now**.

### `now()`
Returns the current date and time as a `date` object.

**Returns** `date`

### `hour(dt?)`, `minute(dt?)`, `second(dt?)`, `millisecond(dt?)`
Extract time-of-day components.

**Returns** `integer`

- `hour`: 0–23
- `minute`: 0–59
- `second`: 0–60 (leap seconds possible)
- `millisecond`: 0–999

### `ampm(dt?)`
Returns `"AM"` or `"PM"` (12-hour clock indicator).

**Returns** `string`

### `day(dt?)`, `month(dt?)`, `year(dt?)`
Calendar date components.

**Returns** `integer`

- `day`: 1–31
- `month`: 1–12
- `year`: full year (e.g. 2026)

### `weekday(dt?)`
Day of week (0 = Sunday, 1 = Monday, …, 6 = Saturday).

**Returns** `integer`

### `yearday(dt?)`
Day of year (1 = Jan 1, up to 365 or 366).

**Returns** `integer`

## Calendar utilities

### `leapyear(_year)`
Checks whether a year is a leap year (Gregorian rules).

**Parameters**

| Type     | Name     |
|----------|----------|
| `integer`| `_year`  |

**Returns** `boolean`

### `monthdays(_year, _month)`
Returns number of days in the specified month/year.

**Parameters**

| Type     | Name      |
|----------|-----------|
| `integer`| `_year`   |
| `integer`| `_month`  |

**Returns** `integer` (28–31)

### `valid_day(_year, _month, _day)`
Checks if the day is valid in the given month/year.

**Parameters**

| Type     | Name      |
|----------|-----------|
| `integer`| `_year`   |
| `integer`| `_month`  |
| `integer`| `_day`    |

**Returns** `boolean`

### `first_day()`
Returns January 1st of the current year.

**Returns** `date`

### `last_day()`
Returns December 31st of the current year.

**Returns** `date`

## Other helpers

### `isdst(dt?)`
Checks whether the time is in Daylight Saving Time.

**Returns** `boolean`

### `timestamp(format?)`
Formats the current time using the `date` type’s `to_string()` rules.

**Parameters**

| Type    | Name     | Default |
|---------|----------|---------|
| `string`| `format` | `'u'`   |

**Returns** `string`

Common formats include `'u'` (ISO-like), `'yyyy-MM-dd'`, `'HH:mm:ss'`, etc.

### `ticks()`
Nanoseconds since Unix epoch (1970-01-01 00:00:00 UTC).

**Returns** `float`

### `ticksms(_ticks)`
Converts ticks (nanoseconds) to milliseconds.

**Parameters**

| Type   | Name      |
|--------|-----------|
| `float`| `_ticks`  |

**Returns** `float`
