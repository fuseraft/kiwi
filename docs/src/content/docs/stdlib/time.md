---
title: time
description: The current time, timestamps, elapsed-time measurement and calendar helpers.
---

The `time` package reads the clock and answers calendar questions.

```kiwi
now = time::now()                         # the current local date and time
println now.to_string("yyyy-MM-dd HH:mm") >= "2024"   # true

println time::timestamp()                 # e.g. 2026-10-19 22:54:13Z (UTC)
println time::timestamp("yyyy-MM-dd")     # e.g. 2026-10-19
```

`timestamp(format)` formats the current UTC time with the [date patterns](/kiwi/reference/builtin-methods/#date-formats); the default, `"u"`, is the sortable `yyyy-MM-dd HH:mm:ssZ` form.

## Measuring elapsed time

`ticks()` reads a high-resolution clock. Subtract two readings and convert the difference to milliseconds with `ticksms`:

```kiwi
started = time::ticks()
task::sleep(15)
elapsed = time::ticksms(time::ticks() - started)
println elapsed >= 15   # true
```

For anything more than a quick measurement, [`stopwatch`](/kiwi/stdlib/stopwatch/) and [`bench`](/kiwi/stdlib/bench/) are more convenient.

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

See [`date`](/kiwi/stdlib/date/) and [Types: Dates](/kiwi/language/types/#dates).

<!-- api: time -->
