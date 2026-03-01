# `collections`

The `collections` package provides specialized data structures for Kiwi programs.

It includes:
- `Heap` — a min-heap or max-heap backed by an array
- `Set` — an unordered collection of unique values with set-algebra operations

---

## `Heap` struct

An array-backed binary heap that can operate as either a **min-heap** (root is smallest) or a **max-heap** (root is largest).

### Heap Constructor

**Parameters**

| Type | Name | Description | Default |
|------|------|-------------|---------|
| `string` | `type` | `"min"` for a min-heap, `"max"` for a max-heap | `"min"` |

**Example**
```kiwi
h = Heap.new("min")
h.insert(5)
h.insert(2)
h.insert(8)
println h.extract_root()  # prints: 2
```

### `insert(value)`

Inserts a value into the heap and restores the heap property.

**Parameters**

| Type | Name | Description |
|------|------|-------------|
| `any` | `value` | The value to insert. |

---

### `extract_root()`

Removes and returns the root element (minimum or maximum depending on heap type).

**Returns** `any`

**Throws**
If the heap is empty.

---

### `peek()`

Returns the root element without removing it.

**Returns** `any`

**Throws**
If the heap is empty.

---

### `size()` {#heap-size}

Returns the number of elements in the heap.

**Returns** `integer`

---

### `is_empty()`

Checks whether the heap has no elements.

**Returns** `boolean`

---

## `Set` struct

An unordered collection of unique values. Accepts a `list` or another `Set` for all set-algebra operations.

### Set Constructor

**Parameters**

| Type | Name | Description | Default |
|------|------|-------------|---------|
| `list` | `data` | Initial values (duplicates are removed). | `[]` |

**Example**
```kiwi
s = Set.new([1, 2, 2, 3])
println s.to_list()  # prints: [1, 2, 3]
```

---

### `add(item)`

Adds an item to the set. Has no effect if the item is already present.

**Parameters**

| Type | Name | Description |
|------|------|-------------|
| `any` | `item` | The value to add. |

---

### `remove(item)`

Removes an item from the set. Has no effect if the item is not present.

**Parameters**

| Type | Name | Description |
|------|------|-------------|
| `any` | `item` | The value to remove. |

---

### `contains(item)`

Checks whether the set contains the given item.

**Parameters**

| Type | Name | Description |
|------|------|-------------|
| `any` | `item` | The value to look for. |

**Returns** `boolean`

---

### `size()` {#set-size}

Returns the number of elements in the set.

**Returns** `integer`

---

### `clear()`

Removes all elements from the set.

---

### `difference(data)`

Returns a new `Set` containing elements that are in this set but **not** in `data`.

**Parameters**

| Type | Name | Description |
|------|------|-------------|
| `list` or `Set` | `data` | The set or list to subtract. |

**Returns** `Set`

---

### `disjoint(data)`

Returns `true` if this set shares no elements with `data`.

**Parameters**

| Type | Name | Description |
|------|------|-------------|
| `list` or `Set` | `data` | The set or list to compare. |

**Returns** `boolean`

---

### `intersect(data)`

Returns `true` if this set shares at least one element with `data`.

**Parameters**

| Type | Name | Description |
|------|------|-------------|
| `list` or `Set` | `data` | The set or list to compare. |

**Returns** `boolean`

---

### `merge(data)`

Adds all elements from `data` into this set (mutates in place).

**Parameters**

| Type | Name | Description |
|------|------|-------------|
| `list` or `Set` | `data` | The set or list to merge in. |

---

### `union(data)`

Returns a **new** `Set` that is the union of this set and `data`.

**Parameters**

| Type | Name | Description |
|------|------|-------------|
| `list` or `Set` | `data` | The set or list to union with. |

**Returns** `Set`

---

### `subset(data)`

Returns `true` if every element of this set is contained in `data`.

**Parameters**

| Type | Name | Description |
|------|------|-------------|
| `list` or `Set` | `data` | The set or list to check against. |

**Returns** `boolean`

---

### `superset(data)`

Returns `true` if this set contains every element of `data`.

**Parameters**

| Type | Name | Description |
|------|------|-------------|
| `list` or `Set` | `data` | The set or list to check against. |

**Returns** `boolean`

---

### `to_list()`

Returns a copy of the set's elements as a list.

**Returns** `list`

---

### `to_string()`

Returns a string representation of the set.

**Returns** `string`
