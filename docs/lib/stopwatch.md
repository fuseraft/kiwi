# `stopwatch`

The `stopwatch` package provides a simple, ergonomic `Stopwatch` struct for measuring elapsed time. It wraps `time::ticks()` and `time::ticksms()` so you never have to capture a start tick manually.

---

## Quick Start

```kiwi
import "stopwatch"

sw = stopwatch::start()

# ... code to measure ...

println sw.elapsed_ms()   # e.g. 42.317ms (live, while running)

sw.stop()
println sw.elapsed_ms()   # frozen at stop time
```

---

## Package Functions

### `stopwatch::new()`
Creates a new, unstarted `Stopwatch`.

**Returns**

| Type | Description |
| :--- | :--- |
| `Stopwatch` | An idle stopwatch. Call `.start()` when ready. |

---

### `stopwatch::start()`
Creates a new `Stopwatch` and immediately starts it.

**Returns**

| Type | Description |
| :--- | :--- |
| `Stopwatch` | A running stopwatch. |

---

## `Stopwatch` Methods

### `.start()`
Starts (or restarts) the stopwatch. Clears any recorded laps.

**Returns** _None_

---

### `.stop()`
Stops the stopwatch, freezing the elapsed time. Has no effect if already stopped.

**Returns** _None_

---

### `.reset()`
Resets the stopwatch to its initial idle state (clears elapsed time and laps).

**Returns** _None_

---

### `.elapsed_ms()`
Returns the elapsed time in milliseconds.

- While **running**: returns live time since `.start()`.
- While **stopped**: returns the frozen elapsed time at the moment `.stop()` was called.
- Before `.start()` is called: returns `0.0`.

**Returns**

| Type | Description |
| :--- | :--- |
| `float` | Elapsed milliseconds. |

---

### `.lap()`
Records a lap and returns the duration of that lap in milliseconds. Each lap is measured from the end of the previous lap (or from `.start()` for the first lap). Has no effect and returns `0.0` if the stopwatch is not running.

**Returns**

| Type | Description |
| :--- | :--- |
| `float` | Duration of the lap in milliseconds. |

---

### `.laps()`
Returns a list of all recorded lap durations in milliseconds.

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | List of `float` lap durations, in order. |

---

### `.to_string()`
Returns the current elapsed time as a formatted string (e.g. `"42.317ms"`), rounded to 3 decimal places.

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | Formatted elapsed time. |

---

## Examples

### Basic elapsed time

```kiwi
import "stopwatch"

sw = stopwatch::start()

total = 0
for i in [1 to 100000] do
  total += i
end

println "Sum: ${total}"
println "Elapsed: ${sw.elapsed_ms()}ms"
```

---

### Start / stop / reset

```kiwi
import "stopwatch"

sw = stopwatch::new()
sw.start()

# ... phase one ...

sw.stop()
println "Phase one: ${sw.elapsed_ms()}ms"

sw.reset()
sw.start()

# ... phase two ...

sw.stop()
println "Phase two: ${sw.elapsed_ms()}ms"
```

---

### Lap timing

```kiwi
import "stopwatch"

sw = stopwatch::start()

repeat 3 do
  # ... work ...
  println "Lap: ${sw.lap()}ms"
end

sw.stop()
println "Total: ${sw.elapsed_ms()}ms"
println "All laps: ${sw.laps()}"
```

---

### Using `to_string()`

```kiwi
import "stopwatch"

sw = stopwatch::start()
# ... work ...
println "Done in ${sw.to_string()}"
# => Done in 7.412ms
```
