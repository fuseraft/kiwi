# `math`

The `math` package contains functionality for working with common math operations.

---

## Package Functions

### `eps(n = 9)`

Computes an epsilon value. The value of `n` must be between 1 and 15 (inclusive).

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `n` | The number of decimal places to compute. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | An epsilon value. |

### `epsilon()`

Gets the machine epsilon.

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The machine epsilon. |

**Example**

```kiwi
import "math"

# Epsilon for floating-point equality comparisons
e9  = math::eps()     # 1e-9  (default)
e6  = math::eps(6)    # 1e-6
mep = math::epsilon() # smallest difference from 1.0 representable as float

println e9   # 1e-9
println e6   # 1e-6
println mep  # ~2.22e-16

# Use eps to compare floats without exact equality
a = 0.1 + 0.2
b = 0.3
if math::abs(a - b) < math::eps(9)
  println "a and b are equal (within 9 decimal places)"
end
```

### `random(_valueX, _valueY)`

Returns a random number between x and y.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer`\|`float` | `_valueX` | The number x. |
| `integer`\|`float` | `_valueY` | The number y. |

**Returns**

| Type | Description |
| :--- | :---|
| `integer`\|`float` | A random number. |

### `random(_base, _limit)`

Returns a random string or list from a string or list with a limited number of characters or elements.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string`\|`list` | `_base` | The base string or list containing the elements to build a random distribution from. |
| `integer` | `_limit` | The total number of characters (in a string) or elements (in a list) to produce from the base. |

### `random_set(x, y, n)`

Returns a list of `n` unique random numbers between `x` and `y`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer`\|`float` | `x` | The lower bound. |
| `integer`\|`float` | `y` | The upper bound. |
| `integer` | `n` | The total number of values in the set. |

**Returns**

| Type | Description |
| :--- | :---|
| `list` | A list of random numbers. |

**Example**

```kiwi
import "math"

# Random integer in [1, 100]
n = math::random(1, 100)
println "Random int: ${n}"

# Random float in [0.0, 1.0]
f = math::random(0.0, 1.0)
println "Random float: ${f}"

# Random string — 8 characters drawn from alphabet
alpha = "abcdefghijklmnopqrstuvwxyz"
token = math::random(alpha, 8)
println "Random token: ${token}"

# Random subset — 6 unique numbers from [1, 49] (lottery style)
picks = math::random_set(1, 49, 6)
println "Lottery picks: ${picks.sort()}"
```

### `listprimes(_limit)`

Get a list of prime numbers up to a limit.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `_limit` | The limit. |

**Returns**

| Type | Description |
| :--- | :---|
| `list` | Prime numbers. |

### `nthprime(_n)`

Get the n-th prime number.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `_n` | The n-th number. |

**Returns**

| Type | Description |
| :--- | :---|
| `integer` | The n-th prime number. |

**Example**

```kiwi
import "math"

# All primes up to 50
primes = math::listprimes(50)
println primes
# [2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47]

# The 10th prime
p10 = math::nthprime(10)
println "10th prime: ${p10}"
# 10th prime: 29

# Count primes up to 1000
println math::listprimes(1000).size()
# 168
```

### `sin(_value)`

Computes the sine of a number.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The angle in radians. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The sine of the input. |

### `cos(_value)`

Computes the cosine of a number.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The angle in radians. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The cosine of the input. |

### `tan(_value)`

Computes the tangent of a number.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The angle in radians. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The tangent of the input. |

### `asin(_value)`

Computes the arc sine of a number.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The arc sine of the input. |

### `acos(_value)`

Computes the arc cosine of a number.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The arc cosine of the input. |

### `atan(_value)`

Computes the arc tangent of a number.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The arc tangent of the input. |

### `atan2(_valueY, _valueX)`

Computes the arc tangent of `y / x`, handling quadrants correctly.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_valueY` | The y-coordinate. |
| `float` | `_valueX` | The x-coordinate. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The arc tangent of `y / x`. |

