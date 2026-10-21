# error

> Construct, inspect and check structured errors.

The `error` package builds the structured errors that Kiwi's `try`/`catch` understands, inspects caught errors, and provides guard helpers that throw when a check fails. [Error Handling](../language/errors.md) explains how errors are thrown and caught; this page is the package reference.

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

## Functions

### `error::new`

```kiwi
error::new(error_type: string = "Error", error_message: string = "An error occurred."): hashmap
```

Creates a structured error hashmap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `error_type` | `string` | `"Error"` | The error type name (default `"Error"`). |
| `error_message` | `string` | `"An error occurred."` | The error message (default `"An error occurred."`). |

**Returns** `hashmap` — A hashmap with `error` and `message` keys.

```kiwi
throw error::new("NotFound", "record 42 does not exist")
```

### `error::type_of`

```kiwi
error::type_of(err): string
```

Returns the type string of a caught error.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `err` | `any` |  | A caught error hashmap (from single-param `catch e`) or a plain string. |

**Returns** `string` — The `error` key if `err` is a structured hashmap, otherwise `err` as a string.

```kiwi
try
  throw error::NotFoundError("item missing")
catch e
  println error::type_of(e)     # NotFoundError
  println error::message_of(e)  # item missing
end
```

### `error::message_of`

```kiwi
error::message_of(err): string
```

Returns the message of a caught error.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `err` | `any` |  | A caught error hashmap (from single-param `catch e`) or a plain string. |

**Returns** `string` — The `message` key if `err` is a structured hashmap, otherwise `err` as a string.

### `error::is_type`

```kiwi
error::is_type(err, error_type: string): boolean
```

Returns true if the caught error matches a specific type name.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `err` | `any` |  | A caught error hashmap (from single-param `catch e`) or a plain string. |
| `error_type` | `string` |  | The type name to match against. |

**Returns** `boolean` — `true` if the error type matches.

```kiwi
# single-param catch — e is a hashmap
try
  throw error::KeyError("missing_key")
catch e
  if error::is_type(e, "KeyError")
    println "handle missing key"
  end
end

# two-param catch — pass the type string directly
try
  throw error::KeyError("missing_key")
catch type, msg
  if error::is_type(type, "KeyError")
    println "handle missing key: ${msg}"
  end
end
```

### `error::assert`

```kiwi
error::assert(condition: boolean, msg: string = "Assertion failed.")
```

Throws an `AssertionError` if `condition` is false.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `condition` | `boolean` |  | The condition to check. |
| `msg` | `string` | `"Assertion failed."` | The error message (default `"Assertion failed."`). |

**Throws** `AssertionError` — If `condition` is `false`.

```kiwi
error::assert(x > 0, "x must be positive")
```

### `error::expect_type`

```kiwi
error::expect_type(val, expected_type: string, name: string = "value")
```

Throws a `TypeError` if `val` is not of the expected type.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `val` | `any` |  | The value to check. |
| `expected_type` | `string` |  | The expected type name (e.g., `"integer"`, `"string"`). |
| `name` | `string` | `"value"` | An optional label for the value, used in the error message. |

**Throws** `TypeError` — If `val.type()` does not match `expected_type`.

```kiwi
fn add(a, b)
  error::expect_type(a, "integer", "a")
  error::expect_type(b, "integer", "b")
  return a + b
end
```

### `error::expect_not_null`

```kiwi
error::expect_not_null(val, name: string = "value")
```

Throws a `ValueError` if `val` is null.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `val` | `any` |  | The value to check. |
| `name` | `string` | `"value"` | An optional label for the value, used in the error message. |

**Throws** `ValueError` — If `val == null`.

```kiwi
fn process(record)
  error::expect_not_null(record, "record")
  # ...
end
```

### `error::ArgumentError`

```kiwi
error::ArgumentError(msg: string = "Invalid argument."): hashmap
```

Creates an `ArgumentError` hashmap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `"Invalid argument."` | The error message (default `"Invalid argument."`). |

**Returns** `hashmap` — A structured error hashmap.

### `error::AssertionError`

