---
title: date
description: Date arithmetic and component methods for the date type.
---

The `date` package adds methods to every `date` value: arithmetic such as `.add_days(n)` and accessors such as `.weekday()`. Because the package [extends the built-in type](/kiwi/language/packages/#extending-built-in-types), you call these as methods; each is also available as `date::name(value, ...)`.

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

Formatting uses the patterns listed under [Date formats](/kiwi/reference/builtin-methods/#date-formats). For the current time, timestamps and calendar helpers, see [`time`](/kiwi/stdlib/time/).

<!-- api: date -->
