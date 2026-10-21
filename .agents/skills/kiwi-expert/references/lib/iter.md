# iter

> ListIterator, a cursor for stepping through a list with peeking and rewinding.

The `iter` package provides `ListIterator`, a cursor over a list. It is handy for hand-written parsers and tokenizers, where you need to look ahead, skip, or step back rather than visit each element once. Lists also have an `.iterator()` method that creates one.

```kiwi
tokens = ["fn", "greet", "(", "name", ")", "end"]
it = tokens.iterator()

while it.can_read() do
  print "${it.consume()} "
end
println ""   # fn greet ( name ) end
```

## Looking around

```kiwi
it = ListIterator.new([1, 2, 3])

while it.can_read() do
  println "${it.current()} then ${it.peek()}"
  it.consume()
end
# 1 then 2
# 2 then 3
# 3 then null
```

| Method | Does |
| :-- | :-- |
| `current()` | Returns the element at the cursor. |
| `peek()` | Returns the next element without moving. |
| `consume()` | Returns the current element and moves forward. |
| `skip(n)`, `rewind(n)` | Moves forward or back `n` places. |
| `reset()` | Moves back to the start. |
| `can_read()`, `eof()` | Whether elements remain. |
| `position()`, `length()` | The cursor position and the list length. |
| `remaining()` | The elements from the cursor on. |
| `grab(index)` | The element at an absolute index, or the default. |
| `reach(offset)` | The element at an offset from the cursor; negative offsets count back. |

```kiwi
it = ListIterator.new(["a", "b", "c", "d", "e"])
it.skip(2)
println it.current()     # c
it.rewind(1)
println it.current()     # b
println it.remaining()   # ["b", "c", "d", "e"]
println it.grab(4)       # e
println it.grab(99)      # null: out of range returns the default
```

`ListIterator.new(list, default)` sets the value returned when reading past either end; it is `null` unless you pass one.

For iterating over a string character by character, see `StringIterator` in [`string`](string.md).

## `ListIterator` struct

An implementation of the iterator pattern.

### `ListIterator.new`

```kiwi
ListIterator.new(list_data: list, default: any = null)
```

Creates a new iterator over a list.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `list_data` | `list` |  | The list to iterate. |
| `default` | `any` | `null` | Value returned when out of bounds (default: null) |

### `.consume`

```kiwi
consume(n: integer = 0): any
```

Advance the cursor by n.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `n` | `integer` | `0` | The number of values to advance. |

**Returns** `any` — Returns the item before advancing.

### `.skip`

```kiwi
skip(n: integer = 1)
```

Skip forward n elements (clamped to end).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `n` | `integer` | `1` | The number of values to skip. |

### `.rewind`

```kiwi
rewind(n: integer = 1)
```

Rewind backward n elements (clamped to 0).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `n` | `integer` | `1` | The number of values to skip. |

### `.can_read`

```kiwi
can_read(): boolean
```

Checks if not at the end of the list.

**Returns** `boolean` — Returns `true` when the position is before the end.

### `.eof`

```kiwi
eof(): boolean
```

Checks if at the end of the list.

**Returns** `boolean` — Returns `true` when the position is the end.

### `.current`

```kiwi
current(): any
```

Current value (or default if out of bounds).

**Returns** `any` — Returns the current value, or the default if out of bounds.

### `.peek`

```kiwi
peek(): any
```

Peek the next value without consuming.

**Returns** `any` — Returns the next value in the list, or the default if out of bounds.

### `.reset`

```kiwi
reset()
```

Resets position to 0.

### `.remaining`

```kiwi
remaining(): list
```

Gets remaining values as a new list.

**Returns** `list` — The remaining values at the current position onward, or an empty list if at the end.

### `.grab`

```kiwi
grab(index: integer): any
```

Gets value at absolute index (safe).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `index` | `integer` |  | The index of the list to grab. |

**Returns** `any` — Returns the value at the specified index, or default if index out of bounds.

### `.reach`

```kiwi
reach(offset: integer): any
```

Gets value relative to end: reach(-1) = last element.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `offset` | `integer` |  | The offset. |

**Returns** `any` — Returns the value relative to the end. Default if out of bounds.

### `.position`

```kiwi
position(): integer
```

Gets the current position (0-based).

**Returns** `integer` — The current position in the list.

### `.length`

```kiwi
length(): integer
```

Gets the total length of list.

**Returns** `integer` — Returns the length of the list.

### `.size`

```kiwi
size(): integer
```

Gets the total length of list.

**Returns** `integer` — Returns the length of the list.

### `.skip_defaults`

```kiwi
skip_defaults()
```

Skips over consecutive default values.

### `.look_ahead`

```kiwi
look_ahead(): any
```

Look ahead: first non-default value from current position.

**Returns** `any` — The first non-default value from the current position, or default if out of bounds.

### `.count_ahead`

```kiwi
count_ahead(): integer
```

Counts how many default values are ahead.

**Returns** `integer` — Returns how many default values are ahead.