### `sinh(_value)`

Computes the hyperbolic sine of a number.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The hyperbolic sine of the input. |

### `cosh(_value)`

Computes the hyperbolic cosine of a number.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The hyperbolic cosine of the input. |

### `tanh(_value)`

Computes the hyperbolic tangent of a number.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The hyperbolic tangent of the input. |

**Example**

```kiwi
import "math"

# PI derived from acos
PI = math::acos(-1.0)

fn deg_to_rad(deg)
  deg * PI / 180.0
end

fn rnd(x)
  math::round(x, 6)
end

# Unit circle: sin and cos at key angles
for deg in [0, 30, 45, 60, 90, 180, 270, 360] do
  rad = deg_to_rad(deg.to_float())
  s = rnd(math::sin(rad))
  c = rnd(math::cos(rad))
  println "${deg}°  sin=${s}  cos=${c}"
end

# Tangent — undefined at 90 degrees (returns very large number)
println rnd(math::tan(deg_to_rad(45.0)))   # 1.0
println rnd(math::tan(deg_to_rad(30.0)))   # 0.57735

# Inverse trig
println rnd(math::asin(0.5) * 180.0 / PI)   # 30.0  degrees
println rnd(math::acos(0.5) * 180.0 / PI)   # 60.0  degrees
println rnd(math::atan(1.0) * 180.0 / PI)   # 45.0  degrees

# atan2 handles all four quadrants correctly
println rnd(math::atan2(1.0, 1.0)  * 180.0 / PI)   # 45.0
println rnd(math::atan2(-1.0, 0.0) * 180.0 / PI)   # -90.0

# Hyperbolic functions — satisfy identity cosh²(x) - sinh²(x) = 1
x = 1.2
identity = math::round(math::cosh(x) ** 2 - math::sinh(x) ** 2, 10)
println "cosh²(x) - sinh²(x) = ${identity}"  # 1.0
println "tanh(0.0) = ${math::tanh(0.0)}"      # 0.0
println "tanh(inf) ≈ ${rnd(math::tanh(10.0))}" # 1.0
```

### `log(_value)`

Computes the natural logarithm (base e) of a number.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The natural logarithm of the input. |

### `log2(_value)`

Computes the logarithm to base 2 of a number.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The base 2 logarithm of the input. |


### `log10(_value)`

Computes the logarithm to base 10 of a number.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The base 10 logarithm of the input. |


### `log1p(_value)`

Computes `log(1 + x)`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number x. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The value of `log(1 + x)` |

**Example**

```kiwi
import "math"

# Natural log (base e)
println math::round(math::log(1.0), 6)    # 0.0
println math::round(math::log(2.718281828459045), 5)  # ~1.0  (log of e)
println math::round(math::log(10.0), 6)   # ~2.302585

# Base-2 log — useful for bit-length calculations
println math::round(math::log2(1.0),  0)  # 0.0
println math::round(math::log2(8.0),  0)  # 3.0
println math::round(math::log2(1024.0), 0) # 10.0

# Base-10 log — useful for orders of magnitude
println math::round(math::log10(1.0),    0)  # 0.0
println math::round(math::log10(100.0),  0)  # 2.0
println math::round(math::log10(1000.0), 0)  # 3.0

# log1p is more accurate than log(1 + x) for very small x
x = 1e-10
println math::log1p(x)       # accurate
println math::log(1.0 + x)   # may lose precision
```


### `sqrt(_value)`

Computes the square root of a number.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The square root of the input. |

### `cbrt(_value)`

Computes the cube root of a number.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The cube root of the input. |

### `fmod(_valueX, _valueY)`

