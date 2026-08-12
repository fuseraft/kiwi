# Loops in Kiwi

Kiwi provides three main looping constructs: `repeat`, `while`, and `for`.
All loops support `break` (exit early) and `next` (skip to next iteration).

## Quick Reference

| Construct     | Use case                              | Iterator var? | Index available? | Infinite possible? |
|---------------|---------------------------------------|---------------|------------------|--------------------|
| `repeat n`    | Known number of repetitions           | Yes (via `as`) | Yes              | No                 |
| `while`       | Condition-based (including infinite)  | No            | Manual           | Yes                |
| `for … in …`  | Collection / range / generator        | Yes           | Yes (optional)   | Yes (generators)   |

## 1. `repeat` – Fixed number of iterations

Simplest loop: run a block exactly `n` times (n must be a positive integer). 

### Basic syntax

```kiwi
repeat 5 do
  println "Hello"
end
# prints "Hello" five times
```

### With iterator variable (`as`)

The iterator starts at **1** and goes up to `n`.

```kiwi
sum = 0

repeat 10 as i do
  sum += i
end

println sum          # 55   (1+2+...+10)
```

**Note:** `repeat 0` is a no-op (does nothing).

## 2. `while` – Condition-based loop

Runs as long as the condition is `true`.

### Basic syntax

```kiwi
count = 1

while count <= 5 do
  println count
  count += 1
end
# 1 2 3 4 5
```

### Infinite loop

```kiwi
while true do
  print "\rWaiting... (Ctrl+C to exit)"
  task::sleep(400)
end
```

### Early exit with `break`

```kiwi
i = 0
while true do
  i += 1
  println i

  break when i >= 7   # clean guard style
end
# prints 1 through 7
```

### Skip iteration with `next`

```kiwi
i = 0
while i < 10 do
  i += 1

  next when i % 2 == 0   # skip even numbers

  println i              # only odd numbers
end
# 1 3 5 7 9
```

## 3. `for` – Iteration over collections, ranges & generators

Most common loop in Kiwi — used for lists, hashmaps, ranges, and generators.

### Syntax

```kiwi
for item in collection do
  # ...
end

# With index
for item, idx in collection do
  # idx starts at 0
end

# With key-value (hashmaps)
for key, value in hashmap do
  # ...
end
```

### Examples

#### Lists

```kiwi
fruits = ["apple", "banana", "cherry"]

for fruit in fruits do
  println fruit
end
# apple
# banana
# cherry

# With index
for fruit, i in fruits do
  println "${i + 1}. ${fruit}"
end
# 1. apple
# 2. banana
# 3. cherry
```

#### Ranges

```kiwi
for i in [1 to 5] do         # 1,2,3,4,5
  println i
end

for i in [10 to 1] do        # countdown
  print "${i} "
end
# 10 9 8 7 6 5 4 3 2 1
```

#### Hashmaps

```kiwi
scores = { alice: 95, bob: 82, carol: 100 }

# key-value pairs
for name, score in scores do
  println "${name}: ${score}"
end

# only keys
for name in scores.keys() do
  println name
end

# only values
for score in scores.values() do
  println score
end
```

### Early exit & skipping in `for`

```kiwi
for num in [1 to 20] do
  next when num % 3 == 0     # skip multiples of 3
  break when num > 15        # stop early

  println num
end
# 1 2 4 5 7 8 10 11 13 14
```

## Best practices & tips

- Prefer `for` when iterating collections or known ranges — it's the most idiomatic.
- Use `while true` + `break when …` for clean infinite/event loops.
- `repeat` is great for simple fixed-count tasks or generating sequences.
- Guard clauses (`break when`, `next when`) reduce nesting and improve readability.
- Avoid mutating the collection inside a `for` loop over it (use indices or `.clone()` if needed).

See also:
- [Ranges](ranges.md)
- [Lists](lists.md)
- [Hashmaps](hashmaps.md)
- [Generators](generators.md)
- [Control Flow – `when` guards](control_structures.md#3-when--guard-clauses--conditional-modifiers)
