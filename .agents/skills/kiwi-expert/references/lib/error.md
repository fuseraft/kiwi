# `error`

The `error` package provides helpers for constructing, throwing, and inspecting structured errors. It is automatically loaded by the Kiwi runtime — no explicit import is needed.

```kiwi
# error is pre-loaded; no import required.
# To import explicitly:
import "error"
```

Errors in Kiwi can be thrown as any value. This package uses hashmaps with `error` and `message` keys as a convention for structured error handling.

---

## Functions

### `error::new(error_type?, error_message?)`

Creates a structured error hashmap.

**Parameters**

| Type | Name | Description | Default |
| :--- | :--- | :--- | :--- |
| `string` | `error_type` | The error type name. | `"Error"` |
| `string` | `error_message` | The error message. | `"An error occurred."` |

**Returns**

| Type | Description |
| :--- | :--- |
| `hashmap` | A hashmap with `error` and `message` keys. |

**Example**

```kiwi
throw error::new("NotFound", "record 42 does not exist")
```

---

### `error::type_of(err)`

Returns the type string of a caught error.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `any` | `err` | A caught error hashmap or a plain string. |

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | The `error` key if `err` is a structured hashmap, otherwise `err` as a string. |

**Example**

```kiwi
try
  throw error::NotFoundError("item missing")
catch e
  println error::type_of(e)     # NotFoundError
  println error::message_of(e)  # item missing
end
```

---

### `error::message_of(err)`

Returns the message of a caught error.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `any` | `err` | A caught error hashmap or a plain string. |

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | The `message` key if `err` is a structured hashmap, otherwise `err` as a string. |

---

### `error::is_type(err, error_type)`

Returns `true` if the caught error matches a specific type name.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `any` | `err` | A caught error hashmap or a plain string. |
| `string` | `error_type` | The type name to match against. |

**Returns**

| Type | Description |
| :--- | :--- |
| `boolean` | `true` if the error type matches. |

**Example**

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

---

### `error::assert(condition, msg?)`

Throws an `AssertionError` if `condition` is false.

**Parameters**

| Type | Name | Description | Default |
| :--- | :--- | :--- | :--- |
| `boolean` | `condition` | The condition to check. | |
| `string` | `msg` | The error message. | `"Assertion failed."` |

**Throws**

`AssertionError` if `condition` is `false`.

**Example**

```kiwi
error::assert(x > 0, "x must be positive")
```

---

### `error::expect_type(val, expected_type, name?)`

Throws a `TypeError` if `val` is not of the expected type.

**Parameters**

| Type | Name | Description | Default |
| :--- | :--- | :--- | :--- |
| `any` | `val` | The value to check. | |
| `string` | `expected_type` | The expected type name (e.g., `"integer"`, `"string"`). | |
| `string` | `name` | A label for the value, used in the error message. | `"value"` |

**Throws**

`TypeError` if `val.type()` does not match `expected_type`.

**Example**

```kiwi
fn add(a, b)
  error::expect_type(a, "integer", "a")
  error::expect_type(b, "integer", "b")
  return a + b
end
```

---

### `error::expect_not_null(val, name?)`

Throws a `ValueError` if `val` is null.

**Parameters**

| Type | Name | Description | Default |
| :--- | :--- | :--- | :--- |
| `any` | `val` | The value to check. | |
| `string` | `name` | A label for the value, used in the error message. | `"value"` |

**Throws**

`ValueError` if `val == null`.

**Example**

```kiwi
fn process(record)
  error::expect_not_null(record, "record")
  # ...
end
```

---

## Named Error Constructors

Each constructor creates a structured error hashmap with a specific type name. All accept an optional `msg` string parameter.

| Function | Default message |
| :--- | :--- |
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
| `error::InvalidRange(msg?)` | `"Invalid range."` |
| `error::InvalidOperation(msg?)` | `"Invalid operation."` |

**Example**

```kiwi
throw error::ValueError("count must be positive")
throw error::NotFoundError("user ${id} does not exist")
throw error::NotImplementedError()
```

---

## Structured Error Handling Example

```kiwi
fn divide(a, b)
  error::expect_type(a, "integer", "a")
  error::expect_type(b, "integer", "b")
  throw error::ArgumentError("Cannot divide by zero.") when b == 0
  return a / b
end

try
  result = divide(10, 0)
catch e
  if error::is_type(e, "ArgumentError")
    println "caught: ${error::message_of(e)}"
  end
end
# prints: caught: Cannot divide by zero.
```
