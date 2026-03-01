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

**Example**

```kiwi
import "time"

today    = time::now()
next_week = time::add_days(today, 7)
last_week = time::add_days(today, -7)

println "Today:     ${today.to_string("yyyy-MM-dd")}"
println "Next week: ${next_week.to_string("yyyy-MM-dd")}"
println "Last week: ${last_week.to_string("yyyy-MM-dd")}"
```

---

### `add_months(dt, n)`
Adds/subtracts months (preserves day-of-month when possible).

**Parameters**

| Type    | Name | Description          |
|---------|------|----------------------|
| `date`  | `dt` | Base date            |
| `integer`| `n`  | Months (±)           |

**Returns** `date`

**Example**

```kiwi
import "time"

today       = time::now()
in_3_months = time::add_months(today, 3)
last_quarter = time::add_months(today, -3)

println "Today:        ${today.to_string("yyyy-MM-dd")}"
println "In 3 months:  ${in_3_months.to_string("yyyy-MM-dd")}"
println "Last quarter: ${last_quarter.to_string("yyyy-MM-dd")}"
```

---

### `add_years(dt, n)`
Adds/subtracts whole years.

**Parameters**

| Type    | Name | Description          |
|---------|------|----------------------|
| `date`  | `dt` | Base date            |
| `integer`| `n`  | Years (±)            |

**Returns** `date`

**Example**

```kiwi
import "time"

today     = time::now()
next_year = time::add_years(today, 1)
decade    = time::add_years(today, 10)

println "Today:     ${today.to_string("yyyy-MM-dd")}"
println "Next year: ${next_year.to_string("yyyy-MM-dd")}"
println "A decade:  ${decade.to_string("yyyy-MM-dd")}"
```

---

### `add_hours(dt, n)`, `add_minutes(dt, n)`, `add_seconds(dt, n)`, `add_milliseconds(dt, n)`
Add/subtract smaller time units (may cross day boundaries).

**Returns** `date`

**Example**

```kiwi
import "time"

now = time::now()

println "Now:           ${now.to_string("yyyy-MM-dd HH:mm:ss")}"
println "+3 hours:      ${time::add_hours(now, 3).to_string("yyyy-MM-dd HH:mm:ss")}"
println "+90 minutes:   ${time::add_minutes(now, 90).to_string("yyyy-MM-dd HH:mm:ss")}"
println "+45 seconds:   ${time::add_seconds(now, 45).to_string("yyyy-MM-dd HH:mm:ss")}"
println "+500ms:        ${time::add_milliseconds(now, 500).to_string("yyyy-MM-dd HH:mm:ss")}"
println "-6 hours:      ${time::add_hours(now, -6).to_string("yyyy-MM-dd HH:mm:ss")}"
```

---

## Current time & components

Most component functions accept an optional `date` argument.
If omitted (`dt = null`), they return the value for **now**.

### `now()`
Returns the current date and time as a `date` object.

**Returns** `date`

**Example**

```kiwi
import "time"

dt = time::now()
println "Full timestamp: ${dt.to_string("yyyy-MM-dd HH:mm:ss")}"
println "Year:  ${time::year(dt)}"
println "Month: ${time::month(dt)}"
println "Day:   ${time::day(dt)}"
```

---

### `hour(dt?)`, `minute(dt?)`, `second(dt?)`, `millisecond(dt?)`
Extract time-of-day components.

**Returns** `integer`

- `hour`: 0–23
- `minute`: 0–59
- `second`: 0–60 (leap seconds possible)
- `millisecond`: 0–999

**Example**

```kiwi
import "time"

# Called without argument — operates on the current time
println "Hour:        ${time::hour()}"
println "Minute:      ${time::minute()}"
println "Second:      ${time::second()}"
println "Millisecond: ${time::millisecond()}"

# Can also pass an explicit date
dt = time::now()
println "Hour from dt: ${time::hour(dt)}"
```

---

### `ampm(dt?)`
Returns `"AM"` or `"PM"` (12-hour clock indicator).

**Returns** `string`

**Example**

```kiwi
import "time"

period = time::ampm()
println "It is currently ${time::hour() % 12}:${time::minute()} ${period}"
# => It is currently 2:34 PM  (example)
```

---

### `day(dt?)`, `month(dt?)`, `year(dt?)`
Calendar date components.

**Returns** `integer`

- `day`: 1–31
- `month`: 1–12
- `year`: full year (e.g. 2026)

**Example**

```kiwi
import "time"

println "Year:  ${time::year()}"
println "Month: ${time::month()}"
println "Day:   ${time::day()}"

# Or pass a specific date
dt = time::add_days(time::now(), 30)
println "30 days from now: ${time::year(dt)}-${time::month(dt)}-${time::day(dt)}"
```

---

### `weekday(dt?)`
Day of week (0 = Sunday, 1 = Monday, …, 6 = Saturday).

**Returns** `integer`

**Example**

