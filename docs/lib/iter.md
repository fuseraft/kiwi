# `iter`

The `iter` package provides a flexible, explicit **iterator pattern** implementation for safely and efficiently traversing lists in Kiwi.

Unlike Kiwi's built-in `for ... in ...` loops (which consume iterators implicitly), `ListIterator` gives you full manual control over position, peeking, skipping, rewinding, bounds-safe access, and default-value handling, making it especially useful for:

- parsers / tokenizers
- stateful streaming algorithms
- lookahead / backtracking
- safe indexing with fallback values
- skipping sections (whitespace, comments, etc.)

---

## `ListIterator` struct

An explicit, bidirectional, bounds-safe iterator over a list with configurable default value for out-of-bounds access.

### Constructor

Creates a new iterator wrapping a list.

**Parameters**

| Type     | Name         | Description                                      | Default   |
|----------|--------------|--------------------------------------------------|-----------|
| `list`   | `list_data`  | The list to iterate over                         | —         |
| `any`    | `default`    | Value returned on out-of-bounds access           | `null`    |

**Example**
```kiwi
it = ListIterator.new(["apple", "banana", "cherry"], default: "")
```

### Navigation

#### `consume(n)`
Returns the **current** value and then advances the position by `n+1` steps (or 1 if `n=0`).

**Parameters**

| Type      | Name | Description                     | Default |
|-----------|------|---------------------------------|---------|
| `integer` | `n`  | Extra steps to skip after read  | `0`     |

**Returns**  

| Type | Description                      |
|------|----------------------------------|
| `any`| The value that was just consumed |

#### `skip(n)`
Moves forward `n` positions (clamped — never goes past end).

**Parameters**

| Type      | Name | Default |
|-----------|------|---------|
| `integer` | `n`  | `1`     |

#### `rewind(n)`
Moves backward `n` positions (clamped — never goes before 0).

**Parameters**

| Type      | Name | Default |
|-----------|------|---------|
| `integer` | `n`  | `1`     |

#### `reset()`
Moves position back to the beginning (`pos = 0`).

### State queries

#### `can_read()`
**Returns** `true` if there is at least one more element ahead.

**Returns** `boolean`

#### `eof()`
**Returns** `true` if the iterator has reached (or passed) the end.

**Returns** `boolean`

#### `position()`
Current 0-based index.

**Returns** `integer`

#### `length()` / `size()`
Total number of elements in the underlying list.

**Returns** `integer`

### Accessing values

#### `current()`
Value at the current position, or `default` if past end.

**Returns** `any`

#### `peek()`
Value at the next position (`pos + 1`), or `default` if none.

**Returns** `any`

#### `grab(index)`
Absolute-index access — safe, returns `default` if out of bounds.

**Parameters**

| Type      | Name    |
|-----------|---------|
| `integer` | `index` |

**Returns** `any`

#### `reach(offset)`
Relative-to-end access: `reach(-1)` → last element, `reach(-2)` → second-to-last, etc.  
Returns `default` if result is out of bounds.

**Parameters**

| Type      | Name     |
|-----------|----------|
| `integer` | `offset` |

**Returns** `any`

#### `remaining()`
Returns a new list containing all elements from current position to end  
(or empty list if already at end).

**Returns** `list`

### Default-value helpers

Useful when iterating over padded / sentinel-terminated data (e.g. token streams, config parsers).

#### `skip_defaults()`
Advances past any consecutive `default` values starting from current position.

#### `look_ahead()`
Returns the first **non-default** value ahead of current position  
(or `default` if none found).

**Returns** `any`

#### `count_ahead()`
Counts how many consecutive `default` values are ahead starting from current position.

**Returns** `integer`
