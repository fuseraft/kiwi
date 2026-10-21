---
title: iter
description: ListIterator, a cursor for stepping through a list with peeking and rewinding.
---

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

For iterating over a string character by character, see `StringIterator` in [`string`](/kiwi/stdlib/string/).

<!-- api: iter -->
