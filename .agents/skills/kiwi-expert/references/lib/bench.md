# `bench`

The `bench` package provides tools for measuring and reporting the performance of Kiwi code. Benchmarks are registered by name, run for a configurable number of iterations (with optional warmup), and produce descriptive statistics: mean, median, min, max, and standard deviation.

---

## Package Functions

### `initialize()`
Initializes the global benchmark registry if it hasn't been initialized yet. Called automatically by `register` and `run_all`.

**Parameters**
_None_

**Returns**
_None_

---

### `register(name, b)`
Registers a named benchmark lambda for later execution by `run_all()`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `name` | The display name of the benchmark. |
| `lambda` | `b` | A no-argument lambda containing the code to benchmark. |

**Returns**
_None_

---

### `run(name, b, iterations = 100, warmup = 5)`
Runs a single benchmark, printing a one-line progress result, and returns a result hashmap.

The benchmark lambda is first called `warmup` times (results discarded) to allow the runtime to reach a steady state. It is then called `iterations` times, recording the wall-clock duration of each call. Statistics are computed over those timed iterations.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `name` | The display name of the benchmark. |
| `lambda` | `b` | A no-argument lambda containing the code to benchmark. |
| `integer` | `iterations` | Number of timed iterations (default `100`). |
| `integer` | `warmup` | Number of un-timed warmup calls before timing begins (default `5`). |

**Returns**

