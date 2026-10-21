---
title: math
description: Trigonometry, logarithms, rounding, random numbers and number theory helpers.
---

The `math` package has the usual mathematical functions.

```kiwi
println math::sqrt(2)            # 1.4142135623730951
println math::pow(2, 10)         # 1024
println math::hypot(3, 4)        # 5
println math::round(3.14159, 2)  # 3.14
println math::floor(3.7)         # 3
println math::abs(-42)           # 42
println math::log10(1000)        # 3
```

The functions accept integers and floats alike. Those that compute a float, such as `sqrt` and `floor`, return a float even for an integer argument.

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
| Primes | `listprimes`, `nthprime` |
| Precision | `epsilon`, `eps` |

## Constants

`math::PI`, `math::E` and `math::TAU` (2π) hold the usual constants:

```kiwi
println math::PI                        # 3.141592653589793
println math::E                         # 2.718281828459045
println math::cos(math::PI)             # -1
```

## Primes

`listprimes(limit)` returns the primes up to `limit`, and `nthprime(n)` the `n`-th prime, counting from 1:

```kiwi
println math::listprimes(20)   # [2, 3, 5, 7, 11, 13, 17, 19]
println math::nthprime(10)     # 29
```

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