```kiwi
import "time"

names = ["Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"]
idx   = time::weekday()
println "Today is ${names[idx]} (index ${idx})"

# Check if today is a weekend
is_weekend = idx == 0 || idx == 6
println "Weekend: ${is_weekend}"
```

---

### `yearday(dt?)`
Day of year (1 = Jan 1, up to 365 or 366).

**Returns** `integer`

**Example**

```kiwi
import "time"

yd = time::yearday()
println "Day of year: ${yd}"

# Days remaining in the year
yr   = time::year()
days = case time::leapyear(yr) | true => 366, false => 365 end
println "Days remaining: ${days - yd}"
```

---

## Calendar utilities

### `leapyear(_year)`
Checks whether a year is a leap year (Gregorian rules).

**Parameters**

| Type     | Name     |
|----------|----------|
| `integer`| `_year`  |

**Returns** `boolean`

**Example**

```kiwi
import "time"

println "2024 leap year: ${time::leapyear(2024)}"  # => true
println "2023 leap year: ${time::leapyear(2023)}"  # => false
println "2000 leap year: ${time::leapyear(2000)}"  # => true
println "1900 leap year: ${time::leapyear(1900)}"  # => false
```

---

### `monthdays(_year, _month)`
Returns number of days in the specified month/year.

**Parameters**

| Type     | Name      |
|----------|-----------|
| `integer`| `_year`   |
| `integer`| `_month`  |

**Returns** `integer` (28–31)

**Example**

```kiwi
import "time"

# February in a leap year vs. a regular year
println "Feb 2024: ${time::monthdays(2024, 2)} days"  # => 29
println "Feb 2023: ${time::monthdays(2023, 2)} days"  # => 28
println "Jan 2026: ${time::monthdays(2026, 1)} days"  # => 31
println "Apr 2026: ${time::monthdays(2026, 4)} days"  # => 30
```

---

### `valid_day(_year, _month, _day)`
Checks if the day is valid in the given month/year.

**Parameters**

| Type     | Name      |
|----------|-----------|
| `integer`| `_year`   |
| `integer`| `_month`  |
| `integer`| `_day`    |

**Returns** `boolean`

**Example**

```kiwi
import "time"

println "2026-03-15 valid: ${time::valid_day(2026, 3, 15)}"  # => true
println "2026-02-29 valid: ${time::valid_day(2026, 2, 29)}"  # => false (2026 not a leap year)
println "2024-02-29 valid: ${time::valid_day(2024, 2, 29)}"  # => true  (2024 is a leap year)
println "2026-13-01 valid: ${time::valid_day(2026, 13, 1)}"  # => false (no 13th month)
```

---

### `first_day()`
Returns January 1st of the current year.

**Returns** `date`

### `last_day()`
Returns December 31st of the current year.

**Returns** `date`

**Example**

```kiwi
import "time"

first = time::first_day()
last  = time::last_day()

println "First day of year: ${first.to_string("yyyy-MM-dd")}"
println "Last day of year:  ${last.to_string("yyyy-MM-dd")}"

# Iterate every day of the year
dt = time::add_days(first, -1)
repeat 365 do
  dt = time::add_days(dt, 1)
  # process dt ...
end
```

---

## Other helpers

### `isdst(dt?)`
Checks whether the time is in Daylight Saving Time.

**Returns** `boolean`

**Example**

```kiwi
import "time"

in_dst = time::isdst()
println "Currently in DST: ${in_dst}"

# Check a specific date
summer = time::add_months(time::now(), 3)
println "DST in 3 months: ${time::isdst(summer)}"
```

---

### `timestamp(format?)`
Formats the current time using the `date` type's `to_string()` rules.

**Parameters**

| Type    | Name     | Default |
|---------|----------|---------|
| `string`| `format` | `'u'`   |

**Returns** `string`

Common formats include `'u'` (ISO-like), `'yyyy-MM-dd'`, `'HH:mm:ss'`, etc.

**Example**

```kiwi
import "time"

println time::timestamp()               # ISO-like default
println time::timestamp("yyyy-MM-dd")   # date only
println time::timestamp("HH:mm:ss")     # time only
println time::timestamp("yyyy-MM-dd HH:mm:ss")  # datetime

# Useful for log prefixes
fn log(msg: string)
  println "[${time::timestamp("HH:mm:ss")}] ${msg}"
end

log("Starting process")
log("Process complete")
```

---

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

**Example**

```kiwi
import "time"

# Time a block of code using ticks
fn expensive_work()
  total = 0
  for i in [1 to 100000] do
    total += i
  end
  total
end

t0     = time::ticks()
result = expensive_work()
t1     = time::ticks()

elapsed_ms = time::ticksms(t1 - t0)
println "Result:  ${result}"
println "Elapsed: ${elapsed_ms}ms"

# Ticks are also useful for simple wall-clock comparisons
start = time::ticks()
# ... do work ...
end_t  = time::ticks()
println "Wall time: ${time::ticksms(end_t - start)}ms"
```