| Type | Description |
| :--- | :--- |
| `hashmap` | A [result hashmap](#result-hashmap) containing the benchmark name and timing statistics. |

---

### `run_all(iterations = 100, warmup = 5)`
Runs every benchmark registered via `register()`, prints a one-line result for each as it runs, then prints a full summary table.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `iterations` | Number of timed iterations per benchmark (default `100`). |
| `integer` | `warmup` | Number of warmup calls per benchmark (default `5`). |

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | A list of [result hashmaps](#result-hashmap), one per registered benchmark. |

---

### `mark(f, name = "benchmark")`
Decorator that registers a function in the global benchmark registry for later execution by `bench::run_all()`. The decorated function remains callable normally.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `lambda` | `f` | The wrapped function (injected by the decorator system). |
| `string` | `name` | Display name for the benchmark (default `"benchmark"`). |

**Returns**

| Type | Description |
| :--- | :--- |
| `lambda` | The original function, unmodified. |

```kiwi
@bench::mark("sum 1k")
fn sum_1k()
  total = 0
  for i in [1..1000] do total += i end
end

@bench::mark("sum 5k")
fn sum_5k()
  total = 0
  for i in [1..5000] do total += i end
end

bench::run_all(iterations: 20, warmup: 2)
```

---

### `profile(f, name = "benchmark", iterations = 100, warmup = 5)`
Decorator that immediately runs a benchmark of the decorated function at definition time. The decorated function remains callable normally afterward.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `lambda` | `f` | The wrapped function (injected by the decorator system). |
| `string` | `name` | Display name (default `"benchmark"`). |
| `integer` | `iterations` | Number of timed iterations (default `100`). |
| `integer` | `warmup` | Number of warmup iterations (default `5`). |

**Returns**

| Type | Description |
| :--- | :--- |
| `lambda` | The original function, unmodified. |

```kiwi
@bench::profile("sqrt loop", 50, 3)
fn sqrt_loop()
  for i in [1..100] do
    math::sqrt(i * 1.0)
  end
end

# Function is still callable normally:
sqrt_loop()
```

---

### `timed(f)`
Decorator that wraps a function so that every call prints its elapsed time. The return value of the original function is preserved.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `lambda` | `f` | The function to wrap (injected by the decorator system). |

**Returns**

| Type | Description |
| :--- | :--- |
| `lambda` | A wrapper that times each call and prints `elapsed: Xms`. |

```kiwi
@bench::timed
fn slow_sum(n)
  total = 0
  for i in [1..n] do total += i end
  return total
end

result = slow_sum(10000)
# elapsed: 10.069ms
println result  # 50005000
```

---

### `print_summary(results)`
Prints a formatted table of benchmark results with columns for iterations, mean, median, min, max, and standard deviation. Called automatically by `run_all()`, but can also be called manually on a list of results collected via `run()`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `results` | A list of [result hashmaps](#result-hashmap). |

**Returns**
_None_

---

### `stats(timings)`
Computes descriptive statistics from a list of timing values. Used internally by `run()`, but available directly if you want to collect timings yourself.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `timings` | A list of `float` timing values in milliseconds. |

**Returns**

| Type | Description |
| :--- | :--- |
| `hashmap` | A hashmap with keys: `total`, `mean`, `median`, `min`, `max`, `stddev` (all `float`, in ms). |

---

### `fmt(val)`
Formats a float timing value as a string rounded to 3 decimal places with an `ms` suffix (e.g., `"1.234ms"`).

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `val` | A timing value in milliseconds. |

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | The formatted string. |

---

## Result Hashmap

Both `run()` and `run_all()` return hashmaps with the following keys:

| Key | Type | Description |
| :--- | :--- | :--- |
| `name` | `string` | The benchmark's display name. |
| `iterations` | `integer` | The number of timed iterations that were run. |
| `total` | `float` | Total elapsed time across all iterations (ms). |
| `mean` | `float` | Arithmetic mean per iteration (ms). |
| `median` | `float` | Median per iteration (ms). |
| `min` | `float` | Fastest single iteration (ms). |
| `max` | `float` | Slowest single iteration (ms). |
| `stddev` | `float` | Population standard deviation (ms). |

---

## Examples

### Running a suite with `register` / `run_all`

```kiwi
fn fib(n: integer): integer
  if n <= 1
    return n
  end
  fib(n - 1) + fib(n - 2)
end

bench::register("fibonacci (n=20)", with do fib(20) end)

bench::register("list sort (500 items)", with do
  math::random_set(0, 5000, 500).sort()
end)

bench::register("string concat (200x)", with do
  s = ""
  repeat 200 do
    s += "kiwi"
  end
  s
end)

bench::run_all(50, 3)
```

**Output:**
```
bench: fibonacci (n=20)                               mean: 19.2ms  min: 17.1ms  max: 24.6ms
bench: list sort (500 items)                          mean: 23.4ms  min: 21.0ms  max: 31.2ms
bench: string concat (200x)                           mean: 0.09ms  min: 0.05ms  max: 0.31ms

Benchmark Summary
---------------------------------------------------------------------------------------------------
Benchmark                                 Iters   Mean        Median      Min         Max         Std Dev
---------------------------------------------------------------------------------------------------
fibonacci (n=20)                          50      19.2ms      19.0ms      17.1ms      24.6ms      1.4ms
list sort (500 items)                     50      23.4ms      22.8ms      21.0ms      31.2ms      2.1ms
string concat (200x)                      50      0.09ms      0.08ms      0.05ms      0.31ms      0.04ms
---------------------------------------------------------------------------------------------------
```

### Running a one-off benchmark with `run`

```kiwi
result = bench::run("sum of squares (0..10000)", with do
  total = 0
  for n in [0 to 10000] do
    total += n * n
  end
  total
end, 20, 2)

println "mean: ${bench::fmt(result.mean)}, stddev: ${bench::fmt(result.stddev)}"
```

### Using `stats` directly

```kiwi
timings = []
repeat 100 do
  t0 = time::ticks()
  # ... code to measure ...
  timings.push(time::ticksms(time::ticks() - t0))
end

s = bench::stats(timings)
println "mean: ${bench::fmt(s.mean)}  stddev: ${bench::fmt(s.stddev)}"
```

### Using decorators

```kiwi
import "bench"
import "math"

# Register multiple benchmarks declaratively, then run them all at once.
@bench::mark("fibonacci 20")
fn fib_bench()
  fn fib(n)
    return n when n <= 1
    fib(n - 1) + fib(n - 2)
  end
  fib(20)
end

@bench::mark("sort 500 items")
fn sort_bench()
  math::random_set(0, 5000, 500).sort()
end

bench::run_all(iterations: 50, warmup: 3)

# Immediately profile a single function at definition time.
@bench::profile("string concat 200x", 50, 3)
fn concat_bench()
  s = ""
  repeat 200 do s += "kiwi" end
  s
end

# Per-call timing — each invocation prints elapsed time.
@bench::timed
fn slow_sum(n)
  total = 0
  for i in [1..n] do total += i end
  return total
end

slow_sum(100000)  # elapsed: Xms
```
