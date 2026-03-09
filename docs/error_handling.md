# Error Handling

Kiwi provides structured error handling through `try`, `catch`, and `finally` blocks, a `throw` statement, and the [`error` standard library package](stdlib/error.md) for constructing and inspecting typed errors.

## Basic Syntax

```kiwi
try
  # code that might fail
catch e
  # e is a hashmap: { error: "...", message: "..." }
finally
  # always runs
end
```

### `try`
Wraps code that might throw. If an error is raised, execution jumps to the matching `catch` block.

### `catch`
Handles a thrown error. Parentheses are optional.

| Syntax | What you get |
|--------|-------------|
| `catch e` | A hashmap `{ error: "TypeName", message: "..." }` |
| `catch e, m` | `e` = type string, `m` = message string |

### `finally`
Runs unconditionally after `try`/`catch`, whether or not an error was thrown. Useful for cleanup.

---

## `catch e` — hashmap form

The single-parameter form gives you a structured hashmap with `error` and `message` keys. Use the [`error` package](stdlib/error.md) helpers to inspect it.

```kiwi
try
  throw error::ValueError("must be positive")
catch e
  println error::type_of(e)     # ValueError
  println error::message_of(e)  # must be positive
end
```

You can also use `e["error"]` and `e["message"]` directly:

```kiwi
try
  throw error::NotFoundError("record 42")
catch e
  println e["error"]    # NotFoundError
  println e["message"]  # record 42
end
```

---

## `catch e, m` — destructured form

The two-parameter form destructures the error into a type string and a message string.

```kiwi
try
  throw error::TimeoutError("30s exceeded")
catch t, m
  println "${t}: ${m}"  # TimeoutError: 30s exceeded
end
```

---

## `throw` — plain strings

You can throw a plain string. In the single-param `catch e` form it is wrapped in a hashmap with type `"KiwiError"`.

```kiwi
try
  throw "something went wrong"
catch e
  println e["error"]    # KiwiError
  println e["message"]  # something went wrong
end
```

In the two-param form:

```kiwi
try
  throw "something went wrong"
catch t, m
  println t  # KiwiError
  println m  # something went wrong
end
```

---

## `finally` block

`finally` always runs — even if an error is thrown and there is no `catch` block. In that case the error is swallowed and execution continues after the `end`.

```kiwi
# try/finally without catch — error is swallowed, finally always runs
x = 0
try
  x = 1
  throw "something failed"
  x = 2
finally
  x += 100
end
println x  # 101
```

With both `catch` and `finally`:

```kiwi
fn read_file(path)
  f = null
  try
    f = fio::open(path, "r")
    return fio::read(f)
  catch e
    eprintln "Read failed: ${error::message_of(e)}"
  finally
    fio::close(f) when f != null
  end
end
```

---

## Nested `try-catch`

```kiwi
for i in [0, 1] do
  try
    println "${i / 0}"
  catch e
    println "loop error: ${error::message_of(e)}"
  finally
    println "done with iteration ${i}"
  end
end
```

Output:
```
loop error: Attempted to divide by zero.
done with iteration 0
loop error: Attempted to divide by zero.
done with iteration 1
```

---

## The `error` package

The `error` standard library package provides helpers for constructing, throwing, and inspecting structured errors. It is loaded automatically.

### Named constructors

Each constructor returns a hashmap `{ error: "TypeName", message: "..." }`.

```kiwi
throw error::ValueError("x must be positive")
throw error::NotFoundError("record 42")
throw error::TypeError("expected integer, got string")
```

Available constructors and their default messages:

| Constructor | Default message |
|-------------|----------------|
| `error::ArgumentError(msg?)` | `"Invalid argument."` |
| `error::AssertionError(msg?)` | `"Assertion failed."` |
| `error::IndexError(msg?)` | `"Index out of bounds."` |
| `error::IOError(msg?)` | `"I/O error."` |
| `error::KeyError(msg?)` | `"Key not found."` |
| `error::NetworkError(msg?)` | `"Network error."` |
| `error::NotFoundError(msg?)` | `"Not found."` |
| `error::NotImplementedError(msg?)` | `"Not implemented."` |
| `error::OverflowError(msg?)` | `"Overflow."` |
| `error::ParseError(msg?)` | `"Parse error."` |
| `error::PermissionError(msg?)` | `"Permission denied."` |
| `error::TimeoutError(msg?)` | `"Operation timed out."` |
| `error::TypeError(msg?)` | `"Type error."` |
| `error::ValueError(msg?)` | `"Invalid value."` |

### `error::new(type, message)`

Creates a structured error hashmap with custom type and message strings.

```kiwi
e = error::new("MyError", "something went wrong")
# { error: "MyError", message: "something went wrong" }
```

Defaults: `type = "Error"`, `message = "An error occurred."`

### Inspection helpers

```kiwi
nfe = error::NotFoundError("item 99")

error::type_of(nfe)        # "NotFoundError"
error::message_of(nfe)     # "item 99"
error::is_type(nfe, "NotFoundError")  # true
error::is_type(nfe, "IOError")        # false
```

These helpers also accept plain strings (as from a two-param `catch`):

```kiwi
try
  throw error::KeyError("name")
catch t, m
  println error::type_of(t)    # KeyError
  println error::message_of(m) # name
end
```

### Guard helpers

**`error::assert(condition, msg?)`** — throws `AssertionError` if `condition` is false.

```kiwi
error::assert(x > 0, "x must be positive")
```

**`error::expect_type(val, expected_type, name?)`** — throws `TypeError` if `val.type()` does not match `expected_type`.

```kiwi
fn add(a, b)
  error::expect_type(a, "integer", "a")
  error::expect_type(b, "integer", "b")
  return a + b
end
```

**`error::expect_not_null(val, name?)`** — throws `ValueError` if `val` is `null`.

```kiwi
fn process(record)
  error::expect_not_null(record, "record")
  # ...
end
```

### Matching error types in `catch`

```kiwi
try
  throw error::KeyError("user_id")
catch e
  if error::is_type(e, "KeyError")
    println "missing key: ${error::message_of(e)}"
  elsif error::is_type(e, "NotFoundError")
    println "not found: ${error::message_of(e)}"
  else
    throw e  # re-throw unhandled errors
  end
end
```

---

## Re-throwing errors

Catch an error and re-throw it if you can't handle it at the current level.

```kiwi
try
  risky_operation()
catch e
  eprintln "Unexpected: ${error::type_of(e)}: ${error::message_of(e)}"
  throw e
end
```

---

## `when` guard with `throw`

The `when` guard can be used with `throw` to raise an error conditionally in a single expression.

```kiwi
fn validate(n)
  throw error::ValueError("must be positive") when n <= 0
end
```
