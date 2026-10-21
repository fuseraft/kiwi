# Lists and Ranges

> Creating, indexing, slicing, transforming and iterating lists, and building ranges.

A list is an ordered, mutable sequence. It can hold values of any type, including other lists and hashmaps, and it grows as needed.

```kiwi
empty  = []
fruits = ["kiwi", "mango", "lime"]
mixed  = [1, "two", 3.0, [4], {five: 5}]

println fruits.size()   # 3
```

## Ranges

`[start to end]` builds a list of consecutive integers. `..` is a synonym for `to`. Both ends are included, and if `end` is smaller than `start` the range counts down.

```kiwi
println [1 to 5]     # [1, 2, 3, 4, 5]
println [1..5]       # [1, 2, 3, 4, 5]
println [5 to 1]     # [5, 4, 3, 2, 1]
```

A `for` loop over a range literal counts without building the list, so `for i in [1 to 100000000] do` uses constant memory.

:::caution
A range never comes out empty. `[0 to n - 1]` is `[0, -1]` when `n` is `0`, because the range counts down. Guard loops whose bounds can cross:

```kiwi
n = 0
for i in [0 to n - 1] do
  break when n == 0
  println i
end
```
:::

Ranges also work in `case` without brackets; see [Control Flow](control-flow.md#ranges).

## Indexing

Indexes start at 0. Reading or writing past the end throws an `IndexError`; use `.push()` to grow a list.

```kiwi
colors = ["red", "green", "blue"]
println colors[0]           # red
println colors[2]           # blue

colors[1] = "emerald"
println colors              # ["red", "emerald", "blue"]
```

Negative indexes are not supported. Use `.last()`, or `xs[xs.size() - 1]`.

## Slicing

`xs[start:end]` returns a new list from `start` up to, but not including, `end`. Leave out either bound to slice from the beginning or to the end. A third value is the step.

```kiwi
nums = [0, 1, 2, 3, 4, 5]
println nums[1:4]    # [1, 2, 3]
println nums[:2]     # [0, 1]
println nums[3:]     # [3, 4, 5]
println nums[::2]    # [0, 2, 4]
println nums[::-1]   # [5, 4, 3, 2, 1, 0]
```

Assigning a list to a slice replaces those elements. The replacement can be shorter or longer than the slice, so the list shrinks or grows, and assigning to an empty slice inserts:

```kiwi
letters = ["a", "b", "c", "d"]
letters[1:3] = ["X", "Y"]
println letters   # ["a", "X", "Y", "d"]

letters[1:3] = ["Z"]
println letters   # ["a", "Z", "d"]

letters[1:1] = ["new"]
println letters   # ["a", "new", "Z", "d"]
```

A slice with a step (`xs[::2] = [...]`) overwrites the selected elements one by one instead.

## Adding and removing

```kiwi
stack = [1, 2]
stack.push(3)            # add to the end
stack.unshift(0)         # add to the front
println stack            # [0, 1, 2, 3]

println stack.pop()      # 3  (remove from the end)
println stack.shift()    # 0  (remove from the front)
println stack            # [1, 2]

stack.insert(1, "x")     # insert at an index
println stack            # [1, "x", 2]

stack.remove("x")        # remove the first matching value
stack.remove_at(0)       # remove by index
println stack            # [2]

stack.concat([3, 4])     # append every element of another list
println stack            # [2, 3, 4]
```

`.push(list)` adds the list as one element; `.concat(list)` adds its elements. `.enqueue()` and `.dequeue()` are synonyms for `push` and `shift`, for queue-style code.

## Transforming

`.map`, `.filter` and `.reduce` take a [lambda](lambdas.md) and return new values without changing the original list:

```kiwi
nums = [1, 2, 3, 4, 5]

println nums.map(do (n) => n * n)              # [1, 4, 9, 16, 25]
println nums.filter(do (n) => n % 2 == 1)      # [1, 3, 5]
println nums.reduce(0, do (acc, n) => acc + n) # 15
println nums.all(do (n) => n > 0)              # true
println nums                                   # [1, 2, 3, 4, 5]
```

Chain them to build pipelines. Start each continued line with `.`:

```kiwi
result = [1 to 10]
  .filter(do (n) => n % 2 == 1)
  .map(do (n) => n * 10)
  .take(3)

println result   # [10, 30, 50]
```

Named functions work anywhere a lambda does:

```kiwi
fn double(n)
  n * 2
end

println [1, 2, 3].map(double)   # [2, 4, 6]
```

`.each` runs a lambda for every element and passes the index as an optional second parameter:

```kiwi
["a", "b"].each(do (value, i)
  println "${i}: ${value}"
end)
```

## Sorting and searching

```kiwi
words = ["kiwi", "fig", "banana"]

println words.contains("fig")    # true
println words.index("banana")    # 2
println words.first()            # kiwi
println words.last()             # banana

words.sort()
println words                    # ["banana", "fig", "kiwi"]

words.sort(do (a, b) => a.size() < b.size())
println words                    # ["fig", "kiwi", "banana"]

println [3, 9, 4].max()          # 9
println [3, 9, 4].sum()          # 16
println [1, 1, 2].unique()       # [1, 2]
```

`.sort()`, `.reverse()`, `.rotate()` and `.swap()` change the list in place (and also return it). Sort a `.clone()` if you need to keep the original order. A comparator lambda receives two elements and returns `true` when the first should come first.

The [`list`](../lib/list.md) package adds more helpers, several of them callable as methods:

```kiwi
nums = [1, 2, 3, 4]
println nums.reject(do (n) => n % 2 == 0)       # [1, 3]
println nums.first_where(do (n) => n > 2)       # 3
println list::count(nums, do (n) => n > 1)      # 3
```

The [Built-in Methods](../reference/builtin-methods.md#lists) reference lists every list method.

## Iterating

```kiwi
planets = ["Mercury", "Venus", "Earth"]

for planet in planets do
  println planet
end

for planet, i in planets do
  println "${i + 1}. ${planet}"
end
```

Loop guards keep filtering logic flat:

```kiwi
for n in [1 to 10] do
  next when n % 3 != 0
  println n   # 3, 6, 9
end
```

Don't add or remove elements of a list while a `for` loop walks it. Iterate over a `.clone()`, or build a new list instead.

## Lists are shared

Assigning a list to another variable, passing it to a function, or reading it out of another collection does not copy it. Every name refers to the same list, so a change through one is visible through all of them.

```kiwi
original = [1, 2]
alias = original
alias.push(3)
println original   # [1, 2, 3]

grid = [[1, 2], [3, 4]]
row = grid[0]
row.push(99)
println grid       # [[1, 2, 99], [3, 4]]
```

Use `.clone()` for an independent copy. It copies nested lists and hashmaps too.

```kiwi
original = [[1], [2]]
copy = original.clone()
copy[0].push(100)
println original   # [[1], [2]]
println copy       # [[1, 100], [2]]
```

Lists and hashmaps are safe to share between [tasks](../lib/task.md).
