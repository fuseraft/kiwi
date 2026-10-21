---
title: Variables
description: Assignment, unpacking, var declarations with type hints, and constants.
---

## Assignment

Assigning to a name creates the variable. Kiwi is dynamically typed, so the same variable can hold a different type later.

```kiwi
count = 10
price = 20.5
greeting = "Hello"
items = [1, 2, 3]

count = "ten"    # now a string
println count    # ten
```

Reading a variable that was never assigned is an error:

```kiwi fails
println missing   # VariableUndefinedError: The variable has not been declared: 'missing'
```

Assignment is an expression, so it can be chained:

```kiwi
a = b = c = 0
println [a, b, c]   # [0, 0, 0]
```

Compound assignment operators (`+=`, `-=`, `*=`, `||=` and so on) are listed under [Operators](/kiwi/language/operators/#assignment).

## Multiple assignment

Separate names with commas to assign several at once. The right-hand side can be a list of expressions or a single list value, which is unpacked.

```kiwi
a, b, c = 10, 20, 30
println "${a} ${b} ${c}"   # 10 20 30

fn bounds()
  [0, 100]
end

low, high = bounds()
println high               # 100

x, y = 1, 2
x, y = y, x                # swap
println [x, y]             # [2, 1]
```

Names without a matching value are set to `null`:

```kiwi
first, second, third = [1, 2]
println third   # null
```

## `var` declarations

`var` declares a variable explicitly, optionally with a type hint. It is never required, but it documents intent and gives typed variables a starting value.

```kiwi
var greeting = "Hello"
var count: integer = 0
var ratio: float = 0.5
var maybe: string? = null
```

With a type hint and no initializer, the variable starts at that type's zero value:

| Hint | Starts as |
| :-- | :-- |
| `integer` | `0` |
| `float` | `0.0` |
| `string` | `""` |
| `boolean` | `false` |
| `list` | `[]` |
| `hashmap` | `{}` |

```kiwi
var total: integer
var names: list

total += 5
names.push("kiwi")
println [total, names]   # [5, ["kiwi"]]
```

:::caution
`var name` with neither an initializer nor one of the hints above does not create the variable; reading it later raises `VariableUndefinedError`. Initialize it instead: `var name = null`.
:::

Group several declarations in parentheses:

```kiwi
var (count: integer = 0, label = "items", scale: float)
println [count, label, scale]   # [0, "items", 0]
```

Type hints on variables are documentation only; Kiwi does not check them at runtime. See [Type Hints](/kiwi/language/type-hints/).

## Constants

`const` declares a value that can't be reassigned. Constant names must use only upper-case letters and underscores.

```kiwi
const MAX_RETRIES = 3
const API_URL = "https://example.com"

println MAX_RETRIES   # 3
```

Reassigning a constant is an error:

```kiwi fails
const LIMIT = 10
LIMIT = 20   # InvalidOperationError: Cannot reassign constant 'LIMIT'.
```

`const` protects the binding, not the value. A constant list or hashmap can still be modified in place:

```kiwi
const NAMES = ["kiwi"]
NAMES.push("mango")
println NAMES   # ["kiwi", "mango"]
```

## Where variables live

Variables assigned at the top level of a script are global. Variables first assigned inside a function belong to that call. See [Scoping](/kiwi/language/scoping/) for the full rules, including closures and struct instance variables.
