# time

> The current time, timestamps, elapsed-time measurement and calendar helpers.

The `time` package reads the clock and answers calendar questions.

```kiwi
now = time::now()                         # the current local date and time
println now.to_string("yyyy-MM-dd HH:mm") >= "2024"   # true

println time::timestamp()                 # e.g. 2026-10-19 22:54:13Z (UTC)
println time::timestamp("yyyy-MM-dd")     # e.g. 2026-10-19
```

`timestamp(format)` formats the current UTC time with the [date patterns](../reference/builtin-methods.md#date-formats); the default, `"u"`, is the sortable `yyyy-MM-dd HH:mm:ssZ` form.

## Measuring elapsed time

`ticks()` reads a high-resolution clock. Subtract two readings and convert the difference to milliseconds with `ticksms`:

```kiwi
started = time::ticks()
task::sleep(15)
elapsed = time::ticksms(time::ticks() - started)
println elapsed >= 15   # true
```

For anything more than a quick measurement, [`stopwatch`](stopwatch.md) and [`bench`](bench.md) are more convenient.

## Calendar helpers

```kiwi
println time::leapyear(2024)            # true
println time::monthdays(2024, 2)        # 29
println time::valid_day(2023, 2, 29)    # false
println time::first_day().day()         # 1: the first day of the current month
```

`last_day()` returns the last day of the current month.

## Date parts and arithmetic

`time::year()`, `month()`, `day()`, `hour()`, `minute()`, `second()`, `millisecond()`, `weekday()`, `yearday()`, `ampm()` and `isdst()` read a part of a date, or of the current time when called without one. `add_days`, `add_months` and the other `add_*` functions do date arithmetic.

The same operations are available as methods on any date, which usually reads better:

```kiwi
release = d(2024, 6, 15)
println time::add_days(release, 10).to_string("yyyy-MM-dd")   # 2024-06-25
println release.add_days(10).to_string("yyyy-MM-dd")          # 2024-06-25
```

See [`date`](date.md) and [Types: Dates](../language/types.md#dates).

## Functions

### `time::add_days`

```kiwi
time::add_days(dt: date, n: integer): date
```

Adds a number of days to a date.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `dt` | `date` |  | The date. |
| `n` | `integer` |  | The number of days. |

**Returns** `date` — The modified date.

### `time::add_months`

```kiwi
time::add_months(dt: date, n: integer): date
```

Adds a number of months to a date.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `dt` | `date` |  | The date. |
| `n` | `integer` |  | The number of months. |

**Returns** `date` — The modified date.

### `time::add_years`

```kiwi
time::add_years(dt: date, n: integer): date
```

Adds a number of years to a date.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `dt` | `date` |  | The date. |
| `n` | `integer` |  | The number of years. |

**Returns** `date` — The modified date.

### `time::add_hours`

```kiwi
time::add_hours(dt: date, n: integer): date
```

Adds a number of hours to a date.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `dt` | `date` |  | The date. |
| `n` | `integer` |  | The number of hours. |

**Returns** `date` — The modified date.

### `time::add_minutes`

```kiwi
time::add_minutes(dt: date, n: integer): date
```

Adds a number of minutes to a date.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `dt` | `date` |  | The date. |
| `n` | `integer` |  | The number of minutes. |

**Returns** `date` — The modified date.

### `time::add_seconds`

```kiwi
time::add_seconds(dt: date, n: integer): date
```

Adds a number of seconds to a date.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `dt` | `date` |  |  |
| `n` | `integer` |  |  |

**Returns** `date` — The modified date.

### `time::add_milliseconds`

```kiwi
time::add_milliseconds(dt: date, n: integer): date
```

Adds a number of milliseconds to a date.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `dt` | `date` |  |  |
| `n` | `integer` |  |  |

**Returns** `date` — The modified date.

### `time::now`

```kiwi
time::now(): date
```

Returns the current time as a date value.

**Returns** `date` — The current time as a date value.

### `time::first_day`

```kiwi
time::first_day(): date
```

Returns the first day of the year

**Returns** `date` — The first day of the year

### `time::last_day`

```kiwi
time::last_day(): date
```

Returns the end of the year

**Returns** `date` — The last day of the year

### `time::hour`

```kiwi
time::hour(dt = null): integer
```

Get hours since midnight (0 to 23).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `dt` | `date` | `null` | The date to extract from. Defaults to null. |

**Returns** `integer` — The hours since midnight.

### `time::minute`

```kiwi
time::minute(dt = null): integer
```

Get minutes after the hour (0 to 59).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `dt` | `date` | `null` | The date to extract from. Defaults to null. |

**Returns** `integer` — The minutes after the hour.

### `time::second`

```kiwi
time::second(dt = null): integer
```

Get seconds after the minute (0 to 60).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `dt` | `date` | `null` | The date to extract from. Defaults to null. |

**Returns** `integer` — The seconds after the minute.

### `time::millisecond`

```kiwi
time::millisecond(dt = null): integer
```

Get milliseconds after the second (0 to 1000).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `dt` | `date` | `null` | The date to extract from. Defaults to null. |

**Returns** `integer` — The milliseconds after the second.

### `time::ampm`

```kiwi
time::ampm(dt = null): string
```

Get time of day designation for 12 hour clocks as "AM" or "PM".

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `dt` | `date` | `null` | The date to extract from. Defaults to null. |

**Returns** `string` — "AM" or "PM".

### `time::day`

```kiwi
time::day(dt = null): integer
```

Get the day of the month (1 to 31).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `dt` | `date` | `null` | The date to extract from. Defaults to null. |

**Returns** `integer` — The day of the month.

### `time::month`

```kiwi
time::month(dt = null): integer
```

Get current calendar month (1 to 12).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `dt` | `date` | `null` | The date to extract from. Defaults to null. |

**Returns** `integer` — The current month, or the month of dt.

### `time::year`

```kiwi
time::year(dt = null): integer
```

Get the current year.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `dt` | `date` | `null` | The date to extract from. Defaults to null. |

**Returns** `integer` — The current year.

### `time::leapyear`

```kiwi
time::leapyear(_year: integer): boolean
```

Check if a given year is a leap year.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_year` | `integer` |  | The year to check. |

**Returns** `boolean` — Returns `true` when _year is a leap year.

### `time::monthdays`

```kiwi
time::monthdays(_year: integer, _month: integer): integer
```

Get how many days are in a given month.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_year` | `integer` |  | The year to check. |
| `_month` | `integer` |  | The month to check. |

**Returns** `integer` — The number of days in a given month.

### `time::weekday`

```kiwi
time::weekday(dt = null): integer
```

Get the day of the week (0 to 6).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `dt` | `date` | `null` | The date to check. Defaults to null. |

**Returns** `integer` — The day of the week starting from Sunday = 0.

### `time::yearday`

```kiwi
time::yearday(dt = null): integer
```

Get the day of the year (1 to 366).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `dt` | `date` | `null` | The date to check. Defaults to null. |

**Returns** `integer` — The day of the year starting from Sunday = 0.

### `time::isdst`

```kiwi
time::isdst(dt = null): boolean
```

Checks if currently in Daylight Savings Time (DST).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `dt` | `date` | `null` | The date to check. Defaults to null. |

**Returns** `boolean` — Returns `true` when in DST.

### `time::valid_day`

```kiwi
time::valid_day(_year: integer, _month: integer, _day: integer): boolean
```

Check if a day is valid within the given month.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_year` | `integer` |  | The year to check. |
| `_month` | `integer` |  | The month to check. |
| `_day` | `integer` |  | The day to check. |

**Returns** `boolean` — The number of days in a given month.

### `time::timestamp`

```kiwi
time::timestamp(format: string = 'u'): string
```

Get current timestamp in a specified format.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `format` | `string` | `'u'` |  |

**Returns** `string` — The current timestamp as a string.

### `time::ticks`

```kiwi
time::ticks(): float
```

Gets a high-resolution timestamp in nanoseconds, for measuring elapsed time. It counts from an arbitrary point, not a calendar date; subtract two readings and convert the difference with `ticksms`.

**Returns** `float` — The timestamp in nanoseconds.

### `time::ticksms`

```kiwi
time::ticksms(_ticks: float): float
```

Converts a difference of `ticks()` readings (nanoseconds) to milliseconds.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_ticks` | `float` |  | The ticks. |

**Returns** `float` — Ticks in milliseconds.
