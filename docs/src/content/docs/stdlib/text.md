---
title: text
description: Line-oriented text processing pipelines.
---

The `text` package processes text line by line, like a small `grep`/`awk` pipeline. Add a source, chain steps, and call `run()` to get the resulting lines.

```kiwi
log = "time level message
10:01 INFO started
10:02 ERROR disk full

10:03 WARN slow request
10:04 ERROR timeout"

errors = text::pipeline()
  .from_string(log)
  .skip(1)                                       # drop the header
  .filter(do (line) => line.contains("ERROR") end)
  .run()

println errors   # ["10:02 ERROR disk full", "10:04 ERROR timeout"]

levels = text::pipeline()
  .from_string(log)
  .skip(1)
  .reject(do (line) => line.trim().empty() end)  # drop blank lines
  .split_on(" ")                                 # each line becomes a list of fields
  .map(do (fields) => fields[1] end)
  .run()

println levels   # ["INFO", "ERROR", "WARN", "ERROR"]
```

| Method | Does |
| :-- | :-- |
| `from_file(path)`, `from_string(text)` | Sets the input. |
| `skip(n)` | Drops the first `n` lines. |
| `filter(lambda)`, `reject(lambda)` | Keeps, or drops, lines for which the lambda is truthy. |
| `match(pattern)` | Keeps lines that match a regular expression. |
| `split_on(delimiter)` | Turns each line into a list of fields. |
| `map(lambda)` | Transforms each line, or each list of fields. |
| `run()` | Runs the steps in order and returns the result list. |

For a log file:

```kiwi nocheck
slow = text::pipeline()
  .from_file("access.log")
  .match('\s\d{4,}ms$')
  .run()
```

For structured data, [`csv`](/kiwi/stdlib/csv/) and [`json`](/kiwi/stdlib/json/) are a better fit.

<!-- api: text -->
