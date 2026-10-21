---
title: stopwatch
description: Measure elapsed time with start, stop, reset and laps.
---

The `stopwatch` package provides `Stopwatch`, a timer you can start, stop, reset and lap.

```kiwi
watch = stopwatch::start()      # create and start in one step

task::sleep(20)
first = watch.lap()             # milliseconds since the start
task::sleep(10)
watch.lap()
watch.stop()

println watch.laps().size()     # 2
println watch.elapsed_ms() >= 30   # true
println watch.to_string()       # e.g. 30.61ms
```

| Method | Does |
| :-- | :-- |
| `Stopwatch.new()` | Creates a stopped stopwatch. `stopwatch::start()` creates a running one. |
| `start()`, `stop()` | Starts timing from zero, or freezes the elapsed time. |
| `reset()` | Clears the elapsed time and laps. |
| `elapsed_ms()` | The elapsed time in milliseconds. |
| `lap()` | Records and returns the time of the current lap. |
| `laps()` | The recorded lap times. |
| `to_string()` | The elapsed time formatted for display. |

For timing code over many runs with statistics, use [`bench`](/kiwi/stdlib/bench/). For raw timestamps, see [`time::ticks`](/kiwi/stdlib/time/).

<!-- api: stopwatch -->