Gets the floating-point remainder of `x / y`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_valueX` | The number x. |
| `float` | `_valueY` | The number y. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The remainder of `x / y`. |

### `hypot(_valueX, _valueY)`

Computes `sqrt(x^2 + y^2)` without undue overflow or underflow.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_valueX` | The number x. |
| `float` | `_valueY` | The number y. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The Euclidean distance between the point (x, y) and the origin. |

### `pow(_valueX, _valueY)`

Get `x` raised to the power of `y`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_valueX` | The base. |
| `float` | `_valueY` | The exponent. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | `x^y` |

**Example**

```kiwi
import "math"

# Square root and cube root
println math::sqrt(9.0)    # 3.0
println math::sqrt(2.0)    # ~1.41421356
println math::cbrt(27.0)   # 3.0
println math::cbrt(-8.0)   # -2.0

# Power
println math::pow(2.0, 10.0)  # 1024.0
println math::pow(9.0, 0.5)   # 3.0  (same as sqrt)

# Floating-point remainder (keeps sign of dividend, like C fmod)
println math::fmod(7.5, 2.5)  # 0.0
println math::fmod(7.0, 3.0)  # 1.0

# Hypotenuse — safe from overflow
a = 3.0
b = 4.0
println math::hypot(a, b)  # 5.0
```

### `isfinite(_value)`

Checks if a number is a finite value.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number. |

**Returns**

| Type | Description |
| :--- | :---|
| `boolean` | Indicates whether the number is finite. |

### `isinf(_value)`

Checks if a number is an infinite value.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number. |

**Returns**

| Type | Description |
| :--- | :---|
| `boolean` | Indicates whether the number is infinite. |

### `isnan(_value)`

Checks if a number is NaN (Not a Number).

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number. |

**Returns**

| Type | Description |
| :--- | :---|
| `boolean` | Indicates whether the number is NaN. |

### `isnormal(_value)`

Checks if a number is a normal floating-point number.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number. |

**Returns**

| Type | Description |
| :--- | :---|
| `boolean` | Indicates whether the number is a normal number. |

**Example**

```kiwi
import "math"

# Produce special float values
inf_val = math::pow(10.0, 400.0)   # overflow -> infinity
nan_val = math::sqrt(-1.0)         # imaginary -> NaN
zero    = 0.0
normal  = 3.14

println math::isfinite(normal)    # true
println math::isfinite(inf_val)   # false
println math::isfinite(nan_val)   # false

println math::isinf(inf_val)      # true
println math::isinf(normal)       # false

println math::isnan(nan_val)      # true
println math::isnan(normal)       # false

println math::isnormal(normal)    # true
println math::isnormal(zero)      # false  (zero is not "normal")
println math::isnormal(nan_val)   # false
```

### `floor(_value)`

Rounds a number to the largest integer not greater than the number.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number. |

**Returns**

| Type | Description |
| :--- | :---|
| `integer` | The floor value. |

### `ceil(_value)`

Rounds a number to the smallest integer not less than the number.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number. |

**Returns**

| Type | Description |
| :--- | :---|
| `integer` | The ceiling value. |

### `round(_value)`

Rounds a number to the nearest integer, away from zero in halfway cases.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number. |

**Returns**

| Type | Description |
| :--- | :---|
| `integer` | The nearest integer. |

### `trunc(_value)`

Truncates a number to the integer part, towards zero.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The truncated value. |

**Example**

```kiwi
import "math"

# Rounding a positive number
x = 2.7
println math::floor(x)   # 2
println math::ceil(x)    # 3
println math::round(x)   # 3
println math::trunc(x)   # 2.0

# Rounding a negative number — floor and ceil go opposite ways
y = -2.7
println math::floor(y)   # -3  (towards negative infinity)
println math::ceil(y)    # -2  (towards positive infinity)
println math::round(y)   # -3  (away from zero at halfway)
println math::trunc(y)   # -2.0 (towards zero)

# round with digit precision
println math::round(3.14159, 2)   # 3.14
println math::round(3.14159, 4)   # 3.1416

