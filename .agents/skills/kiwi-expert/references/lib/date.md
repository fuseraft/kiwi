# date

> Date arithmetic and component methods for the date type.

The `date` package adds methods to every `date` value: arithmetic such as `.add_days(n)` and accessors such as `.weekday()`. Because the package [extends the built-in type](../language/packages.md#extending-built-in-types), you call these as methods; each is also available as `date::name(value, ...)`.

```kiwi
release = d(2024, 3, 15, 10, 30, 45)

println release.add_days(30).to_string("yyyy-MM-dd")    # 2024-04-14
println release.add_months(-6).to_string("yyyy-MM-dd")  # 2023-09-15
println release.weekday()                               # 5 (Friday)
println release.yearday()                               # 75
println release.ampm()                                  # AM
```

## Arithmetic

`add_years`, `add_months`, `add_days`, `add_hours`, `add_minutes`, `add_seconds` and `add_milliseconds` each return a new date; the original is unchanged. Pass a negative number to go back in time. Calls chain:

```kiwi
due = d(2024, 1, 31).add_months(1).add_days(-1)
println due.to_string("yyyy-MM-dd")   # 2024-02-28
```

Adding months clamps to the end of shorter months, so January 31 plus one month is February 29 in a leap year.

## Components

| Method | Returns |
| :-- | :-- |
| `year()`, `month()`, `day()` | Calendar parts; months run 1 to 12 |
| `hour()`, `minute()`, `second()`, `millisecond()` | Time-of-day parts |
| `weekday()` | Day of the week, 0 (Sunday) to 6 (Saturday) |
| `yearday()` | Day of the year, 1 to 366 |
| `ampm()` | `"AM"` or `"PM"` |
| `isdst()` | Whether daylight saving time is in effect |

## Examples

A week of day names starting from a date:

```kiwi
week = [0 to 6].map(do (i) => d(2024, 1, 1).add_days(i).to_string("ddd"))
println week   # ["Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"]
```

Formatting uses the patterns listed under [Date formats](../reference/builtin-methods.md#date-formats). For the current time, timestamps and calendar helpers, see [`time`](time.md).

## Functions

### `date::add_days`

```kiwi
date::add_days(_dt: date, n: integer): date
```

Adds `n` days to the date. Use a negative value to subtract.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_dt` | `date` |  | The date. |
| `n` | `integer` |  | Number of days to add (negative to subtract). |

**Returns** `date` — The new date.

### `date::add_months`

```kiwi
date::add_months(_dt: date, n: integer): date
```

Adds `n` months to the date. Use a negative value to subtract.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_dt` | `date` |  | The date. |
| `n` | `integer` |  | Number of months to add (negative to subtract). |

**Returns** `date` — The new date.

### `date::add_years`

```kiwi
date::add_years(_dt: date, n: integer): date
```

Adds `n` years to the date. Use a negative value to subtract.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_dt` | `date` |  | The date. |
| `n` | `integer` |  | Number of years to add (negative to subtract). |

**Returns** `date` — The new date.

### `date::add_hours`

```kiwi
date::add_hours(_dt: date, n: integer): date
```

Adds `n` hours to the date. Use a negative value to subtract.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_dt` | `date` |  | The date. |
| `n` | `integer` |  | Number of hours to add (negative to subtract). |

**Returns** `date` — The new date.

### `date::add_minutes`

```kiwi
date::add_minutes(_dt: date, n: integer): date
```

Adds `n` minutes to the date. Use a negative value to subtract.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_dt` | `date` |  | The date. |
| `n` | `integer` |  | Number of minutes to add (negative to subtract). |

**Returns** `date` — The new date.

### `date::add_seconds`

```kiwi
date::add_seconds(_dt: date, n: integer): date
```

Adds `n` seconds to the date. Use a negative value to subtract.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_dt` | `date` |  | The date. |
| `n` | `integer` |  | Number of seconds to add (negative to subtract). |

**Returns** `date` — The new date.

### `date::add_milliseconds`

```kiwi
date::add_milliseconds(_dt: date, n: integer): date
```

Adds `n` milliseconds to the date. Use a negative value to subtract.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_dt` | `date` |  | The date. |
| `n` | `integer` |  | Number of milliseconds to add (negative to subtract). |

**Returns** `date` — The new date.

### `date::year`

```kiwi
date::year(_dt: date): integer
```

Returns the year component of the date.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_dt` | `date` |  | The date. |

**Returns** `integer` — The year.

### `date::month`

```kiwi
date::month(_dt: date): integer
```

Returns the month component of the date (1–12).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_dt` | `date` |  | The date. |

**Returns** `integer` — The month.

### `date::day`

```kiwi
date::day(_dt: date): integer
```

Returns the day-of-month component of the date (1–31).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_dt` | `date` |  | The date. |

**Returns** `integer` — The day.

### `date::hour`

```kiwi
date::hour(_dt: date): integer
```

Returns the hour component of the date (0–23).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_dt` | `date` |  | The date. |

**Returns** `integer` — The hour.

### `date::minute`

```kiwi
date::minute(_dt: date): integer
```

Returns the minute component of the date (0–59).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_dt` | `date` |  | The date. |

**Returns** `integer` — The minute.

### `date::second`

```kiwi
date::second(_dt: date): integer
```

Returns the second component of the date (0–60).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_dt` | `date` |  | The date. |

**Returns** `integer` — The second.

### `date::millisecond`

```kiwi
date::millisecond(_dt: date): integer
```

Returns the millisecond component of the date (0–999).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_dt` | `date` |  | The date. |

**Returns** `integer` — The millisecond.

### `date::weekday`

```kiwi
date::weekday(_dt: date): integer
```

Returns the day of the week (0 = Sunday … 6 = Saturday).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_dt` | `date` |  | The date. |

**Returns** `integer` — The weekday index.

### `date::yearday`

```kiwi
date::yearday(_dt: date): integer
```

Returns the day of the year (1–366).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_dt` | `date` |  | The date. |

**Returns** `integer` — The day of the year.

### `date::isdst`

```kiwi
date::isdst(_dt: date): boolean
```

Returns `true` if the date falls within Daylight Saving Time.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_dt` | `date` |  | The date. |

**Returns** `boolean` — `true` if DST is in effect.

### `date::ampm`

```kiwi
date::ampm(_dt: date): string
```

Returns `"AM"` or `"PM"` for the date's time.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_dt` | `date` |  | The date. |

**Returns** `string` — `"AM"` or `"PM"`.
