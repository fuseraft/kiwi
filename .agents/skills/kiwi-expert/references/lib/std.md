# std

> Small general-purpose helpers for type checks and ranges.

The `std` package holds a few general helpers, mostly function-style versions of built-in methods.

```kiwi
println std::len("kiwi")        # 4
println std::type([1, 2])       # list
println std::is_int(3)          # true
println std::is_float(3)        # false
println std::is_string("x")     # true
println std::is_hashmap({})     # true
println std::range(1, 5)        # [1, 2, 3, 4]
```

`std::range(x, y)` is end-exclusive, unlike the `[x to y]` [range literal](../language/lists.md#ranges), which includes `y`.

Most code uses the built-in equivalents directly: `.size()`, `typeof`, and `.is_a(type)`.

## Functions

### `std::len`

```kiwi
std::len(n)
```

Returns the number of elements or characters in a value.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `n` | `any` |  | A string, list, hashmap, or any value with a `size()` method. |

**Returns** `integer` — The size of the value.

### `std::type`

```kiwi
std::type(n)
```

Returns the type name of a value.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `n` | `any` |  | Any value. |

**Returns** `string` — The type name (e.g., `"integer"`, `"string"`, `"list"`).

### `std::is_int`

```kiwi
std::is_int(n)
```

Returns true if the value is an integer.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `n` | `any` |  | Any value. |

**Returns** `boolean` — `true` if `n` is an integer.

### `std::is_float`

```kiwi
std::is_float(n)
```

Returns true if the value is a float.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `n` | `any` |  | Any value. |

**Returns** `boolean` — `true` if `n` is a float.

### `std::is_string`

```kiwi
std::is_string(s)
```

Returns true if the value is a string.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `s` | `any` |  | Any value. |

**Returns** `boolean` — `true` if `s` is a string.

### `std::is_hashmap`

```kiwi
std::is_hashmap(s)
```

Returns true if the value is a hashmap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `s` | `any` |  | Any value. |

**Returns** `boolean` — `true` if `s` is a hashmap.

### `std::range`

```kiwi
std::range(x, y)
```

Returns an integer range `[x, y)` as a list.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `x` | `integer` |  | The inclusive start of the range. |
| `y` | `integer` |  | The exclusive end of the range. |

**Returns** `list` — A list of integers from `x` to `y - 1`, or an empty list when `y <= x`.

**Throws** `InvalidRange` — If either argument is not an integer.
