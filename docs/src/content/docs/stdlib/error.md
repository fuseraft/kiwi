---
title: error
description: Construct, inspect and check structured errors.
---

The `error` package builds the structured errors that Kiwi's `try`/`catch` understands, inspects caught errors, and provides guard helpers that throw when a check fails. [Error Handling](/kiwi/language/errors/) explains how errors are thrown and caught; this page is the package reference.

A structured error is a hashmap with an `error` key (the type name) and a `message` key:

```kiwi
e = error::NotFoundError("user 42")
println e                       # {"error": "NotFoundError", "message": "user 42"}

try
  throw e
catch caught
  println error::type_of(caught)              # NotFoundError
  println error::message_of(caught)           # user 42
  println error::is_type(caught, "KeyError")  # false
end
```

`type_of`, `message_of` and `is_type` also accept the plain strings that a two-variable `catch type, message` gives you.

## Constructors

`error::new(type, message)` makes an error with any type name. The named constructors take an optional message:

| Constructor | Default message |
| :-- | :-- |
| `ArgumentError` | Invalid argument. |
| `AssertionError` | Assertion failed. |
| `IndexError` | Index out of bounds. |
| `IOError` | I/O error. |
| `InvalidOperation` | Invalid operation. |
| `InvalidRange` | Invalid range. |
| `KeyError` | Key not found. |
| `NetworkError` | Network error. |
| `NotFoundError` | Not found. |
| `NotImplementedError` | Not implemented. |
| `OverflowError` | Overflow. |
| `ParseError` | Parse error. |
| `PermissionError` | Permission denied. |
| `TimeoutError` | Operation timed out. |
| `TypeError` | Type error. |
| `ValueError` | Invalid value. |

## Guards

```kiwi
fn set_volume(level)
  error::expect_type(level, "integer", "level")
  error::assert(level.between(0, 10), "level must be 0..10")
  level
end

println set_volume(7)   # 7

try
  set_volume("loud")
catch e
  println e["message"]  # Expected level to be integer, got string.
end
```

| Function | Throws when |
| :-- | :-- |
| `assert(condition, message)` | `condition` is false (`AssertionError`) |
| `expect_type(value, type, name)` | `value` is not of type `type` (`TypeError`) |
| `expect_not_null(value, name)` | `value` is `null` (`ValueError`) |

<!-- api: error -->
