# Operators

Kiwi provides a full set of operators for arithmetic, comparison, logic, bitwise manipulation, and assignment. Most operators work on the types you'd expect; a few have extra behavior for strings and lists worth knowing about.

---

## Arithmetic

| Operator | Name | Example | Result |
| :--- | :--- | :--- | :--- |
| `+` | Add | `3 + 4` | `7` |
| `-` | Subtract | `10 - 3` | `7` |
| `*` | Multiply | `3 * 4` | `12` |
| `/` | Divide | `10 / 4` | `2` |
| `%` | Modulo | `10 % 3` | `1` |
| `**` | Exponent | `2 ** 8` | `256` |

**Integer vs float division:** When both operands are integers the result is an integer. Add a decimal point to get a float.

```kiwi
println 10 / 4    # 2     (integer division)
println 10 / 4.0  # 2.5   (float division)
println 10. / 4   # 2.5   (trailing dot promotes to float)
```

**String and list `+` and `*`:** `+` concatenates strings and lists; `*` repeats them.

```kiwi
println "hello" + " world"   # hello world
println [1, 2] + [3, 4]      # [1, 2, 3, 4]

println "ha" * 3             # hahaha
println [0] * 4              # [0, 0, 0, 0]
```

---

## Comparison

| Operator | Name | Example |
| :--- | :--- | :--- |
| `==` | Equal | `a == b` |
| `!=` | Not equal | `a != b` |
| `<` | Less than | `a < b` |
| `<=` | Less than or equal | `a <= b` |
| `>` | Greater than | `a > b` |
| `>=` | Greater than or equal | `a >= b` |

Comparison works across `integer` and `float` without casting. Strings are compared lexicographically.

```kiwi
println 3 < 3.14      # true
println "apple" < "banana"  # true
println 1 == 1.0      # true
```

---

## Logical

Kiwi accepts both symbol and keyword forms — they are identical in behavior.

| Symbol | Keyword | Description |
| :--- | :--- | :--- |
| `&&` | `and` | True when both operands are truthy |
| `\|\|` | `or` | True when at least one operand is truthy |
| `!` | `not` | Inverts a boolean |

Both forms short-circuit: `&&`/`and` stops at the first falsy value; `||`/`or` stops at the first truthy value.

```kiwi
println true && false    # false
println true and false   # false (same)

println false || true    # true
println false or true    # true  (same)

println !true            # false
println not true         # false (same)
```

---

## Null-coalescing (`??`)

Returns the left operand if it is not `null`, otherwise evaluates and returns the right operand. Chains left-to-right and short-circuits at the first non-null value.

```kiwi
x = null
println x ?? "default"    # default

# Chains resolve to the first non-null value
println null ?? null ?? "found" ?? "other"   # found

# Useful for optional function results
fn maybe(): any
  null
end

result = maybe() ?? "fallback"
println result   # fallback
```

---

## Ternary (`? :`)

Inline conditional expression.

```kiwi
x = 10
label = x > 5 ? "big" : "small"
println label    # big
```

The `when` postfix guard is a clean alternative for single-expression returns and early exits:

```kiwi
fn clamp(n, lo, hi)
  return lo when n < lo
  return hi when n > hi
  n
end
```

---

## Bitwise

| Operator | Name | Example | Notes |
| :--- | :--- | :--- | :--- |
| `&` | AND | `5 & 9` → `1` | Bit-by-bit AND |
| `\|` | OR | `5 \| 9` → `13` | Bit-by-bit OR |
| `^` | XOR | `5 ^ 9` → `12` | Bit-by-bit XOR |
| `~` | NOT | `~5` → `-6` | Bitwise complement |
| `<<` | Left shift | `5 << 1` → `10` | Shift bits left |
| `>>` | Right shift | `5 >> 1` → `2` | Arithmetic right shift |
| `>>>` | Unsigned right shift | `a >>> b` | Logical right shift (no sign fill) |

```kiwi
a = 5   # 0101
b = 9   # 1001

println a & b   # 1   (0001)
println a | b   # 13  (1101)
println a ^ b   # 12  (1100)
println a << 1  # 10  (1010)
println a >> 1  # 2   (0010)
println ~a      # -6
```

**XOR swap** — swap two variables without a temporary:

```kiwi
a = a ^ b
b = a ^ b
a = a ^ b
# a and b are now swapped
```

---

## Assignment

### Basic and chained assignment

```kiwi
x = 42
a = b = c = 0    # chained — all receive the same value
```

### Compound assignment

