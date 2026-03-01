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

---

## Examples

### Heap — priority queue (min-heap)

```kiwi
import "collections"

h = Heap.new("min")
[5, 1, 8, 3, 2].each(with (v) do h.insert(v) end)

println h.peek()          # 1 — smallest, not removed
println h.size()          # 5

sorted = []
while not h.is_empty() do
  sorted.push(h.extract_root())
end
println sorted   # [1, 2, 3, 5, 8]
```

### Heap — max-heap

```kiwi
import "collections"

h = Heap.new("max")
[5, 1, 8, 3, 2].each(with (v) do h.insert(v) end)

println h.extract_root()  # 8
println h.extract_root()  # 5
println h.extract_root()  # 3
```

### Set — basic operations

```kiwi
import "collections"

s = Set.new([1, 2, 2, 3])
println s.size()          # 3 (duplicates removed)
println s.contains(2)     # true

s.add(4)
println s.to_list()       # [1, 2, 3, 4]

s.remove(2)
println s.contains(2)     # false

s.clear()
println s.size()          # 0
```

### Set — algebra

```kiwi
import "collections"

a = Set.new([1, 2, 3, 4])
b = Set.new([3, 4, 5, 6])

println a.intersect(b)              # true  (share 3 and 4)
println a.disjoint(Set.new([7, 8])) # true  (no overlap)

diff = a.difference(b)
println diff.to_list()   # [1, 2]

u = a.union(b)
println u.to_list()      # [1, 2, 3, 4, 5, 6]

c = Set.new([1, 2])
println c.subset(a)      # true  (c ⊆ a)
println a.superset(c)    # true  (a ⊇ c)
```

### Set — merge in place

```kiwi
import "collections"

s = Set.new([1, 2, 3])
s.merge([3, 4, 5])
println s.to_list()   # [1, 2, 3, 4, 5]
```
