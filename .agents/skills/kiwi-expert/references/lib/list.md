# list

> Predicates and helpers for lists, also callable as list methods.

The `list` package adds predicate helpers and searches to lists. Because the package [extends the `list` type](../language/packages.md#extending-built-in-types), most functions can also be called as methods:

```kiwi
nums = [3, 8, 1, 8, 5]

println nums.reject(do (n) => n > 4)          # [3, 1]
println nums.first_where(do (n) => n > 4)     # 8
println nums.last_where(do (n) => n > 4)      # 5
println nums.one(do (n) => n == 1)            # true
println nums.is_unique()                      # false
println nums.index_of(8)                      # 1
println nums.last_index_of(8)                 # 3
println [1, 2.5, "x", 3].sum_integer()        # 4
```

A few functions share their names with [built-in methods](../reference/builtin-methods.md#lists) or type keywords. The built-in wins when called as a method, so call these with the package prefix:

```kiwi
nums = [3, 8, 1, 8, 5]

println list::any(nums, do (n) => n > 7)      # true
println list::none(nums, do (n) => n > 9)     # true
println list::all(nums, do (n) => n > 0)      # true
println list::count(nums, do (n) => n == 8)   # 2
println list::find(nums, do (n) => n == 8)    # 1 (the index)
```

`list::iterator(xs)`, or `xs.iterator()`, returns a [`ListIterator`](iter.md).

## Functions

### `list::iterator`

```kiwi
list::iterator(_list: list)
```

Creates an iterator over a list.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_list` | `list` |  | The list to iterate over. |

**Returns** `object` — A `ListIterator` instance.

### `list::all`

```kiwi
list::all(_list: list, _predicate: lambda): boolean
```

Returns true if every element in the list satisfies the predicate.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_list` | `list` |  | The list to test. |
| `_predicate` | `lambda` |  | A single-argument lambda returning a boolean. |

**Returns** `boolean` — `true` if all elements satisfy the predicate, `false` otherwise.

### `list::any`

```kiwi
list::any(_list: list, _predicate: lambda): boolean
```

Returns true if any element in the list satisfies the predicate.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_list` | `list` |  | The list to test. |
| `_predicate` | `lambda` |  | A single-argument lambda returning a boolean. |

**Returns** `boolean` — `true` if at least one element satisfies the predicate.

### `list::count`

```kiwi
list::count(_list: list, _predicate: lambda): integer
```

Returns the count of elements satisfying the predicate.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_list` | `list` |  | The list to test. |
| `_predicate` | `lambda` |  | A single-argument lambda returning a boolean. |

**Returns** `integer` — The number of matching elements.

### `list::find`

```kiwi
list::find(_list: list, _predicate: lambda): integer
```

Finds the index of the first element satisfying the predicate.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_list` | `list` |  | The list to search. |
| `_predicate` | `lambda` |  | A single-argument lambda returning a boolean. |

**Returns** `integer` — The index of the first matching element, or `-1` if not found.

### `list::first_where`

```kiwi
list::first_where(_list: list, _predicate: lambda): any
```

Returns the first element satisfying the predicate, or null.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_list` | `list` |  | The list to search. |
| `_predicate` | `lambda` |  | A single-argument lambda returning a boolean. |

**Returns** `any` — The first matching element, or `null` if none found.

### `list::index_of`

```kiwi
list::index_of(_list: list, _value: any): integer
```

Returns the first index of a given value in the list.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_list` | `list` |  | The list to search. |
| `_value` | `any` |  | The value to find. |

**Returns** `integer` — The index of the first occurrence, or `-1` if not found.

### `list::last_index_of`

```kiwi
list::last_index_of(_list: list, _value: any): integer
```

Returns the last index of a given value in the list.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_list` | `list` |  | The list to search. |
| `_value` | `any` |  | The value to find. |

**Returns** `integer` — The index of the last occurrence, or `-1` if not found.

### `list::is_unique`

```kiwi
list::is_unique(_list: list): boolean
```

Returns true if every element in the list is unique.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_list` | `list` |  | The list to check. |

**Returns** `boolean` — `true` if all elements are distinct.

### `list::last_where`

```kiwi
list::last_where(_list: list, _predicate: lambda): any
```

Returns the last element satisfying the predicate, or null.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_list` | `list` |  | The list to search. |
| `_predicate` | `lambda` |  | A single-argument lambda returning a boolean. |

**Returns** `any` — The last matching element, or `null` if none found.

### `list::none`

```kiwi
list::none(_list: list, _predicate: lambda): boolean
```

Returns true if no element satisfies the predicate.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_list` | `list` |  | The list to test. |
| `_predicate` | `lambda` |  | A single-argument lambda returning a boolean. |

**Returns** `boolean` — `true` if no elements satisfy the predicate.

### `list::one`

```kiwi
list::one(_list: list, _predicate: lambda): boolean
```

Returns true if exactly one element satisfies the predicate.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_list` | `list` |  | The list to test. |
| `_predicate` | `lambda` |  | A single-argument lambda returning a boolean. |

**Returns** `boolean` — `true` if exactly one element matches.

### `list::reject`

```kiwi
list::reject(_list: list, _predicate: lambda): list
```

Returns a list of elements that do not satisfy the predicate.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_list` | `list` |  | The source list. |
| `_predicate` | `lambda` |  | A single-argument lambda returning a boolean. |

**Returns** `list` — A new list of elements for which the predicate returned `false`.

### `list::sum_integer`

```kiwi
list::sum_integer(_list: list): integer
```

Computes the integer sum of all integer elements in the list.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_list` | `list` |  | A list of mixed values (non-integer values are discarded). |

**Returns** `integer` — The sum of all integer elements.

### `list::sum_float`

```kiwi
list::sum_float(_list: list): float
```

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_list` | `list` |  |  |

**Returns** `float`