Each compound operator reads the current value, applies the operation, and writes the result back.

| Operator | Equivalent to |
| :--- | :--- |
| `+=` | `a = a + b` |
| `-=` | `a = a - b` |
| `*=` | `a = a * b` |
| `/=` | `a = a / b` |
| `%=` | `a = a % b` |
| `**=` | `a = a ** b` |

```kiwi
n = 20
n += 5    # 25
n -= 10   # 15
n *= 4    # 60
n /= 3    # 20
n %= 7    # 6
n **= 3   # 216
```

Compound assignment also works on list indices and hashmap keys:

```kiwi
lst = [10, 20, 30]
lst[1] += 5         # lst is now [10, 25, 30]

h = { score: 100 }
h["score"] -= 10    # h["score"] is now 90
```

### Unpack assignment (`=<`)

Assigns multiple variables at once from a comma-separated list or a list value.

```kiwi
a, b, c =< 1, 2, 3
println a   # 1
println b   # 2
println c   # 3

# Unpack a list
coords = [10, 20]
x, y =< coords
println x   # 10
println y   # 20

# Swap without a temp variable
p, q =< 7, 13
p, q =< q, p
println p   # 13
println q   # 7

# Destructure a function's return value
fn bounds(): list
  [0, 100]
end

lo, hi =< bounds()
```

---

## Logical assignment

Short-circuit assignment using the logical operators.

| Operator | Meaning |
| :--- | :--- |
| `&&=` | Assign right side only if left side is truthy |
| `\|\|=` | Assign right side only if left side is falsy |

```kiwi
x = true
x &&= false    # x becomes false (was truthy, so rhs was evaluated)

y = false
y ||= "default"   # y becomes "default" (was falsy, so rhs was evaluated)

z = "existing"
z ||= "fallback"  # z stays "existing" (was truthy, short-circuits)
```

---

## Bitwise assignment

| Operator | Equivalent to |
| :--- | :--- |
| `&=` | `a = a & b` |
| `\|=` | `a = a \| b` |
| `^=` | `a = a ^ b` |
| `~=` | `a = ~b` |
| `<<=` | `a = a << b` |
| `>>=` | `a = a >> b` |

```kiwi
flags = 0b1010
flags |= 0b0001   # set bit 0   → 0b1011
flags &= 0b1110   # clear bit 0 → 0b1010
flags ^= 0b1111   # toggle all  → 0b0101
flags <<= 1       # shift left  → 0b1010
flags >>= 2       # shift right → 0b0010
```

---

## Precedence

Operators are evaluated in the following order (highest to lowest). Operators on the same row have equal precedence and associate left-to-right unless noted.

| Precedence | Operators | Notes |
| :---: | :--- | :--- |
| 1 (highest) | `**` | Right-associative |
| 2 | unary `-`, `!`, `not`, `~` | |
| 3 | `*`, `/`, `%` | |
| 4 | `+`, `-` | |
| 5 | `<<`, `>>`, `>>>` | |
| 6 | `&` | |
| 7 | `^` | |
| 8 | `\|` | |
| 9 | `==`, `!=`, `<`, `<=`, `>`, `>=` | |
| 10 | `&&`, `and` | |
| 11 | `\|\|`, `or` | |
| 12 | `??` | |
| 13 | `? :` | Right-associative |
| 14 (lowest) | `=`, `+=`, `-=`, `*=`, etc. | Right-associative |

When in doubt, use parentheses:

```kiwi
# Without parens — may not read as intended
println 2 + 3 * 4      # 14 (not 20)

# With parens — unambiguous
println (2 + 3) * 4    # 20
```

---

## Multi-line expressions

Kiwi treats newlines as whitespace, so expressions can continue freely across lines. Place the operator at the end of a line or the start of the next — both work.

```kiwi
ok = a > 0
  && b > 0
  && c > 0

total = price
  * quantity
  + tax
```

---

## Operator overloading

Structs can define custom behavior for binary operators by declaring a method named after the operator symbol. See [Operator Overloading](operator_overloading.md) for the full reference.

```kiwi
struct Vec2
  fn new(x, y)
    @x = x
    @y = y
  end

  fn +(other: Vec2): Vec2
    Vec2.new(@x + other.x, @y + other.y)
  end

  fn ==(other: Vec2): boolean
    @x == other.x && @y == other.y
  end

  fn to_string(): string
    "(${@x}, ${@y})"
  end
end

a = Vec2.new(1, 2)
b = Vec2.new(3, 4)
println a + b    # (4, 6)
println a == b   # false
```
