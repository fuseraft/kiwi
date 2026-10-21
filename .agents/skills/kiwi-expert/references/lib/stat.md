# stat

> Descriptive statistics for lists of numbers.

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

`sum`, `min`, `max` and `sorted` round out the package. For timing statistics, see [`bench::stats`](bench.md).

## Functions

### `stat::sum`

```kiwi
stat::sum(data)
```

Computes the sum of a list of numbers.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `list` |  | A list of numeric values. |

**Returns** `integer|float` — The sum.

### `stat::min`

```kiwi
stat::min(data)
```

Returns the minimum value in a list.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `list` |  | A list of numeric values. |

**Returns** `integer|float` — The minimum value.

### `stat::max`

```kiwi
stat::max(data)
```

Returns the maximum value in a list.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `list` |  | A list of numeric values. |

**Returns** `integer|float` — The maximum value.

### `stat::range`

```kiwi
stat::range(data)
```

Computes the range (max - min) of a list.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `list` |  | A list of numeric values. |

**Returns** `integer|float` — The range.

### `stat::mean`

```kiwi
stat::mean(data)
```

Computes the arithmetic mean of a list.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `list` |  | A list of numeric values. |

**Returns** `float` — The mean.

### `stat::sorted`

```kiwi
stat::sorted(data)
```

Returns a sorted copy of a list.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `list` |  | A list of comparable values. |

**Returns** `list` — Sorted list.

### `stat::median`

```kiwi
stat::median(data)
```

Computes the median of a list.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `list` |  | A list of numeric values. |

**Returns** `integer|float` — The median.

### `stat::mode`

```kiwi
stat::mode(data)
```

Returns all modes (most frequent values) of a list.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `list` |  | A list of values. |

**Returns** `list` — A list of modal values.

### `stat::variance`

```kiwi
stat::variance(data, population = false)
```

Computes the variance of a list.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `list` |  | A list of numeric values. |
| `population` | `boolean` | `false` | If true, computes population variance. Defaults to false (sample). |

**Returns** `float` — The variance.

### `stat::stdev`

```kiwi
stat::stdev(data, population = false)
```

Computes the standard deviation of a list.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `list` |  | A list of numeric values. |
| `population` | `boolean` | `false` | If true, computes population stdev. Defaults to false (sample). |

**Returns** `float` — The standard deviation.

### `stat::percentile`

```kiwi
stat::percentile(data, p)
```

Computes the p-th percentile of a list (linear interpolation).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `list` |  | A list of numeric values. |
| `p` | `integer\|float` |  | Percentile in [0, 100]. |

**Returns** `float` — The p-th percentile.

### `stat::covariance`

```kiwi
stat::covariance(x_data, y_data)
```

Computes the sample covariance of two lists.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `x_data` | `list` |  | First list of numeric values. |
| `y_data` | `list` |  | Second list of numeric values. |

**Returns** `float` — The sample covariance.

### `stat::correlation`

```kiwi
stat::correlation(x_data, y_data)
```

Computes the Pearson correlation coefficient of two lists.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `x_data` | `list` |  | First list of numeric values. |
| `y_data` | `list` |  | Second list of numeric values. |

**Returns** `float` — Correlation in [-1, 1].

### `stat::zscore`

```kiwi
stat::zscore(data)
```

Computes z-scores (standard scores) for a list.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `list` |  | A list of numeric values. |

**Returns** `list` — List of z-scores.

### `stat::frequency`

```kiwi
stat::frequency(data)
```

Computes a frequency table for a list.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `list` |  | A list of values. |

**Returns** `hashmap` — Maps each unique value (as string) to its count.

### `stat::normalize`

```kiwi
stat::normalize(data)
```

Normalizes a list to [0, 1] range.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `list` |  | A list of numeric values. |

**Returns** `list` — Normalized values.