```kiwi
error::AssertionError(msg: string = "Assertion failed."): hashmap
```

Creates an `AssertionError` hashmap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `"Assertion failed."` | The error message (default `"Assertion failed."`). |

**Returns** `hashmap` — A structured error hashmap.

### `error::IndexError`

```kiwi
error::IndexError(msg: string = "Index out of bounds."): hashmap
```

Creates an `IndexError` hashmap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `"Index out of bounds."` | The error message (default `"Index out of bounds."`). |

**Returns** `hashmap` — A structured error hashmap.

### `error::IOError`

```kiwi
error::IOError(msg: string = "I/O error."): hashmap
```

Creates an `IOError` hashmap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `"I/O error."` | The error message (default `"I/O error."`). |

**Returns** `hashmap` — A structured error hashmap.

### `error::KeyError`

```kiwi
error::KeyError(msg: string = "Key not found."): hashmap
```

Creates a `KeyError` hashmap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `"Key not found."` | The error message (default `"Key not found."`). |

**Returns** `hashmap` — A structured error hashmap.

### `error::NetworkError`

```kiwi
error::NetworkError(msg: string = "Network error."): hashmap
```

Creates a `NetworkError` hashmap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `"Network error."` | The error message (default `"Network error."`). |

**Returns** `hashmap` — A structured error hashmap.

### `error::NotFoundError`

```kiwi
error::NotFoundError(msg: string = "Not found."): hashmap
```

Creates a `NotFoundError` hashmap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `"Not found."` | The error message (default `"Not found."`). |

**Returns** `hashmap` — A structured error hashmap.

### `error::NotImplementedError`

```kiwi
error::NotImplementedError(msg: string = "Not implemented."): hashmap
```

Creates a `NotImplementedError` hashmap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `"Not implemented."` | The error message (default `"Not implemented."`). |

**Returns** `hashmap` — A structured error hashmap.

### `error::OverflowError`

```kiwi
error::OverflowError(msg: string = "Overflow."): hashmap
```

Creates an `OverflowError` hashmap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `"Overflow."` | The error message (default `"Overflow."`). |

**Returns** `hashmap` — A structured error hashmap.

### `error::ParseError`

```kiwi
error::ParseError(msg: string = "Parse error."): hashmap
```

Creates a `ParseError` hashmap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `"Parse error."` | The error message (default `"Parse error."`). |

**Returns** `hashmap` — A structured error hashmap.

### `error::PermissionError`

```kiwi
error::PermissionError(msg: string = "Permission denied."): hashmap
```

Creates a `PermissionError` hashmap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `"Permission denied."` | The error message (default `"Permission denied."`). |

**Returns** `hashmap` — A structured error hashmap.

### `error::TimeoutError`

```kiwi
error::TimeoutError(msg: string = "Operation timed out."): hashmap
```

Creates a `TimeoutError` hashmap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `"Operation timed out."` | The error message (default `"Operation timed out."`). |

**Returns** `hashmap` — A structured error hashmap.

### `error::TypeError`

```kiwi
error::TypeError(msg: string = "Type error."): hashmap
```

Creates a `TypeError` hashmap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `"Type error."` | The error message (default `"Type error."`). |

**Returns** `hashmap` — A structured error hashmap.

### `error::ValueError`

```kiwi
error::ValueError(msg: string = "Invalid value."): hashmap
```

Creates a `ValueError` hashmap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `"Invalid value."` | The error message (default `"Invalid value."`). |

**Returns** `hashmap` — A structured error hashmap.

### `error::InvalidRange`

```kiwi
error::InvalidRange(msg: string = "Invalid range."): hashmap
```

Creates an `InvalidRange` hashmap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `"Invalid range."` | The error message (default `"Invalid range."`). |

**Returns** `hashmap` — A structured error hashmap.

### `error::InvalidOperation`

```kiwi
error::InvalidOperation(msg: string = "Invalid operation."): hashmap
```

Creates an `InvalidOperation` hashmap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `"Invalid operation."` | The error message (default `"Invalid operation."`). |

**Returns** `hashmap` — A structured error hashmap.
