# math

> Trigonometry, logarithms, rounding, random numbers and number theory helpers.

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

For realistic fake data, see [`faker`](faker.md).

## Euclidean division

`ediv` and `emod` perform Euclidean division, whose remainder is never negative, unlike `%`. `divmod` returns both as a list.

```kiwi
println -7 % 2                # -1
println math::emod(-7, 2)     # 1
println math::ediv(-7, 2)     # -4
println math::divmod(-7, 2)   # [-4, 1]
```

They satisfy `dividend == divisor * ediv(dividend, divisor) + emod(dividend, divisor)`.

For statistics such as mean, median and standard deviation, see [`stat`](stat.md).

## Functions

### `math::epsilon`

```kiwi
math::epsilon()
```

Gets the smallest difference from 1.0 that a floating-point type can accurately represent.

**Returns** `float` — The machine epsilon value.

### `math::eps`

```kiwi
math::eps(n = 9)
```

Returns a decimal epsilon value of the form `1e-n`.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `n` | `integer` | `9` | The exponent (must be between 1 and 15, default `9`). |

**Returns** `float` — A floating-point value of `0.1 ** n`.

### `math::random`

```kiwi
math::random(_valueX, _valueY)
```

Gets a random number between x and y.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_valueX` | `integer\|float` |  | The lower bound. |
| `_valueY` | `integer\|float` |  | The upper bound. |

**Returns** `integer|float` — A random number in the range `[_valueX, _valueY]`.

### `math::random_set`

```kiwi
math::random_set(_range_start, _range_end, _size)
```

Gets a random set of unique numbers within a given range.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_range_start` | `integer\|float` |  | The lower bound. |
| `_range_end` | `integer\|float` |  | The upper bound. |
| `_size` | `integer` |  | The number of unique values to generate. |

**Returns** `list` — A list of `_size` unique random numbers, or `[]` if the range is invalid.

### `math::listprimes`

```kiwi
math::listprimes(_limit)
```

Gets a list of prime numbers up to a limit.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_limit` | `integer` |  | The upper bound (inclusive). |

**Returns** `list` — A list of prime numbers up to `_limit`.

### `math::nthprime`

```kiwi
math::nthprime(_n)
```

Gets the n-th prime number.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_n` | `integer` |  | The 1-based index of the prime to retrieve. |

**Returns** `integer` — The n-th prime number.

### `math::sin`

```kiwi
math::sin(_value)
```

Computes the sine of a number.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The angle in radians. |

**Returns** `float` — The sine of `_value`.

### `math::cos`

```kiwi
math::cos(_value)
```

Computes the cosine of a number.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The angle in radians. |

**Returns** `float` — The cosine of `_value`.

### `math::tan`

```kiwi
math::tan(_value)
```

Computes the tangent of a number.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The angle in radians. |

**Returns** `float` — The tangent of `_value`.

### `math::asin`

```kiwi
math::asin(_value)
```

Computes the arc sine of a number.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | A value in the range `[-1, 1]`. |

**Returns** `float` — The arc sine of `_value` in radians.

### `math::acos`

```kiwi
math::acos(_value)
```

Computes the arc cosine of a number.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | A value in the range `[-1, 1]`. |

**Returns** `float` — The arc cosine of `_value` in radians.

### `math::atan`

```kiwi
math::atan(_value)
```

Computes the arc tangent of a number.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The number. |

**Returns** `float` — The arc tangent of `_value` in radians.

### `math::atan2`

```kiwi
math::atan2(_valueY, _valueX)
```

