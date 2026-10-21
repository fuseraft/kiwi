---
title: Loops
description: for, while and repeat loops, plus break and next.
---

Kiwi has three loops. Each opens its body with `do` and closes it with `end`.

| Loop | Use it for |
| :-- | :-- |
| `for ... in ... do` | Walking a list, range, string, hashmap or generator |
| `while condition do` | Repeating until a condition becomes false |
| `repeat n do` | Running a block a fixed number of times |

## `for`

`for` iterates over a collection. Add a second variable to get the index (or, for hashmaps, the value).

```kiwi
fruits = ["kiwi", "mango", "lime"]

for fruit in fruits do
  println fruit
end

for fruit, i in fruits do
  println "${i}: ${fruit}"   # 0: kiwi, 1: mango, 2: lime
end
```

It works on every iterable type:

```kiwi
# Ranges count without building a list.
for i in [1 to 3] do
  print "${i} "
end
println ""                 # 1 2 3

# Strings yield one character at a time.
for ch in "abc" do
  print ch.uppercase()
end
println ""                 # ABC

# Hashmaps yield keys, or key-value pairs.
scores = {alice: 95, bob: 82}
for name, score in scores do
  println "${name} scored ${score}"
end
```

[Generators](/kiwi/language/generators/) are consumed one value at a time, so a `for` loop can walk an infinite sequence as long as it breaks out.

## `while`

`while` repeats as long as its condition is truthy.

```kiwi
count = 1
while count <= 3 do
  println count
  count += 1
end
```

`while true do` with a `break when` inside is the idiom for a loop whose exit is in the middle:

```kiwi
attempts = 0
while true do
  attempts += 1
  break when attempts >= 3
end
println attempts   # 3
```

## `repeat`

`repeat n do` runs the body `n` times. `as` names a counter that runs from 1 to `n`. A count of zero or less runs nothing.

```kiwi
repeat 2 do
  println "Hello"
end

total = 0
repeat 10 as i do
  total += i
end
println total   # 55
```

## `break` and `next`

`break` leaves the innermost loop. `next` skips to the next iteration. Both take an optional `when` guard, which usually reads better than wrapping them in `if`.

```kiwi
for n in [1 to 20] do
  next when n % 3 == 0    # skip multiples of 3
  break when n > 10       # stop after 10
  print "${n} "
end
println ""                # 1 2 4 5 7 8 10
```

In nested loops, `break` and `next` affect only the loop they're in:

```kiwi
for row in [1, 2] do
  for col in [1, 2, 3] do
    break when col == 2
    print "${row}${col} "
  end
end
println ""   # 11 21
```

## Loop variables

The loop variable keeps its last value after the loop ends, and variables first assigned in the body stay visible too. See [Scoping](/kiwi/language/scoping/).

```kiwi
for n in [1, 2, 3] do
  last = n
end
println last   # 3
```

## Tips

- Prefer `for` over index arithmetic with `while`.
- Don't add or remove elements of the list you are iterating; loop over a `.clone()` instead.
- For transformations, a list method such as `.map`, `.filter` or `.reduce` is often clearer than a loop. See [Lists](/kiwi/language/lists/#transforming).
