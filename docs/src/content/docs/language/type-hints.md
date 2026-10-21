---
title: Type Hints
description: Annotating parameters, return values and variables with types.
---

Type hints document what a function expects and returns. They are optional, and you can add them to some parameters and not others.

```kiwi
fn area(width: float, height: float): float
  width * height
end

println area(2.5, 4.0)   # 10
```

:::caution[Hints are not enforced]
Kiwi does not currently check type hints at runtime. A call like `area("a", "b")` is not rejected because of the hints; it fails (or not) based on what the body does with the values. Treat hints as documentation, and validate input explicitly where it matters, for example with [`error::expect_type`](/kiwi/stdlib/error/).
:::

The one place a hint changes behavior is a `var` declaration without a value, which starts at the hinted type's zero value. See [`var` declarations](/kiwi/language/variables/#var-declarations).

## Syntax

Put `: type` after a parameter, after the parameter list for the return type, or after a `var` name:

```kiwi
fn greet(name: string, times: integer = 1): string
  "Hello, ${name}! " * times
end

var count: integer = 0
var label: string
```

## Type names

| Hint | Accepts |
| :-- | :-- |
| `integer`, `float`, `boolean`, `string` | Scalars |
| `list`, `hashmap`, `bytes`, `date` | Collections and other built-ins |
| `lambda` | Lambdas and named functions |
| `generator` | Generator objects |
| `object` | Any struct instance |
| A struct name | Instances of that struct (the struct must be defined above the hint) |
| `any` | Anything |
| `none` | `null` |

## Unions and nullable types

Separate types with `|` to accept any of them. Append `?` to a type to also allow `null`; `string?` means the same as `string|none`.

```kiwi
fn describe(value: integer|float): string
  "number ${value}"
end

fn find_user(id: integer): string?
  return null when id < 0
  "user-${id}"
end

fn greet(name: string? = null)
  println name == null ? "Hello, stranger!" : "Hello, ${name}!"
end

println describe(3.5)     # number 3.5
println find_user(-1)     # null
greet()                   # Hello, stranger!
greet("Ada")              # Hello, Ada!
```

Use `?` or `none` for null. The word `null` is not a type name, so `integer|null` is a syntax error.

## Lambdas

Lambda parameters and returns take hints the same way:

```kiwi
scale = do (x: float, factor: float = 2.0): float => x * factor
println scale(1.5)   # 3
```

## Documenting types

For stdlib-style documentation, repeat the types in a [docstring](/kiwi/language/syntax/#docstrings) along with a description of each parameter:

```kiwi
/#
@summary Clamps a number into a range.
@param float value : The number to clamp.
@param float low : The lower bound.
@param float high : The upper bound.
@return float : `value` limited to `low..high`.
#/
fn clamp(value: float, low: float, high: float): float
  [[value, low].max(), high].min()
end

println clamp(12.0, 0.0, 10.0)   # 10
```
