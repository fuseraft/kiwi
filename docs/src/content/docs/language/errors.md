---
title: Error Handling
description: Throwing and catching errors with try, catch and finally, and the error package.
---

## Throwing

`throw` raises an error. You can throw a string or a structured error from the [`error`](/kiwi/stdlib/error/) package:

```kiwi
fn withdraw(balance, amount)
  throw "Amount must be positive" when amount <= 0
  throw error::ValueError("Insufficient funds") when amount > balance
  balance - amount
end

println withdraw(100, 30)   # 70
```

An error that isn't caught stops the program. Kiwi prints the error type, message, location and call stack to stderr and exits with status `1`:

```kiwi fails
throw error::NotFoundError("record 42")
```

```text
[NotFoundError]: record 42
File: script.kiwi:1:1
throw error::NotFoundError("record 42")
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
```

## Catching

`try` runs a block. If it throws, control jumps to `catch`. `finally`, if present, runs afterwards either way.

```kiwi
try
  result = 10 / 0
catch e
  println "Failed: ${e["message"]}"
finally
  println "Cleanup"
end
```

`catch` takes the error in one of three forms. Parentheses around the names are optional.

| Form | You get |
| :-- | :-- |
| `catch` | Nothing; the error is discarded |
| `catch e` | A hashmap: `{"error": "TypeName", "message": "..."}` |
| `catch type, message` | The error type and message as two strings |

```kiwi
try
  throw error::TimeoutError("30s exceeded")
catch e
  println e                  # {"error": "TimeoutError", "message": "30s exceeded"}
end

try
  throw error::TimeoutError("30s exceeded")
catch (kind, message)
  println "${kind}: ${message}"   # TimeoutError: 30s exceeded
end
```

A thrown string is wrapped with the type `KiwiError`:

```kiwi
try
  throw "something went wrong"
catch e
  println e["error"]     # KiwiError
  println e["message"]   # something went wrong
end
```

Errors raised by the runtime itself have descriptive types:

```kiwi
failures = [
  do () => 1 / 0,
  do () => [1, 2][5],
  do () => "abc".to_integer(),
  do () => undefined_function()
]

for action in failures do
  try
    action()
  catch e
    println e["error"]
  end
end
```

That prints `DivideByZeroError`, `IndexError`, `ConversionError` and `VariableUndefinedError`.

## Handling specific errors

Check the error type and rethrow anything you can't handle:

```kiwi
fn load(key)
  throw error::KeyError(key)
end

try
  load("user_id")
catch e
  if error::is_type(e, "KeyError")
    println "missing key: ${error::message_of(e)}"
  else
    throw e
  end
end
```

`throw e` inside a `catch` passes the caught error on unchanged.

## `finally`

`finally` runs however the `try` statement ends: after the `try` block completes, after `catch` handles an error, and when the block leaves early through `return`, `break` or `next`:

```kiwi
fn read_config()
  try
    return "loaded"
  finally
    println "closing the file"
  end
end

println read_config()
```

```
closing the file
loaded
```

Without a `catch`, `finally` runs and the error keeps propagating to the caller. The same happens when a `catch` block throws:

```kiwi
attempts = 0
try
  try
    attempts += 1
    throw "failed"
  finally
    attempts += 100
  end
catch e
  println "caught: ${e["message"]}"
end
println attempts   # 101
```

## Custom errors

Any hashmap with `error` and `message` keys is a structured error. `error::new(type, message)` builds one with your own type name:

```kiwi
fn parse_port(text)
  port = text.to_integer()
  throw error::new("ConfigError", "port out of range: ${port}") when port < 1 || port > 65535
  port
end

try
  parse_port("70000")
catch kind, message
  println "${kind}: ${message}"   # ConfigError: port out of range: 70000
end
```

The `error` package also has ready-made constructors: `ArgumentError`, `AssertionError`, `IndexError`, `IOError`, `KeyError`, `NetworkError`, `NotFoundError`, `NotImplementedError`, `OverflowError`, `ParseError`, `PermissionError`, `TimeoutError`, `TypeError`, `ValueError`, `InvalidRange` and `InvalidOperation`.

## Guard helpers

```kiwi
fn add(a, b)
  error::expect_type(a, "integer", "a")
  error::expect_type(b, "integer", "b")
  a + b
end

try
  add(1, "2")
catch e
  println e["message"]   # Expected b to be integer, got string.
end

try
  error::assert(1 > 2, "math is broken")
catch e
  println e["error"]     # AssertionError
end
```

See the [`error`](/kiwi/stdlib/error/) reference for the full list.