# Practical: chunk a list into N-sized pages
chunk_count = math::ceil(107.0 / 10.0).to_integer()
println "Pages needed: ${chunk_count}"  # 11
```

### `remainder(_valueX, _valueY)`

Computes the IEEE 754 floating-point remainder of `x / y`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_valueX` | The number x. |
| `float` | `_valueY` | The number y. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The remainder of `x / y`. |

### `exp(_value)`

Computes `e^x`, where `e` is the base of the natural logarithm.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number x. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The value of `e^x`. |

### `expm1(_value)`

Computes `e^x - 1`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number x. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The value of `e^x - 1`. |

**Example**

```kiwi
import "math"

# remainder: IEEE 754 version — result is in [-y/2, y/2]
println math::remainder(7.0, 3.0)    # 1.0
println math::remainder(7.5, 2.5)    # 0.0
println math::remainder(-7.0, 3.0)   # -1.0

# exp: e raised to x
println math::round(math::exp(0.0), 6)  # 1.0
println math::round(math::exp(1.0), 5)  # 2.71828  (approximately e)
println math::round(math::exp(2.0), 5)  # 7.38906

# expm1 is more accurate than exp(x) - 1 for small x
x = 1e-9
println math::expm1(x)        # precise
println math::exp(x) - 1.0   # may lose precision due to cancellation
```

### `erf(_value)`

Error function.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The error function of a number. |

### `erfc(_value)`

Complementary error function.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The complementary error function of a number. |

### `lgamma(_value)`

The natural logarithm of the absolute value of the gamma function.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The natural logarithm of the gamma function of a number. |

### `tgamma(_value)`

The gamma function.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The gamma function of a number. |

**Example**

```kiwi
import "math"

# erf and erfc are complementary: erf(x) + erfc(x) = 1
x = 1.0
println math::round(math::erf(x), 6)   # 0.842701
println math::round(math::erfc(x), 6)  # 0.157299
println math::round(math::erf(x) + math::erfc(x), 10)  # 1.0

# tgamma(n) = (n-1)! for positive integers
println math::round(math::tgamma(1.0), 0)   # 1  (0!)
println math::round(math::tgamma(2.0), 0)   # 1  (1!)
println math::round(math::tgamma(5.0), 0)   # 24 (4!)
println math::round(math::tgamma(6.0), 0)   # 120 (5!)

# lgamma avoids overflow for large inputs
println math::round(math::lgamma(100.0), 4)  # log of 99!
```

### `copysign(_valueX, _valueY)`

Copies the sign of `y` to `x`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_valueX` | The number. |
| `float` | `_valueY` | The sign of the number. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | Contains `x` with the sign of `y`. |

### `nextafter(_valueX, _valueY)`

Get the next representable value after x towards y.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_valueX` | The number x. |
| `float` | `_valueY` | The number x is moving towards. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The next representable floating-point value moving from `x` towards `y`. |

### `fmax(_valueX, _valueY)`

Get the maximum of x and y.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_valueX` | The number x. |
| `float` | `_valueY` | The number y. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The maximum value of `x` and `y`. |

### `fmin(_valueX, _valueY)`

Get the minimum of `x` and `y`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_valueX` | The number x. |
| `float` | `_valueY` | The number y. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The minimum value of `x` and `y`. |

### `fdim(_valueX, _valueY)`

Get the positive difference between x and y.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_valueX` | The number x. |
| `float` | `_valueY` | The number y. |

**Returns**

| Type | Description |
| :--- | :---|
| `float` | The positive difference between `x` and `y`. |

**Example**

```kiwi
import "math"

# copysign: take magnitude of x, sign of y
println math::copysign(5.0, -1.0)   # -5.0
println math::copysign(-3.0, 1.0)   # 3.0
println math::copysign(0.0, -2.0)   # -0.0

