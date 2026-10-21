---
title: math
description: Trigonometry, logarithms, rounding, random numbers and number theory helpers.
---

The `math` package has the usual mathematical functions.

```kiwi
println math::sqrt(2.0)          # 1.4142135623730951
println math::pow(2.0, 10.0)     # 1024
println math::hypot(3.0, 4.0)    # 5
println math::round(3.14159, 2)  # 3.14
println math::floor(3.7)         # 3
println math::abs(-42)           # 42
println math::log10(1000.0)      # 3
```

:::caution
Most functions currently require float arguments; passing an integer raises a `ParameterTypeMismatchError`. Write `math::sqrt(16.0)` or convert with `.to_float()`. `abs`, `random`, `random_set`, `divmod`, `ediv` and `emod` accept integers.
:::

| Group | Functions |
| :-- | :-- |
| Powers and roots | `sqrt`, `cbrt`, `pow`, `exp`, `expm1`, `hypot` |
| Logarithms | `log`, `log2`, `log10`, `log1p` |
| Trigonometry | `sin`, `cos`, `tan`, `asin`, `acos`, `atan`, `atan2`, `sinh`, `cosh`, `tanh` |
| Rounding | `floor`, `ceil`, `round`, `trunc` |
| Comparison | `abs`, `fmax`, `fmin`, `fdim`, `copysign`, `nextafter` |
| Division | `fmod`, `remainder`, `divmod`, `ediv`, `emod` |
| Classification | `isfinite`, `isinf`, `isnan`, `isnormal` |
| Special functions | `erf`, `erfc`, `lgamma`, `tgamma` |
| Random | `random`, `random_set` |
| Precision | `epsilon`, `eps` |

There are no constants for π or e; use `math::acos(-1.0)` and `math::exp(1.0)`.

## Random numbers

`random(low, high)` returns a random number between the bounds: an integer for integer bounds, a float for float bounds. `random_set(low, high, count)` returns a list of random integers.

```kiwi
roll = math::random(1, 6)
println roll.between(1, 6)                    # true
println math::random_set(1, 100, 5).size()    # 5
```

For realistic fake data, see [`faker`](/kiwi/stdlib/faker/).

## Euclidean division

`ediv` and `emod` perform Euclidean division, whose remainder is never negative, unlike `%`. `divmod` returns both as a list.

```kiwi
println -7 % 2                # -1
println math::emod(-7, 2)     # 1
println math::ediv(-7, 2)     # -4
println math::divmod(-7, 2)   # [-4, 1]
```

They satisfy `dividend == divisor * ediv(dividend, divisor) + emod(dividend, divisor)`.

For statistics such as mean, median and standard deviation, see [`stat`](/kiwi/stdlib/stat/).

<!-- api: math -->
