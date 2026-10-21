---
title: Operators
description: Arithmetic, comparison, logical, bitwise and assignment operators, and their precedence.
---

## Arithmetic

| Operator | Meaning | Example | Result |
| :-- | :-- | :-- | :-- |
| `+` | Add | `3 + 4` | `7` |
| `-` | Subtract, or negate | `10 - 3` | `7` |
| `*` | Multiply | `3 * 4` | `12` |
| `/` | Divide (always a float) | `10 / 4` | `2.5` |
| `//` | Floor divide (always an integer) | `7 // 2` | `3` |
| `%` | Remainder | `10 % 3` | `1` |
| `**` | Power | `2 ** 8` | `256` |

`/` always returns a float, even for two integers. `//` floors both operands and rounds the result toward negative infinity:

```kiwi
println 10 / 4     # 2.5
println 10 // 4    # 2
println -7 // 2    # -4
println 7.9 // 2   # 3
```

`+` and `*` also work on strings and lists. `+` concatenates and `*` repeats. If either side of `+` is a string, the other side is converted to a string.

```kiwi
println "hello" + " world"   # hello world
println [1, 2] + [3, 4]      # [1, 2, 3, 4]
println "ha" * 3             # hahaha
println [0] * 4              # [0, 0, 0, 0]
println "count: " + 5        # count: 5
```

## Comparison

| Operator | Meaning |
| :-- | :-- |
| `==`, `!=` | Equal, not equal |
| `<`, `<=`, `>`, `>=` | Ordering |
| `in` | Membership |

Integers and floats compare numerically. Strings compare lexicographically. `==` compares lists and hashmaps by value. See [Equality](/kiwi/language/types/#equality).

```kiwi
println 3 < 3.14              # true
println 1 == 1.0              # true
println "apple" < "banana"    # true
println [1, 2] == [1, 2]      # true
```

`in` tests whether a list contains a value, a hashmap contains a key, or a string contains a substring:

```kiwi
println 2 in [1, 2, 3]        # true
println "name" in {name: 1}   # true
println "ell" in "hello"      # true
println !(4 in [1, 2, 3])     # true
```

## Logical

Each logical operator has a symbol and a keyword form. They behave identically.

| Symbol | Keyword | Meaning |
| :-- | :-- | :-- |
| `&&` | `and` | True when both sides are truthy |
| `\|\|` | `or` | True when either side is truthy |
| `!` | `not` | Negation |

`&&` and `||` short-circuit: the right side is only evaluated when it is needed.

```kiwi
println true && false     # false
println true and false    # false
println false || true     # true
println not true          # false
```

## Null coalescing

`a ?? b` is `a` unless `a` is `null`, in which case it is `b`. Only `null` triggers the fallback; `false`, `0` and `""` don't.

```kiwi
name = null
println name ?? "anonymous"           # anonymous
println false ?? "fallback"           # false
println null ?? null ?? "found"       # found
```

## Ternary

`condition ? a : b` picks one of two values:

```kiwi
age = 20
println age >= 18 ? "adult" : "minor"   # adult
```

## Bitwise

| Operator | Meaning | Example | Result |
| :-- | :-- | :-- | :-- |
| `&` | AND | `5 & 9` | `1` |
| `\|` | OR | `5 \| 9` | `13` |
| `^` | XOR | `5 ^ 9` | `12` |
| `~` | NOT | `~5` | `-6` |
| `<<` | Shift left | `5 << 1` | `10` |
| `>>` | Arithmetic shift right | `-16 >> 2` | `-4` |
| `>>>` | Logical shift right | `-16 >>> 28` | `15` |

```kiwi
flags = 0b1010
println flags & 0b0010    # 2
println flags | 0b0001    # 11
println flags ^ 0b1111    # 5
println -16 >> 2          # -4
```

## Assignment

| Operator | Equivalent to |
| :-- | :-- |
| `a = b` | Assign |
| `a += b` | `a = a + b` |
| `a -= b` | `a = a - b` |
| `a *= b` | `a = a * b` |
| `a /= b` | `a = a / b` |
| `a //= b` | `a = a // b` |
| `a %= b` | `a = a % b` |
| `a **= b` | `a = a ** b` |
| `a &= b`, `a \|= b`, `a ^= b` | Bitwise AND, OR, XOR |
| `a <<= b`, `a >>= b`, `a >>>= b` | Shifts |
| `a &&= b` | Assign `b` only if `a` is truthy |
| `a \|\|= b` | Assign `b` only if `a` is falsy |

```kiwi
n = 20
n += 5     # 25
n -= 10    # 15
n *= 4     # 60
n //= 7    # 8
n **= 2    # 64
println n  # 64

port = 0
port ||= 8080
println port   # 8080
```

Compound assignment works on list elements, hashmap keys and instance variables too:

```kiwi
scores = [10, 20, 30]
scores[1] += 5
println scores   # [10, 25, 30]

totals = {apples: 3}
totals["apples"] *= 2
println totals   # {"apples": 6}
```

Assigning to several names at once is covered in [Variables](/kiwi/language/variables/#multiple-assignment).

## Precedence

From highest to lowest. Operators on the same row are evaluated left to right.

| Level | Operators |
| :--: | :-- |
| 1 | Unary `-`, `!`, `not`, `~` |
| 2 | `*`, `/`, `//`, `%`, `**` |
| 3 | `+`, `-` |
| 4 | `<<`, `>>`, `>>>` |
| 5 | `<`, `<=`, `>`, `>=`, `in` |
| 6 | `==`, `!=` |
| 7 | `&` |
| 8 | `^` |
| 9 | `\|` |
| 10 | `&&`, `and` |
| 11 | `\|\|`, `or`, `??` |
| 12 | `? :` |
| 13 | Assignment |

Two consequences are worth knowing:

- `**` has the same precedence as `*` and groups left to right, so `2 ** 3 ** 2` is `(2 ** 3) ** 2`, which is `64`. Unary minus binds tighter, so `-2 ** 2` is `4`.
- Comparisons bind tighter than the bitwise operators, so `x & 1 == 0` means `x & (1 == 0)`. Parenthesize bit tests: `(x & 1) == 0`.

```kiwi
println 2 + 3 * 4       # 14
println (2 + 3) * 4     # 20
println 2 ** 3 ** 2     # 64
println (6 & 1) == 0    # true
```

## Line breaks in expressions

A binary operator must be on the same line as its left operand. To split a long expression, end the line with the operator:

```kiwi
a, b, c = 1, 2, 3

ok = a > 0 &&
  b > 0 &&
  c > 0

println ok   # true
```

## Operator overloading

Structs can define their own behavior for binary operators. See [Operator Overloading](/kiwi/language/operator-overloading/).
