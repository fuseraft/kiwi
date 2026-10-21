---
title: bench
description: Benchmark Kiwi code and report timing statistics.
---

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

<!-- api: bench -->