# nextafter: the very next representable float
x = 1.0
next_up   = math::nextafter(x, 2.0)
next_down = math::nextafter(x, 0.0)
println next_up > x    # true
println next_down < x  # true

# fmax / fmin: NaN-safe min/max
println math::fmax(3.0, 7.0)   # 7.0
println math::fmin(3.0, 7.0)   # 3.0

# fdim: positive difference, or 0 if x <= y
println math::fdim(5.0, 3.0)   # 2.0
println math::fdim(3.0, 5.0)   # 0.0
```

### `abs(_value)`

Get the absolute value of a number.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `float` | `_value` | The number. |

**Returns**

| Type | Description |
| :--- | :---|
| `integer`\|`float` | The absolute value of the number. |

**Example**

```kiwi
import "math"

println math::abs(-42)      # 42
println math::abs(42)       # 42
println math::abs(-3.14)    # 3.14
println math::abs(0)        # 0

# Common use: distance between two values
a = 17
b = 53
println "Distance: ${math::abs(a - b)}"  # 36

# Tolerance check
diff = math::abs(0.1 + 0.2 - 0.3)
println diff < math::eps(9)  # true
```

---

## Euclidean Division

Euclidean division guarantees that the remainder `r` is always non-negative (`0 <= r < |divisor|`), regardless of the signs of the operands. This differs from truncated division (`/`) and floor division (`//`) when the operands have mixed signs.

The three operations satisfy the identity:

```
dividend == divisor * ediv(dividend, divisor) + emod(dividend, divisor)
```

| `a` | `b` | `a // b` (floor) | `ediv(a, b)` | `emod(a, b)` |
| ---: | ---: | ---: | ---: | ---: |
| 7 | 2 | 3 | 3 | 1 |
| -7 | 2 | -4 | -4 | 1 |
| 7 | -2 | -4 | -3 | 1 |
| -7 | -2 | 3 | 4 | 1 |

### `ediv(_dividend, _divisor)`

Compute the Euclidean quotient.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer`\|`float` | `_dividend` | The dividend. |
| `integer`\|`float` | `_divisor` | The divisor (must not be zero). |

**Returns**

| Type | Description |
| :--- | :--- |
| `integer` | The Euclidean quotient. |

**Example**

```kiwi
import "math"

println math::ediv(7, 2)    # 3
println math::ediv(-7, 2)   # -4
println math::ediv(7, -2)   # -3
println math::ediv(-7, -2)  # 4
```

### `emod(_dividend, _divisor)`

Compute the Euclidean remainder (always non-negative).

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer`\|`float` | `_dividend` | The dividend. |
| `integer`\|`float` | `_divisor` | The divisor (must not be zero). |

**Returns**

| Type | Description |
| :--- | :--- |
| `integer` | The Euclidean remainder (`0 <= r < |divisor|`). |

**Example**

```kiwi
import "math"

println math::emod(7, 2)    # 1
println math::emod(-7, 2)   # 1
println math::emod(7, -2)   # 1
println math::emod(-7, -2)  # 1
```

### `divmod(_dividend, _divisor)`

Compute the Euclidean quotient and remainder together, returning `[q, r]`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer`\|`float` | `_dividend` | The dividend. |
| `integer`\|`float` | `_divisor` | The divisor (must not be zero). |

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | A two-element list `[quotient, remainder]`. |

**Example**

```kiwi
import "math"

q, r = math::divmod(17, 5)
println "17 = 5 * ${q} + ${r}"   # 17 = 5 * 3 + 2

# Works correctly for negative dividends
q, r = math::divmod(-17, 5)
println "-17 = 5 * ${q} + ${r}"  # -17 = 5 * -4 + 3

# Useful for time/unit conversions
total_seconds = 3661
hours, rem     = math::divmod(total_seconds, 3600)
minutes, secs  = math::divmod(rem, 60)
println "${hours}h ${minutes}m ${secs}s"   # 1h 1m 1s
```

