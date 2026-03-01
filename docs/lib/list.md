# `list`

The `list` package provides higher-order utility functions for working with lists. All functions take a list as their first argument and are called via the `list::` namespace.

```kiwi
import "list"
```

---

## Package Functions

### `iterator(_list)`

Creates a `ListIterator` instance wrapping the given list. See the [`iter`](iter.md) documentation for the full `ListIterator` API.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `_list` | The list to iterate over. |

**Returns**

| Type | Description |
| :--- | :--- |
| `ListIterator` | An iterator over the list. |

**Example**

```kiwi
import "list"

it = list::iterator([10, 20, 30])
while it.can_read() do
  println it.consume()
end
# prints: 10, 20, 30
```

---

### `all(_list, _predicate)`

Returns `true` if every element in the list satisfies the predicate. Short-circuits on the first failure.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `_list` | The list to test. |
| `lambda` | `_predicate` | A single-argument lambda returning a boolean. |

**Returns**

| Type | Description |
| :--- | :--- |
| `boolean` | `true` if all elements satisfy the predicate, `false` otherwise. |

**Example**

```kiwi
import "list"

println list::all([2, 4, 6], do (n) => n % 2 == 0)  # prints: true
println list::all([2, 3, 6], do (n) => n % 2 == 0)  # prints: false
```

---

### `_any(_list, _predicate)`

Returns `true` if at least one element in the list satisfies the predicate. Short-circuits on the first match.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `_list` | The list to test. |
| `lambda` | `_predicate` | A single-argument lambda returning a boolean. |

**Returns**

| Type | Description |
| :--- | :--- |
| `boolean` | `true` if at least one element satisfies the predicate. |

**Example**

```kiwi
import "list"

println list::_any([1, 3, 4], do (n) => n % 2 == 0)  # prints: true
println list::_any([1, 3, 5], do (n) => n % 2 == 0)  # prints: false
```

---

### `_count(_list, _predicate)`

Returns the number of elements in the list that satisfy the predicate.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `_list` | The list to test. |
| `lambda` | `_predicate` | A single-argument lambda returning a boolean. |

**Returns**

| Type | Description |
| :--- | :--- |
| `integer` | The number of elements for which the predicate returns `true`. |

**Example**

```kiwi
import "list"

println list::_count([1, 2, 3, 4, 5], do (n) => n > 2)  # prints: 3
```

---

### `find(_list, _predicate)`

Returns the index of the first element that satisfies the predicate, or `-1` if no match is found.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `_list` | The list to search. |
| `lambda` | `_predicate` | A single-argument lambda returning a boolean. |

**Returns**

| Type | Description |
| :--- | :--- |
| `integer` | The zero-based index of the first matching element, or `-1`. |

**Example**

```kiwi
import "list"

println list::find(["a", "bb", "ccc"], do (s) => s.size() > 1)  # prints: 1
println list::find(["a", "b"], do (s) => s.size() > 5)          # prints: -1
```

---

### `first_where(_list, _predicate)`

Returns the first element that satisfies the predicate, or `null` if none is found.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `_list` | The list to search. |
| `lambda` | `_predicate` | A single-argument lambda returning a boolean. |

**Returns**

| Type | Description |
| :--- | :--- |
| `any` | The first matching element, or `null`. |

**Example**

```kiwi
import "list"

result = list::first_where([1, 2, 3, 4], do (n) => n > 2)
println result  # prints: 3
```

---

### `index_of(_list, _value)`

Returns the index of the first occurrence of `_value` in the list, or `-1` if not found.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `_list` | The list to search. |
| `any` | `_value` | The value to look for. |

**Returns**

| Type | Description |
| :--- | :--- |
| `integer` | The zero-based index of the first occurrence, or `-1`. |

**Example**

```kiwi
import "list"

println list::index_of(["x", "y", "z"], "y")  # prints: 1
println list::index_of(["x", "y", "z"], "w")  # prints: -1
```

---

### `last_index_of(_list, _value)`

