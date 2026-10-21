---
title: stat
description: Descriptive statistics for lists of numbers.
---

The `stat` package computes descriptive statistics over lists of numbers.

```kiwi
data = [2, 4, 4, 4, 5, 5, 7, 9]

println stat::mean(data)            # 5
println stat::median(data)          # 4.5
println stat::mode(data)            # [4]
println stat::range(data)           # 7
println stat::variance(data)        # 4.571428571428571
println stat::stdev(data)           # 2.138089935299395
println stat::stdev(data, true)     # 2 (population standard deviation)
println stat::percentile(data, 90)  # 7.6
```

`variance` and `stdev` compute the sample statistic by default; pass `true` as the second argument for the population statistic. `mode` returns a list, since several values can tie.

## Relationships and shape

```kiwi
x = [1, 2, 3, 4]
y = [2, 4, 5, 9]

println stat::correlation(x, y)          # Pearson correlation, from -1 to 1
println stat::covariance(x, y)
println stat::zscore([1, 2, 3])          # [-1, 0, 1]
println stat::normalize([10, 20, 30])    # [0, 0.5, 1]
println stat::frequency(["a", "b", "a"]) # {"a": 2, "b": 1}
```

`sum`, `min`, `max` and `sorted` round out the package. For timing statistics, see [`bench::stats`](/kiwi/stdlib/bench/).

<!-- api: stat -->
