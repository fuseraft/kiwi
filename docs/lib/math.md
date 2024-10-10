# `math`

The `math` package contains functionality for working with common math operations.

## Table of Contents

- [Package Functions](#package-functions)
  - [`abs(_value)`](#abs_value)
  - [`acos(_value)`](#acos_value)
  - [`asin(_value)`](#asin_value)
  - [`atan(_value)`](#atan_value)
  - [`atan2(_valueY, _valueX)`](#atan2_valuey-_valuex)
  - [`cbrt(_value)`](#cbrt_value)
  - [`ceil(_value)`](#ceil_value)
  - [`copysign(_valueX, _valueY)`](#copysign_valuex-_valuey)
  - [`cos(_value)`](#cos_value)
  - [`cosh(_value)`](#cosh_value)
  - [`eps(n = 9)`](#epsn--9)
  - [`epsilon()`](#epsilon)
  - [`erf(_value)`](#erf_value)
  - [`erfc(_value)`](#erfc_value)
  - [`exp(_value)`](#exp_value)
  - [`expm1(_value)`](#expm1_value)
  - [`fdim(_valueX, _valueY)`](#fdim_valuex-_valuey)
  - [`floor(_value)`](#floor_value)
  - [`fmax(_valueX, _valueY)`](#fmax_valuex-_valuey)
  - [`fmin(_valueX, _valueY)`](#fmin_valuex-_valuey)
  - [`fmod(_valueX, _valueY)`](#fmod_valuex-_valuey)
  - [`hypot(_valueX, _valueY)`](#hypot_valuex-_valuey)
  - [`isfinite(_value)`](#isfinite_value)
  - [`isinf(_value)`](#isinf_value)
  - [`isnan(_value)`](#isnan_value)
  - [`isnormal(_value)`](#isnormal_value)
  - [`lgamma(_value)`](#lgamma_value)
  - [`listprimes(_limit)`](#listprimes_limit)
  - [`log(_value)`](#log_value)
  - [`log10(_value)`](#log10_value)
  - [`log1p(_value)`](#log1p_value)
  - [`log2(_value)`](#log2_value)
  - [`nextafter(_valueX, _valueY)`](#nextafter_valuex-_valuey)
  - [`nthprime(_n)`](#nthprime_n)
  - [`pow(_valueX, _valueY)`](#pow_valuex-_valuey)
  - [`random(_base, _limit)`](#random_base-_limit)
  - [`random(_valueX, _valueY)`](#random_valuex-_valuey)
  - [`random_set(x, y, n)`](#random_setx-y-n)
  - [`remainder(_valueX, _valueY)`](#remainder_valuex-_valuey)
  - [`round(_value)`](#round_value)
  - [`sin(_value)`](#sin_value)
  - [`sinh(_value)`](#sinh_value)
  - [`sqrt(_value)`](#sqrt_value)
  - [`tan(_value)`](#tan_value)
  - [`tanh(_value)`](#tanh_value)
  - [`tgamma(_value)`](#tgamma_value)
  - [`trunc(_value)`](#trunc_value)

## Package Functions

### `eps(n = 9)`

Computes an epsilon value. The value of `n` must be between 1 and 15 (inclusive).

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `Integer` | `n` | The number of decimal places to compute. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | An epsilon value. |

### `epsilon()`

Gets the machine epsilon.

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The machine epsilon. |

### `random(_valueX, _valueY)`

Returns a random number between x and y.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `Integer`\|`Float` | `_valueX` | The number x. |
| `Integer`\|`Float` | `_valueY` | The number y. |

**Returns**
| Type | Description |
| :--- | :---|
| `Integer`\|`Float` | A random number. |

### `random(_base, _limit)`

Returns a random string or list from a string or list with a limited number of characters or elements.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String`\|`List` | `_base` | The base string or list containing the elements to build a random distribution from. |
| `Integer` | `_limit` | The total number of characters (in a string) or elements (in a list) to produce from the base. |

### `random_set(x, y, n)`

Returns a list of `n` unique random numbers between `x` and `y`.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `Integer`\|`Float` | `x` | The lower bound. |
| `Integer`\|`Float` | `y` | The upper bound. |
| `Integer` | `n` | The total number of values in the set. |

**Returns**
| Type | Description |
| :--- | :---|
| `List` | A list of random numbers. |

### `listprimes(_limit)`

Get a list of prime numbers up to a limit.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `Integer` | `_limit` | The limit. |

**Returns**
| Type | Description |
| :--- | :---|
| `List` | Prime numbers. |

### `nthprime(_n)`

Get the n-th prime number.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `Integer` | `_n` | The n-th number. |

**Returns**
| Type | Description |
| :--- | :---|
| `Integer` | The n-th prime number. |

### `sin(_value)`

Computes the sine of a number.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The angle in radians. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The sine of the input. |

### `cos(_value)`

Computes the cosine of a number.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The angle in radians. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The cosine of the input. |

### `tan(_value)`

Computes the tangent of a number.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The angle in radians. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The tangent of the input. |

### `asin(_value)`

Computes the arc sine of a number.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The arc sine of the input. |

### `acos(_value)`

Computes the arc cosine of a number.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The arc cosine of the input. |

### `atan(_value)`

Computes the arc tangent of a number.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The arc tangent of the input. |

### `atan2(_valueY, _valueX)`

Computes the arc tangent of `y / x`, handling quadrants correctly.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_valueY` | The y-coordinate. |
| `Float` | `_valueX` | The x-coordinate. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The arc tangent of `y / x`. |

### `sinh(_value)`

Computes the hyperbolic sine of a number.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The hyperbolic sine of the input. |

### `cosh(_value)`

Computes the hyperbolic cosine of a number.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The hyperbolic cosine of the input. |

### `tanh(_value)`

Computes the hyperbolic tangent of a number.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The hyperbolic tangent of the input. |

### `log(_value)`

Computes the natural logarithm (base e) of a number.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The natural logarithm of the input. |

### `log2(_value)`

Computes the logarithm to base 2 of a number.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The base 2 logarithm of the input. |


### `log10(_value)`

Computes the logarithm to base 10 of a number.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The base 10 logarithm of the input. |


### `log1p(_value)`

Computes `log(1 + x)`.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number x. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The value of `log(1 + x)` |


### `sqrt(_value)`

Computes the square root of a number.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The square root of the input. |

### `cbrt(_value)`

Computes the cube root of a number.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The cube root of the input. |

### `fmod(_valueX, _valueY)`

Gets the floating-point remainder of `x / y`.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_valueX` | The number x. |
| `Float` | `_valueY` | The number y. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The remainder of `x / y`. |

### `hypot(_valueX, _valueY)`

Computes `sqrt(x^2 + y^2)` without undue overflow or underflow.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_valueX` | The number x. |
| `Float` | `_valueY` | The number y. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The Euclidean distance between the point (x, y) and the origin. |

### `isfinite(_value)`

Checks if a number is a finite value.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number. |

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | Indicates whether the number is finite. |

### `isinf(_value)`

Checks if a number is an infinite value.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number. |

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | Indicates whether the number is infinite. |

### `isnan(_value)`

Checks if a number is NaN (Not a Number).

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number. |

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | Indicates whether the number is NaN. |

### `isnormal(_value)`

Checks if a number is a normal floating-point number.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number. |

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | Indicates whether the number is a normal number. |

### `floor(_value)`

Rounds a number to the largest integer not greater than the number.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number. |

**Returns**
| Type | Description |
| :--- | :---|
| `Integer` | The floor value. |

### `ceil(_value)`

Rounds a number to the smallest integer not less than the number.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number. |

**Returns**
| Type | Description |
| :--- | :---|
| `Integer` | The ceiling value. |

### `round(_value)`

Rounds a number to the nearest integer, away from zero in halfway cases.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number. |

**Returns**
| Type | Description |
| :--- | :---|
| `Integer` | The nearest integer. |

### `trunc(_value)`

Truncates a number to the integer part, towards zero.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The truncated value. |

### `remainder(_valueX, _valueY)`

Computes the IEEE 754 floating-point remainder of `x / y`.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_valueX` | The number x. |
| `Float` | `_valueY` | The number y. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The remainder of `x / y`. |

### `exp(_value)`

Computes `e^x`, where `e` is the base of the natural logarithm.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number x. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The value of `e^x`. |

### `expm1(_value)`

Computes `e^x - 1`.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number x. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The value of `e^x - 1`. |

### `erf(_value)`

Error function.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The error function of a number. |

### `erfc(_value)`

Complementary error function.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The complementary error function of a number. |

### `lgamma(_value)`

The natural logarithm of the absolute value of the gamma function.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The natural logarithm of the gamma function of a number. |

### `tgamma(_value)`

The gamma function.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The gamma function of a number. |

### `copysign(_valueX, _valueY)`

Copies the sign of `y` to `x`.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_valueX` | The number. |
| `Float` | `_valueY` | The sign of the number. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | Contains `x` with the sign of `y`. |

### `nextafter(_valueX, _valueY)`

Get the next representable value after x towards y.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_valueX` | The number x. |
| `Float` | `_valueY` | The number x is moving towards. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The next representable floating-point value moving from `x` towards `y`. |

### `fmax(_valueX, _valueY)`

Get the maximum of x and y.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_valueX` | The number x. |
| `Float` | `_valueY` | The number y. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The maximum value of `x` and `y`. |

### `fmin(_valueX, _valueY)`

Get the minimum of `x` and `y`.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_valueX` | The number x. |
| `Float` | `_valueY` | The number y. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The minimum value of `x` and `y`. |

### `fdim(_valueX, _valueY)`

Get the positive difference between x and y.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_valueX` | The number x. |
| `Float` | `_valueY` | The number y. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | The positive difference between `x` and `y`. |

### `pow(_valueX, _valueY)`

Get `x` raised to the power of `y`.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_valueX` | The base. |
| `Float` | `_valueY` | The exponent. |

**Returns**
| Type | Description |
| :--- | :---|
| `Float` | `x^y` |

### `abs(_value)`

Get the absolute value of a number.

**Parameters**: 
| Type | Name | Description |
| :--- | :--- | :--- |
| `Float` | `_value` | The number. |

**Returns**
| Type | Description |
| :--- | :---|
| `Integer`\|`Float` | The absolute value of the number. |
