# `math`

## Overview

The `math` module contains functionality for working with common math operations.

## Importing the Module
To use the `math` module, import it at the beginning of your Kiwi script.

```ruby
import "@kiwi/math"
```

## Module Functions

### `epsilon()`
- **Summary**: Gets the smallest difference from 1.0 that a floating-point type can accurately represent.
- **Parameters**: None.
- **Returns**: Double representing the epsilon value.

### `random(@_valueX, @_valueY)`
- **Summary**: Returns a random number between x and y.
- **Parameters**: 
  - `@_valueX` - The number x.
  - `@_valueY` - The number y.
- **Returns**: Integer or double random number.

### `sin(@_value)`
- **Summary**: Computes the sine of a number.
- **Parameters**: `@_value` - The angle in radians.
- **Returns**: Double representing the sine of the input.

### `cos(@_value)`
- **Summary**: Computes the cosine of a number.
- **Parameters**: `@_value` - The angle in radians.
- **Returns**: Double representing the cosine of the input.

### `tan(@_value)`
- **Summary**: Computes the tangent of a number.
- **Parameters**: `@_value` - The angle in radians.
- **Returns**: Double representing the tangent of the input.

### `asin(@_value)`
- **Summary**: Computes the arc sine of a number.
- **Parameters**: `@_value` - The number.
- **Returns**: Double representing the arc sine of the input.

### `acos(@_value)`
- **Summary**: Computes the arc cosine of a number.
- **Parameters**: `@_value` - The number.
- **Returns**: Double representing the arc cosine of the input.

### `atan(@_value)`
- **Summary**: Computes the arc tangent of a number.
- **Parameters**: `@_value` - The number.
- **Returns**: Double representing the arc tangent of the input.

### `atan2(@_valueY, @_valueX)`
- **Summary**: Computes the arc tangent of y / x, handling quadrants correctly.
- **Parameters**: `@_valueY` - The y-coordinate, `@_valueX` - The x-coordinate.
- **Returns**: Double representing the arc tangent of y / x.

### `sinh(@_value)`
- **Summary**: Computes the hyperbolic sine of a number.
- **Parameters**: `@_value` - The number.
- **Returns**: Double representing the hyperbolic sine of the input.

### `cosh(@_value)`
- **Summary**: Computes the hyperbolic cosine of a number.
- **Parameters**: `@_value` - The number.
- **Returns**: Double representing the hyperbolic cosine of the input.

### `tanh(@_value)`
- **Summary**: Computes the hyperbolic tangent of a number.
- **Parameters**: `@_value` - The number.
- **Returns**: Double representing the hyperbolic tangent of the input.

### `log(@_value)`
- **Summary**: Computes the natural logarithm (base e) of a number.
- **Parameters**: `@_value` - The number.
- **Returns**: Double representing the natural logarithm of the input.

### `log2(@_value)`
- **Summary**: Computes the logarithm to base 2 of a number.
- **Parameters**: `@_value` - The number.
- **Returns**: Double representing the base 2 logarithm of the input.

### `log10(@_value)`
- **Summary**: Computes the logarithm to base 10 of a number.
- **Parameters**: `@_value` - The number.
- **Returns**: Double representing the base 10 logarithm of the input.

### `log1p(@_value)`
- **Summary**: Computes log(1 + x).
- **Parameters**: 
  - `@_value`: The number x.
- **Returns**: Double representing log(1 + x).

### `sqrt(@_value)`
- **Summary**: Computes the square root of a number.
- **Parameters**: 
  - `@_value`: The number.
- **Returns**: Double representing the square root of the input.

### `cbrt(@_value)`
- **Summary**: Computes the cube root of a number.
- **Parameters**: 
  - `@_value`: The number.
- **Returns**: Double representing the cube root of the input.

### `fmod(@_valueX, @_valueY)`
- **Summary**: Gets the FP remainder of x / y.
- **Parameters**: 
  - `@_valueX`: The number x.
  - `@_valueY`: The number y.
- **Returns**: Double representing the remainder of x / y.

### `hypot(@_valueX, @_valueY)`
- **Summary**: Computes sqrt(x^2 + y^2) without undue overflow or underflow.
- **Parameters**: 
  - `@_valueX`: The number x.
  - `@_valueY`: The number y.
- **Returns**: Double representing the Euclidean distance between the point (x, y) and the origin.

### `isfinite(@_value)`
- **Summary**: Checks if x is a finite value.
- **Parameters**: 
  - `@_value`: The number x.
- **Returns**: Boolean indicating whether x is finite.

