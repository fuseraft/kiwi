# bench

> Benchmark Kiwi code and report timing statistics.

The `bench` package measures how long code takes. A benchmark is a lambda; `bench` runs it a number of times after a short warmup and reports the mean, median, minimum, maximum and standard deviation.

## Benchmarking a suite

Register benchmarks by name, then run them together. `run_all` prints a line per benchmark as it runs, followed by a summary table.

```kiwi
fn fib(n)
  return n when n <= 1
  fib(n - 1) + fib(n - 2)
end

bench::register("fibonacci(15)", do () => fib(15))

bench::register("sort 500 numbers", do ()
  math::random_set(0, 5000, 500).sort()
end)

results = bench::run_all(20, 2)   # 20 timed iterations, 2 warmup runs each
```

The same can be written with the `@bench::mark` decorator:

```kiwi
@bench::mark("join 1000 strings")
fn join_bench()
  [1 to 1000].map(do (n) => n.to_string()).join(",")
end

bench::run_all(10, 1)
```

## Benchmarking one thing

`run` benchmarks a single lambda and returns its statistics:

```kiwi
result = bench::run("sum of squares", do ()
  total = 0
  for n in [0 to 10000] do
    total += n * n
  end
  total
end, 20, 2)

println "mean: ${bench::fmt(result["mean"])}"
```

Every result is a hashmap with these keys. Times are in milliseconds.

| Key | Meaning |
| :-- | :-- |
| `name` | The benchmark name |
| `iterations` | The number of timed runs |
| `total` | Total time across all runs |
| `mean`, `median` | Average and median time per run |
| `min`, `max` | Fastest and slowest run |
| `stddev` | Population standard deviation |

## Timing calls

`@bench::timed` prints the elapsed time of every call to a function, and `@bench::profile(name, iterations, warmup)` benchmarks a function once, as soon as it is defined.

```kiwi
@bench::timed
fn slow_sum(n)
  [1 to n].sum()
end

slow_sum(100000)
```

To compute statistics over timings you collected yourself, pass a list of milliseconds to `bench::stats`:

```kiwi
timings = []
repeat 10 do
  started = time::ticks()
  [1 to 1000].sum()
  timings.push(time::ticksms(time::ticks() - started))
end

summary = bench::stats(timings)
println summary.keys()
```

## Functions

### `bench::initialize`

```kiwi
bench::initialize()
```

Initializes the global benchmark registry if not already set up.

### `bench::register`

```kiwi
bench::register(name: string, b: lambda)
```

Registers a named benchmark lambda for later execution by `run_all()`.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | The display name of the benchmark. |
| `b` | `lambda` |  | A no-argument lambda containing the code to benchmark. |

### `bench::stats`

```kiwi
bench::stats(timings: list): hashmap
```

Computes descriptive statistics from a list of timing values.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `timings` | `list` |  | A list of float timing values (in ms). |

**Returns** `hashmap` — A hashmap with keys: total, mean, median, min, max, stddev.

### `bench::fmt`

```kiwi
bench::fmt(val: float): string
```

Formats a float timing value as a string with 3 decimal places and "ms" suffix.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `val` | `float` |  | The timing value in milliseconds. |

**Returns** `string` — The formatted string (e.g., "1.234ms").

### `bench::run`

```kiwi
bench::run(name: string, b: lambda, iterations: integer = 100, warmup: integer = 5): hashmap
```

Runs a single named benchmark, printing a one-line result.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | The display name of the benchmark. |
| `b` | `lambda` |  | A no-argument lambda containing the code to benchmark. |
| `iterations` | `integer` | `100` | Number of timed iterations to run (default 100). |
| `warmup` | `integer` | `5` | Number of warmup iterations before timing (default 5). |

**Returns** `hashmap` — A result hashmap with keys: name, iterations, total, mean, median, min, max, stddev.

### `bench::run_all`

```kiwi
bench::run_all(iterations: integer = 100, warmup: integer = 5): list
```

Runs all registered benchmarks and prints a summary table.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `iterations` | `integer` | `100` | Number of timed iterations per benchmark (default 100). |
| `warmup` | `integer` | `5` | Number of warmup iterations per benchmark (default 5). |

**Returns** `list` — A list of result hashmaps for all registered benchmarks.

### `bench::mark`

```kiwi
bench::mark(f, name: string = "benchmark")
```

Decorator — registers a function in the global benchmark registry for `bench::run_all()`. The decorated function remains callable normally.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `f` | `lambda` |  | The function to register (injected by the decorator system). |
| `name` | `string` | `"benchmark"` | The display name of the benchmark (required). |

**Returns** `lambda` — The original function, unmodified.

```kiwi
@bench::mark("my bench")
fn my_bench()
  # ... code to benchmark ...
end
bench::run_all()
```

### `bench::profile`

```kiwi
bench::profile(f, name: string = "benchmark", iterations: integer = 100, warmup: integer = 5)
```

Decorator — immediately runs a benchmark of the decorated function at definition time. The decorated function remains callable normally.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `f` | `lambda` |  | The function to benchmark (injected by the decorator system). |
| `name` | `string` | `"benchmark"` | The display name (default: "benchmark"). |
| `iterations` | `integer` | `100` | Number of timed iterations (default: 100). |
| `warmup` | `integer` | `5` | Number of warmup iterations (default: 5). |

**Returns** `lambda` — The original function, unmodified.

```kiwi
@bench::profile("fast add", 1000, 10)
fn fast_add(a, b)
  return a + b
end
```

### `bench::timed`

```kiwi
bench::timed(f)
```

Decorator — wraps a function so that every call prints its elapsed time.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `f` | `lambda` |  | The function to wrap (injected by the decorator system). |

**Returns** `lambda` — A wrapper that times each call and prints the result.

```kiwi
@bench::timed
fn expensive(n)
  # ... slow computation ...
end
expensive(1000)  # prints: expensive: 1.234ms
```

### `bench::print_summary`

```kiwi
bench::print_summary(results: list)
```

Prints a formatted summary table of benchmark results.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `results` | `list` |  | A list of result hashmaps from `run()` or `run_all()`. |
