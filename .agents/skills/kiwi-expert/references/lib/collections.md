# collections

> Heap (priority queue) and Set data structures.

The `collections` package adds two data structures that lists and hashmaps don't cover well: `Heap`, a priority queue, and `Set`, a collection of unique values with set algebra.

## Heap

A `Heap` always gives back its smallest value first (a min-heap, the default) or its largest (`Heap.new("max")`). Inserting and removing take logarithmic time, which makes it the right tool for priority queues and "top N" problems.

```kiwi
queue = Heap.new("min")
[5, 1, 8, 3, 2].each(do (v) => queue.insert(v))

println queue.peek()   # 1, without removing it
println queue.size()   # 5

ordered = []
while !queue.is_empty() do
  ordered.push(queue.extract_root())
end
println ordered        # [1, 2, 3, 5, 8]
```

```kiwi
largest = Heap.new("max")
[5, 1, 8, 3, 2].each(do (v) => largest.insert(v))

println largest.extract_root()   # 8
println largest.extract_root()   # 5
```

`peek` and `extract_root` throw if the heap is empty, so check `is_empty()` first.

## Set

A `Set` holds each value at most once. Create one from a list, then add, remove and test membership:

```kiwi
tags = Set.new(["kiwi", "fruit", "kiwi"])
println tags.size()             # 2
println tags.contains("fruit")  # true

tags.add("green")
tags.remove("fruit")
println tags.to_list()          # ["kiwi", "green"]
```

Set algebra methods accept another `Set` or a plain list:

```kiwi
a = Set.new([1, 2, 3, 4])
b = Set.new([3, 4, 5, 6])

println a.union(b).to_list()        # [1, 2, 3, 4, 5, 6]
println a.difference(b).to_list()   # [1, 2]
println a.intersect(b)              # true: they share elements
println a.disjoint([7, 8])          # true: no shared elements
println Set.new([1, 2]).subset(a)   # true
println a.superset([1, 2])          # true

a.merge([9, 10])                    # add elements in place
println a.size()                    # 6
```

## `Heap` struct

A binary heap data structure supporting min-heap and max-heap ordering.

```kiwi
h = Heap.new("min")
h.insert(5)
h.insert(1)
h.insert(3)
println h.extract_root()  # 1
```

### `Heap.new`

```kiwi
Heap.new(type: string = "min")
```

Creates a new heap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `type` | `string` | `"min"` | The heap ordering — `"min"` for a min-heap or `"max"` for a max-heap (default `"min"`). |

### `.insert`

```kiwi
insert(value: any)
```

Inserts a value into the heap.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `value` | `any` |  | The value to insert. |

### `.extract_root`

```kiwi
extract_root(): any
```

Removes and returns the root element (minimum for min-heap, maximum for max-heap).

**Returns** `any` — The root element.

**Throws** `string` — If the heap is empty.

### `.peek`

```kiwi
peek(): any
```

Returns the root element without removing it.

**Returns** `any` — The root element.

**Throws** `string` — If the heap is empty.

### `.size`

```kiwi
size(): integer
```

Returns the number of elements in the heap.

**Returns** `integer` — The heap size.

### `.is_empty`

```kiwi
is_empty(): boolean
```

Returns true if the heap contains no elements.

**Returns** `boolean` — `true` if the heap is empty.

## `Set` struct

An unordered collection of unique values with set-algebra operations.

```kiwi
s = Set.new([1, 2, 3])
s.add(4)
println s.contains(2)  # true
println s.size()       # 4
```

### `Set.new`

```kiwi
Set.new(data: list = [])
```

Creates a new set, optionally initialized from a list.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `list` | `[]` | Initial values; duplicates are removed (default `[]`). |

### `.add`

```kiwi
add(item: any)
```

Adds an item to the set (no-op if already present).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `item` | `any` |  | The value to add. |

### `.remove`

```kiwi
remove(item: any)
```

Removes an item from the set.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `item` | `any` |  | The value to remove. |

### `.contains`

```kiwi
contains(item: any): boolean
```

Returns true if the set contains the given item.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `item` | `any` |  | The value to check. |

**Returns** `boolean` — `true` if the item is present.

### `.size`

```kiwi
size(): integer
```

Returns the number of elements in the set.

**Returns** `integer` — The set size.

### `.clear`

```kiwi
clear()
```

Removes all elements from the set.

### `.difference`

```kiwi
difference(data: any)
```

Returns a new set containing elements in this set but not in `data`.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `any` |  | The set or list to subtract. |

**Returns** `Set` — The difference set.

### `.disjoint`

```kiwi
disjoint(data: any)
```

Returns true if this set and `data` share no common elements.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `any` |  | The set or list to compare against. |

**Returns** `boolean` — `true` if the sets are disjoint.

### `.intersect`

```kiwi
intersect(data: any)
```

Returns true if this set and `data` share at least one common element.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `any` |  | The set or list to compare against. |

**Returns** `boolean` — `true` if the sets intersect.

### `.merge`

```kiwi
merge(data: any)
```

Merges `data` into this set in place (modifies the set).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `any` |  | The set or list to merge in. |

### `.union`

```kiwi
union(data: any): object
```

Returns a new set containing all elements from both this set and `data`.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `any` |  | The set or list to union with. |

**Returns** `Set` — A new set with all unique elements from both.

### `.subset`

```kiwi
subset(data: any): boolean
```

Returns true if this set is a subset of `data`.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `any` |  | The potential superset. |

**Returns** `boolean` — `true` if every element of this set is in `data`.

### `.superset`

```kiwi
superset(data: any): boolean
```

Returns true if this set is a superset of `data`.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `any` |  | The potential subset. |

**Returns** `boolean` — `true` if every element of `data` is in this set.

### `.to_list`

```kiwi
to_list(): list
```

Returns the set contents as a list.

**Returns** `list` — A list of all elements in the set.

### `.to_string`

```kiwi
to_string(): string
```

Returns a string representation of the set.

**Returns** `string` — The set contents as a string.

### `.type`

```kiwi
type(): string
```

Returns the type name of this object.

**Returns** `string` — Always `"Set"`.