### `isinf(@_value)`
- **Summary**: Checks if x is an infinite value.
- **Parameters**: 
  - `@_value`: The number x.
- **Returns**: Boolean indicating whether x is infinite.

### `isnan(@_value)`
- **Summary**: Checks if x is NaN (Not a Number).
- **Parameters**: 
  - `@_value`: The number x.
- **Returns**: Boolean indicating whether x is NaN.

### `isnormal(@_value)`
- **Summary**: Checks if x is a normal floating-point number.
- **Parameters**: 
  - `@_value`: The number x.
- **Returns**: Boolean indicating whether x is a normal number.

### `floor(@_value)`
- **Summary**: Rounds x to the largest integer not greater than x.
- **Parameters**: 
  - `@_value`: The number x.
- **Returns**: Double representing the largest integer not greater than x.

### `ceil(@_value)`
- **Summary**: Rounds x to the smallest integer not less than x.
- **Parameters**: 
  - `@_value`: The number x.
- **Returns**: Double representing the smallest integer not less than x.

### `round(@_value)`
- **Summary**: Rounds x to the nearest integer, away from zero in halfway cases.
- **Parameters**: 
  - `@_value`: The number x.
- **Returns**: Double representing the nearest integer to x.

### `trunc(@_value)`
- **Summary**: Truncates x to the integer part, towards zero.
- **Parameters**: 
  - `@_value`: The number x.
- **Returns**: Double representing the truncated value of x.

### `remainder(@_valueX, @_valueY)`
- **Summary**: IEEE 754 floating-point remainder of x / y.
- **Parameters**: 
  - `@_valueX`: The number x.
  - `@_valueY`: The number y.
- **Returns**: Double representing the remainder of the division of x by y.

### `exp(@_value)`
- **Summary**: Computes e^x, where e is the base of the natural logarithms.
- **Parameters**: 
  - `@_value`: The number x.
- **Returns**: Double representing e raised to the power of x.

### `expm1(@_value)`
- **Summary**: Computes e^x - 1.
- **Parameters**: 
  - `@_value`: The number x.
- **Returns**: Double representing (e^x) - 1.

### `erf(@_value)`
- **Summary**: Error function.
- **Parameters**: 
  - `@_value`: The number.
- **Returns**: Double representing the error function of x.

### `erfc(@_value)`
- **Summary**: Complementary error function.
- **Parameters**: 
  - `@_value`: The number.
- **Returns**: Double representing the complementary error function of x.

### `lgamma(@_value)`
- **Summary**: The natural logarithm of the absolute value of the gamma function.
- **Parameters**: 
  - `@_value`: The number.
- **Returns**: Double representing the natural logarithm of the gamma function of x.

### `tgamma(@_value)`
- **Summary**: The gamma function.
- **Parameters**: 
  - `@_value`: The number.
- **Returns**: Double representing the gamma function of x.

### `copysign(@_valueX, @_valueY)`
- **Summary**: Copies the sign of y to x.
- **Parameters**:
  - `@_valueX`: The magnitude of the number.
  - `@_valueY`: The sign of the number.
- **Returns**: Double with the magnitude of x and the sign of y.

### `nextafter(@_valueX, @_valueY)`
- **Summary**: Get the next representable value after x towards y.
- **Parameters**:
  - `@_valueX`: The starting point number.
  - `@_valueY`: The number x is moving towards.
- **Returns**: Double representing the next representable floating-point value moving from x towards y.

### `fmax(@_valueX, @_valueY)`
- **Summary**: Get the maximum of x and y.
- **Parameters**:
  - `@_valueX`: The first number.
  - `@_valueY`: The second number.
- **Returns**: Double representing the maximum value of x and y.

### `fmin(@_valueX, @_valueY)`
- **Summary**: Get the minimum of x and y.
- **Parameters**:
  - `@_valueX`: The first number.
  - `@_valueY`: The second number.
- **Returns**: Double representing the minimum value of x and y.

### `fdim(@_valueX, @_valueY)`
- **Summary**: Get the positive difference between x and y.
- **Parameters**:
  - `@_valueX`: The first number.
  - `@_valueY`: The second number.
- **Returns**: Double representing the positive difference between x and y.

### `pow(@_valueX, @_valueY)`
- **Summary**: Get x raised to the power of y.
- **Parameters**:
  - `@_valueX`: The base number.
  - `@_valueY`: The exponent number.
- **Returns**: Double representing x raised to the power of y.

### `abs(@_value)`
- **Summary**: Get the absolute value of a number.
- **Parameters**:
  - `@_value`: The number.
- **Returns**: Integer|Double representing the absolute value of x.
