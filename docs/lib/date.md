# `date`

The `date` package extends the built-in `date` type with method-style arithmetic and component accessors.

After this package is loaded (it is part of the standard library and auto-imported), every `date` value gains these methods directly — no package prefix needed.

```kiwi
today = time::now()
println today.add_days(7)    # instead of time::add_days(today, 7)
println today.year()         # instead of time::year(today)
```

All arithmetic methods return a **new** `date` — the original is never mutated. Use a negative `n` to subtract.

---

## Arithmetic

### `add_days(n)`

**Parameters**

| Type      | Name | Description                     | Default |
|-----------|------|---------------------------------|---------|
| `integer` | `n`  | Days to add (negative subtracts) | —      |

**Returns** `date`

**Example**
```kiwi
today     = time::now()
next_week = today.add_days(7)
last_week = today.add_days(-7)
```

---

### `add_months(n)`

Preserves day-of-month where possible (e.g. Jan 31 + 1 month → Feb 28/29).

**Parameters**

| Type      | Name | Description                       | Default |
|-----------|------|-----------------------------------|---------|
| `integer` | `n`  | Months to add (negative subtracts) | —      |

**Returns** `date`

**Example**
```kiwi
today       = time::now()
next_quarter = today.add_months(3)
last_month   = today.add_months(-1)
```

---

### `add_years(n)`

**Parameters**

| Type      | Name | Description                      | Default |
|-----------|------|----------------------------------|---------|
| `integer` | `n`  | Years to add (negative subtracts) | —      |

**Returns** `date`

**Example**
```kiwi
today     = time::now()
next_year = today.add_years(1)
last_year = today.add_years(-1)
```

---

### `add_hours(n)`

May cross day boundaries.

**Parameters**

| Type      | Name | Description                      | Default |
|-----------|------|----------------------------------|---------|
| `integer` | `n`  | Hours to add (negative subtracts) | —      |

**Returns** `date`

---

### `add_minutes(n)`

**Parameters**

| Type      | Name | Description                        | Default |
|-----------|------|------------------------------------|---------|
| `integer` | `n`  | Minutes to add (negative subtracts) | —      |

**Returns** `date`

---

### `add_seconds(n)`

**Parameters**

| Type      | Name | Description                        | Default |
|-----------|------|------------------------------------|---------|
| `integer` | `n`  | Seconds to add (negative subtracts) | —      |

**Returns** `date`

---

### `add_milliseconds(n)`

**Parameters**

| Type      | Name | Description                              | Default |
|-----------|------|------------------------------------------|---------|
| `integer` | `n`  | Milliseconds to add (negative subtracts) | —      |

**Returns** `date`

---

## Component accessors

All of these extract a single field from the date value.

| Method            | Returns   | Description                              |
|-------------------|-----------|------------------------------------------|
| `year()`          | `integer` | Calendar year (e.g. `2024`)              |
| `month()`         | `integer` | Month of year, 1–12                      |
| `day()`           | `integer` | Day of month, 1–31                       |
| `hour()`          | `integer` | Hour of day, 0–23                        |
| `minute()`        | `integer` | Minute of hour, 0–59                     |
| `second()`        | `integer` | Second of minute, 0–60                   |
| `millisecond()`   | `integer` | Millisecond of second, 0–999             |
| `weekday()`       | `integer` | Day of week: 0 = Sunday … 6 = Saturday   |
| `yearday()`       | `integer` | Day of year, 1–366                       |
| `ampm()`          | `string`  | `"AM"` or `"PM"`                         |
| `isdst()`         | `boolean` | `true` if Daylight Saving Time is active |

**Example**
```kiwi
dt = d(2024, 3, 15, 10, 30, 45)

println dt.year()       # 2024
println dt.month()      # 3
println dt.day()        # 15
println dt.hour()       # 10
println dt.minute()     # 30
println dt.second()     # 45
println dt.weekday()    # 5  (Friday)
println dt.yearday()    # 75
println dt.ampm()       # "AM"
```

---

## Examples

### Arithmetic and formatting

```kiwi
today = time::now()

println today.to_string("yyyy-MM-dd")
println today.add_days(30).to_string("yyyy-MM-dd")
println today.add_months(-6).to_string("yyyy-MM-dd")
```

### Chaining

Methods chain naturally since each returns a new `date`:

```kiwi
result = time::now()
           .add_years(1)
           .add_months(-3)
           .add_days(10)

println result.to_string("yyyy-MM-dd")
```

### Building a date range

```kiwi
start = d(2024, 1, 1)
dates = []

for i in [0..6] do
  dates.push(start.add_days(i).to_string("yyyy-MM-dd"))
end

println dates
# ["2024-01-01", "2024-01-02", ..., "2024-01-07"]
```

### Days until an event

```kiwi
today  = time::now()
event  = d(2024, 12, 31)

days = 0
cur  = today
while cur.year() < event.year() || cur.month() < event.month() || cur.day() < event.day() do
  cur  = cur.add_days(1)
  days += 1
end

println "Days until event: ${days}"
```