Computes the arc tangent of y / x, handling quadrants correctly.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_valueY` | `integer\|float` |  | The y coordinate. |
| `_valueX` | `integer\|float` |  | The x coordinate. |

**Returns** `float` — The angle in radians between the positive x-axis and the point `(x, y)`.

### `math::sinh`

```kiwi
math::sinh(_value)
```

Computes the hyperbolic sine of a number.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The number. |

**Returns** `float` — The hyperbolic sine of `_value`.

### `math::cosh`

```kiwi
math::cosh(_value)
```

Computes the hyperbolic cosine of a number.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The number. |

**Returns** `float` — The hyperbolic cosine of `_value`.

### `math::tanh`

```kiwi
math::tanh(_value)
```

Computes the hyperbolic tangent of a number.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The number. |

**Returns** `float` — The hyperbolic tangent of `_value`.

### `math::log`

```kiwi
math::log(_value)
```

Computes the natural logarithm (base e) of a number.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The number. |

**Returns** `float` — The natural logarithm of `_value`.

### `math::log2`

```kiwi
math::log2(_value)
```

Computes the base-2 logarithm of a number.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The number. |

**Returns** `float` — The base-2 logarithm of `_value`.

### `math::log10`

```kiwi
math::log10(_value)
```

Computes the base-10 logarithm of a number.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The number. |

**Returns** `float` — The base-10 logarithm of `_value`.

### `math::log1p`

```kiwi
math::log1p(_value)
```

Computes log(1 + x) accurately for small values of x.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The number x. |

**Returns** `float` — The value of `log(1 + x)`.

### `math::sqrt`

```kiwi
math::sqrt(_value)
```

Computes the square root of a number.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The number. |

**Returns** `float` — The square root of `_value`.

### `math::cbrt`

```kiwi
math::cbrt(_value)
```

Computes the cube root of a number.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The number. |

**Returns** `float` — The cube root of `_value`.

### `math::fmod`

```kiwi
math::fmod(_valueX, _valueY)
```

Computes the floating-point remainder of x / y.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_valueX` | `integer\|float` |  | The dividend x. |
| `_valueY` | `integer\|float` |  | The divisor y. |

**Returns** `float` — The remainder of `x / y`.

### `math::hypot`

```kiwi
math::hypot(_valueX, _valueY)
```

Computes `sqrt(x**2 + y**2)` without undue overflow or underflow.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_valueX` | `integer\|float` |  | The number x. |
| `_valueY` | `integer\|float` |  | The number y. |

**Returns** `float` — The Euclidean distance from the origin to `(x, y)`.

### `math::isfinite`

```kiwi
math::isfinite(_value)
```

Checks if a value is a finite floating-point number.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The number to test. |

**Returns** `boolean` — `true` if `_value` is finite.

### `math::isinf`

```kiwi
math::isinf(_value)
```

Checks if a value is positive or negative infinity.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The number to test. |

**Returns** `boolean` — `true` if `_value` is infinite.

### `math::isnan`

```kiwi
math::isnan(_value)
```

Checks if a value is NaN (Not a Number).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The number to test. |

**Returns** `boolean` — `true` if `_value` is NaN.

### `math::isnormal`

```kiwi
math::isnormal(_value)
```

Checks if a value is a normal floating-point number.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The number to test. |

**Returns** `boolean` — `true` if `_value` is a normal FP number (not zero, subnormal, infinite, or NaN).

### `math::floor`

```kiwi
math::floor(_value)
```

Rounds x down to the largest integer not greater than x.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The number x. |

**Returns** `float` — The floor of `_value`.

### `math::ceil`

```kiwi
math::ceil(_value)
```

Rounds x up to the smallest integer not less than x.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The number x. |

**Returns** `float` — The ceiling of `_value`.

### `math::round`

```kiwi
math::round(_value: float, _digits: integer = 0)
```

Rounds a double-precision floating-point value to a specified number of fractional digits, and rounds midpoint values to the nearest even number.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `float` |  | The value. |
| `_digits` | `integer` | `0` | The fractional digits. |

**Returns** `float` — The rounded value.

### `math::trunc`

```kiwi
math::trunc(_value)
```

Truncates x to the integer part, rounding towards zero.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The number x. |

**Returns** `float` — The truncated value.

### `math::remainder`

```kiwi
math::remainder(_valueX, _valueY)
```

Computes the IEEE 754 floating-point remainder of x / y.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_valueX` | `integer\|float` |  | The dividend x. |
| `_valueY` | `integer\|float` |  | The divisor y. |

**Returns** `float` — The IEEE 754 remainder of `x / y`.

### `math::exp`

```kiwi
math::exp(_value)
```

