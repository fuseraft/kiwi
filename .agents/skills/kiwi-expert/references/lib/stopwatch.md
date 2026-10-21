# stopwatch

> Measure elapsed time with start, stop, reset and laps.

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

For timing code over many runs with statistics, use [`bench`](bench.md). For raw timestamps, see [`time::ticks`](time.md).

## Functions

### `stopwatch::new`

```kiwi
stopwatch::new(): Stopwatch
```

Creates a new, unstarted Stopwatch.

**Returns** `Stopwatch` — A new Stopwatch instance.

### `stopwatch::start`

```kiwi
stopwatch::start(): Stopwatch
```

Creates a new Stopwatch and immediately starts it.

**Returns** `Stopwatch` — A running Stopwatch instance.

## `Stopwatch` struct

A stopwatch struct for measuring elapsed time.

```kiwi
sw = stopwatch::start()
# ... code to time ...
println sw.elapsed_ms()   # e.g. 42.3

sw.stop()
println sw.elapsed_ms()   # frozen at stop time

sw.reset()
sw.start()
lap1 = sw.lap()
# ... more code ...
lap2 = sw.lap()
println sw.laps()         # [lap1_ms, lap2_ms]
```

### `.start`

```kiwi
start()
```

Starts (or restarts) the stopwatch, clearing any previous laps.

### `.stop`

```kiwi
stop()
```

Stops the stopwatch, freezing the elapsed time.

### `.reset`

```kiwi
reset()
```

Resets the stopwatch to its initial state.

### `.elapsed_ms`

```kiwi
elapsed_ms(): float
```

Returns elapsed time in milliseconds. If running, returns time since start. If stopped, returns frozen elapsed time.

**Returns** `float` — Elapsed milliseconds.

### `.lap`

```kiwi
lap(): float
```

Records a lap and returns the duration of that lap in milliseconds. Each lap is measured from the previous lap (or from start for the first lap).

**Returns** `float` — Lap duration in milliseconds.

### `.laps`

```kiwi
laps(): list
```

Returns a list of all recorded lap durations in milliseconds.

**Returns** `list` — List of float lap durations.

### `.to_string`

```kiwi
to_string(): string
```

Returns a string representation of the current elapsed time.

**Returns** `string` — e.g. "42.3ms"