Returns the index of the last occurrence of `_value` in the list, or `-1` if not found.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `_list` | The list to search. |
| `any` | `_value` | The value to look for. |

**Returns**

| Type | Description |
| :--- | :--- |
| `integer` | The zero-based index of the last occurrence, or `-1`. |

**Example**

```kiwi
import "list"

println list::last_index_of([1, 2, 1, 3, 1], 1)  # prints: 4
```

---

### `is_unique(_list)`

Returns `true` if every element in the list is distinct (no duplicates).

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `_list` | The list to check. |

**Returns**

| Type | Description |
| :--- | :--- |
| `boolean` | `true` if all elements are unique. |

**Example**

```kiwi
import "list"

println list::is_unique([1, 2, 3])     # prints: true
println list::is_unique([1, 2, 2, 3])  # prints: false
```

---

### `last_where(_list, _predicate)`

Returns the last element that satisfies the predicate, or `null` if none is found. Searches the list in reverse order.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `_list` | The list to search. |
| `lambda` | `_predicate` | A single-argument lambda returning a boolean. |

**Returns**

| Type | Description |
| :--- | :--- |
| `any` | The last matching element, or `null`. |

**Example**

```kiwi
import "list"

result = list::last_where([1, 2, 3, 4], do (n) => n % 2 == 0)
println result  # prints: 4
```

---

### `_none(_list, _predicate)`

Returns `true` if no element in the list satisfies the predicate.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `_list` | The list to test. |
| `lambda` | `_predicate` | A single-argument lambda returning a boolean. |

**Returns**

| Type | Description |
| :--- | :--- |
| `boolean` | `true` if no elements satisfy the predicate. |

**Example**

```kiwi
import "list"

println list::_none([1, 3, 5], do (n) => n % 2 == 0)  # prints: true
println list::_none([1, 2, 5], do (n) => n % 2 == 0)  # prints: false
```

---

### `one(_list, _predicate)`

Returns `true` if exactly one element in the list satisfies the predicate.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `_list` | The list to test. |
| `lambda` | `_predicate` | A single-argument lambda returning a boolean. |

**Returns**

| Type | Description |
| :--- | :--- |
| `boolean` | `true` if exactly one element matches. |

**Example**

```kiwi
import "list"

println list::one([1, 2, 3], do (n) => n % 2 == 0)  # prints: true  (only 2)
println list::one([1, 2, 4], do (n) => n % 2 == 0)  # prints: false (two matches)
```

---

### `reject(_list, _predicate)`

Returns a new list containing only the elements for which the predicate returns `false`. This is the inverse of the built-in `.filter()` method.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `_list` | The source list. |
| `lambda` | `_predicate` | A single-argument lambda returning a boolean. |

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | A new list of elements that did not match the predicate. |

**Example**

```kiwi
import "list"

result = list::reject([1, 2, 3, 4, 5], do (n) => n % 2 == 0)
println result  # prints: [1, 3, 5]
```

---

### `sum_integer(_list)`

Computes the sum of all elements in the list that are of type `integer`. Non-integer values are silently ignored.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `_list` | A list of mixed values. |

**Returns**

| Type | Description |
| :--- | :--- |
| `integer` | The sum of all integer elements. |

**Example**

```kiwi
import "list"

println list::sum_integer([1, 2.5, 3, "x", 4])  # prints: 8
```

---

### `sum_float(_list)`

Computes the sum of all elements in the list that are of type `float`. Non-float values are silently ignored.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `_list` | A list of mixed values. |

**Returns**

| Type | Description |
| :--- | :--- |
| `float` | The sum of all float elements. |

**Example**

```kiwi
import "list"

println list::sum_float([1, 1.5, 2.5, "x", 3])  # prints: 4.0
```

---

## Notes on Underscore-Prefixed Functions

The functions `_any`, `_count`, and `_none` have underscore prefixes in their names to avoid conflicts with Kiwi built-in keywords or reserved identifiers. They are called exactly as written, including the underscore:

```kiwi
list::_any(data, predicate)
list::_count(data, predicate)
list::_none(data, predicate)
```