Computes e raised to the power of x.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The exponent x. |

**Returns** `float` — The value of `e ** x`.

### `math::expm1`

```kiwi
math::expm1(_value)
```

Computes e raised to the power of x, minus 1.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The exponent x. |

**Returns** `float` — The value of `e ** x - 1`, accurate for small `x`.

### `math::erf`

```kiwi
math::erf(_value)
```

Computes the error function of a number.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The number. |

**Returns** `float` — The error function value `erf(_value)`.

### `math::erfc`

```kiwi
math::erfc(_value)
```

Computes the complementary error function of a number.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The number. |

**Returns** `float` — The complementary error function value `1 - erf(_value)`.

### `math::lgamma`

```kiwi
math::lgamma(_value)
```

Computes the natural logarithm of the absolute value of the gamma function.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The number. |

**Returns** `float` — The value of `log(|gamma(_value)|)`.

### `math::tgamma`

```kiwi
math::tgamma(_value)
```

Computes the gamma function of a number.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The number. |

**Returns** `float` — The gamma function value `gamma(_value)`.

### `math::copysign`

```kiwi
math::copysign(_valueX, _valueY)
```

Copies the sign of y to x.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_valueX` | `integer\|float` |  | The magnitude source. |
| `_valueY` | `integer\|float` |  | The sign source. |

**Returns** `float` — A value with the magnitude of `x` and the sign of `y`.

### `math::nextafter`

```kiwi
math::nextafter(_valueX, _valueY)
```

Gets the next representable floating-point value after x towards y.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_valueX` | `integer\|float` |  | The starting value. |
| `_valueY` | `integer\|float` |  | The direction. |

**Returns** `float` — The next representable FP value from `x` towards `y`.

### `math::fmax`

```kiwi
math::fmax(_valueX, _valueY)
```

Returns the larger of two numbers.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_valueX` | `integer\|float` |  | The first number. |
| `_valueY` | `integer\|float` |  | The second number. |

**Returns** `integer|float` — The maximum of `x` and `y`.

### `math::fmin`

```kiwi
math::fmin(_valueX, _valueY)
```

Returns the smaller of two numbers.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_valueX` | `integer\|float` |  | The first number. |
| `_valueY` | `integer\|float` |  | The second number. |

**Returns** `integer|float` — The minimum of `x` and `y`.

### `math::fdim`

```kiwi
math::fdim(_valueX, _valueY)
```

Returns the positive difference between x and y.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_valueX` | `integer\|float` |  | The first number. |
| `_valueY` | `integer\|float` |  | The second number. |

**Returns** `float` — `max(x - y, 0)`.

### `math::pow`

```kiwi
math::pow(_valueX, _valueY)
```

Raises x to the power of y.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_valueX` | `integer\|float` |  | The base. |
| `_valueY` | `integer\|float` |  | The exponent. |

**Returns** `float` — The value of `x ** y`.

### `math::abs`

```kiwi
math::abs(_value)
```

Returns the absolute value of a number.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_value` | `integer\|float` |  | The number. |

**Returns** `integer|float` — The absolute value of `_value`.

### `math::ediv`

```kiwi
math::ediv(_dividend, _divisor)
```

Computes the Euclidean quotient of `_dividend` divided by `_divisor`.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_dividend` | `integer\|float` |  | The dividend. |
| `_divisor` | `integer\|float` |  | The divisor (must not be zero). |

**Returns** `integer` — The Euclidean quotient.

### `math::emod`

```kiwi
math::emod(_dividend, _divisor)
```

Computes the Euclidean remainder of `_dividend` divided by `_divisor`.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_dividend` | `integer\|float` |  | The dividend. |
| `_divisor` | `integer\|float` |  | The divisor (must not be zero). |

**Returns** `integer` — The Euclidean remainder.

### `math::divmod`

```kiwi
math::divmod(_dividend, _divisor)
```

Performs Euclidean division and returns both the quotient and remainder.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_dividend` | `integer\|float` |  | The dividend. |
| `_divisor` | `integer\|float` |  | The divisor (must not be zero). |

**Returns** `list` — A two-element list `[quotient, remainder]`.
