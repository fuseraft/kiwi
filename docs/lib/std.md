# `std`

The `std` package provides general-purpose utility functions for introspection and common operations. It is automatically loaded by the Kiwi runtime — no explicit import is needed.

```kiwi
# std is pre-loaded; no import required.
# To import explicitly:
import "std"
```

---

## `std` Package Functions

### `std::len(n)`

Returns the number of elements or characters in a value. Delegates to the built-in `.size()` method.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `any` | `n` | A string, list, hashmap, or any value with a `size()` method. |

**Returns**

| Type | Description |
| :--- | :--- |
| `integer` | The size of the value. |

**Example**

```kiwi
println std::len("hello")       # prints: 5
println std::len([1, 2, 3])     # prints: 3
println std::len({a: 1, b: 2})  # prints: 2
```

---

### `std::type(n)`

Returns the type name of a value as a string.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `any` | `n` | Any value. |

**Returns**

| Type | Description |
| :--- | :--- |
| `string` | The type name (e.g., `"integer"`, `"float"`, `"string"`, `"list"`, `"hashmap"`, `"boolean"`). |

**Example**

```kiwi
println std::type(42)        # prints: integer
println std::type(3.14)      # prints: float
println std::type("hello")   # prints: string
println std::type([1, 2])    # prints: list
println std::type({a: 1})    # prints: hashmap
println std::type(true)      # prints: boolean
```

---

### `std::is_int(n)`

Returns `true` if the value is an `integer`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `any` | `n` | Any value. |

**Returns**

| Type | Description |
| :--- | :--- |
| `boolean` | `true` if `n` is an integer. |

**Example**

```kiwi
println std::is_int(10)    # prints: true
println std::is_int(10.0)  # prints: false
println std::is_int("10")  # prints: false
```

---

### `std::is_float(n)`

Returns `true` if the value is a `float`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `any` | `n` | Any value. |

**Returns**

| Type | Description |
| :--- | :--- |
| `boolean` | `true` if `n` is a float. |

**Example**

```kiwi
println std::is_float(3.14)  # prints: true
println std::is_float(3)     # prints: false
```

---

### `std::is_string(n)`

Returns `true` if the value is a `string`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `any` | `n` | Any value. |

**Returns**

| Type | Description |
| :--- | :--- |
| `boolean` | `true` if `n` is a string. |

**Example**

```kiwi
println std::is_string("hello")  # prints: true
println std::is_string(42)       # prints: false
```

---

### `std::is_hashmap(n)`

Returns `true` if the value is a `hashmap`.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `any` | `n` | Any value. |

**Returns**

| Type | Description |
| :--- | :--- |
| `boolean` | `true` if `n` is a hashmap. |

**Example**

```kiwi
println std::is_hashmap({a: 1})  # prints: true
println std::is_hashmap([1, 2])  # prints: false
```

---

### `std::range(x, y)`

Returns a list of consecutive integers from `x` up to and including `y - 1` (exclusive end). Throws an `InvalidRange` error if either argument is not an integer.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `x` | The inclusive start of the range. |
| `integer` | `y` | The exclusive end of the range. |

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | A list of integers `[x, x+1, ..., y-1]`. |

**Throws**

`InvalidRange` if either `x` or `y` is not an integer.

**Example**

```kiwi
println std::range(0, 5)   # prints: [0, 1, 2, 3, 4]
println std::range(3, 7)   # prints: [3, 4, 5, 6]

for i in std::range(1, 4) do
  println i
end
# prints: 1, 2, 3
```

---

## See Also

- [`error`](error.md) — helpers for constructing, throwing, and inspecting structured errors.
