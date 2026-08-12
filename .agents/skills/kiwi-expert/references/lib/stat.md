# `stat`

The `stat` package provides descriptive statistics functions for numeric lists: measures of central tendency, spread, distribution, correlation, and normalization.

---

## Package Functions

### Basic Measures

| Function              | Returns            | Description                                      |
|-----------------------|--------------------|--------------------------------------------------|
| `sum(data)`           | `integer\|float`   | Sum of all values                                |
| `min(data)`           | `integer\|float`   | Minimum value                                    |
| `max(data)`           | `integer\|float`   | Maximum value                                    |
| `range(data)`         | `integer\|float`   | `max - min`                                      |
| `mean(data)`          | `float`            | Arithmetic mean                                  |
| `median(data)`        | `integer\|float`   | Middle value (or average of two middle values)   |
| `mode(data)`          | `list`             | List of most-frequent values                     |
| `sorted(data)`        | `list`             | Sorted copy (does not mutate the original)       |

---

### Spread

| Function                         | Returns  | Description                                                       |
|----------------------------------|----------|-------------------------------------------------------------------|
| `variance(data, population?)`    | `float`  | Sample variance by default; pass `true` for population variance  |
| `stdev(data, population?)`       | `float`  | Sample standard deviation; pass `true` for population            |
| `percentile(data, p)`            | `float`  | p-th percentile in [0, 100] via linear interpolation             |
| `zscore(data)`                   | `list`   | List of z-scores (standard scores)                               |

---

### Bivariate

| Function                     | Returns  | Description                            |
|------------------------------|----------|----------------------------------------|
| `covariance(x_data, y_data)` | `float`  | Sample covariance of two lists          |
| `correlation(x_data, y_data)`| `float`  | Pearson correlation coefficient [-1, 1] |

---

### Distribution

| Function           | Returns   | Description                                                     |
|--------------------|-----------|-----------------------------------------------------------------|
| `frequency(data)`  | `hashmap` | Maps each unique value (as string) to its count                 |
| `normalize(data)`  | `list`    | Rescales values to [0.0, 1.0] range                             |

---

## Function Details

### `percentile(data, p)`

**Parameters**

| Type              | Name   | Description                     |
|-------------------|--------|---------------------------------|
| `list`            | `data` | List of numeric values          |
| `integer\|float`  | `p`    | Percentile in `[0, 100]`        |

**Returns** `float`

---

### `variance(data, population?)` / `stdev(data, population?)`

**Parameters**

| Type      | Name         | Description                                          | Default |
|-----------|--------------|------------------------------------------------------|---------|
| `list`    | `data`       | List of numeric values                               | —       |
| `boolean` | `population` | `true` for population statistic, `false` for sample  | `false` |

---

### `covariance(x_data, y_data)` / `correlation(x_data, y_data)`

Both lists must have the same length.

---

## Examples

```kiwi
import "stat"

data = [4, 7, 13, 2, 1, 9, 7, 3]

println stat::sum(data)        # 46
println stat::min(data)        # 1
println stat::max(data)        # 13
println stat::range(data)      # 12
println stat::mean(data)       # 5.75
println stat::median(data)     # 5.5
println stat::mode(data)       # [7]
```

### Spread

```kiwi
import "stat"

data = [2.0, 4.0, 4.0, 4.0, 5.0, 5.0, 7.0, 9.0]

println stat::variance(data)              # 4.571... (sample)
println stat::variance(data, true)        # 4.0      (population)
println stat::stdev(data)                 # 2.138...

println stat::percentile(data, 25)        # 3.5
println stat::percentile(data, 75)        # 5.5

scores = stat::zscore(data)
println scores   # [-1.16, -0.46, ...]
```

### Correlation

```kiwi
import "stat"

x = [1, 2, 3, 4, 5]
y = [2, 4, 5, 4, 5]

println stat::covariance(x, y)    # 1.5
println stat::correlation(x, y)   # 0.874...
```

### Frequency and normalization

```kiwi
import "stat"

data = [1, 2, 2, 3, 3, 3, 4]

freq = stat::frequency(data)
println freq    # { "1": 1, "2": 2, "3": 3, "4": 1 }

normalized = stat::normalize(data)
println normalized  # [0.0, 0.333..., 0.333..., 0.666..., 0.666..., 1.0, ...]
```
