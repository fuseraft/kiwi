# Lists in Kiwi

Lists are dynamic, ordered, mutable sequences that can hold values of any type (including other lists, hashmaps, etc.).
They grow automatically as needed and support slicing, and many useful built-in methods.

---

## Quick Reference – Common Operations

| Operation                  | Syntax / Method                          | Returns          | Mutates? |
|----------------------------|------------------------------------------|------------------|----------|
| Create                     | `[]`, `[1, 2, 3]`, `[1 to 10]`           | `list`           | —        |
| Get element                | `lst[3]`, `lst[0]`                       | value            | No       |
| Set element                | `lst[2] = "new"`                         | —                | Yes      |
| Append                     | `lst.push(value)`, `lst.concat(values)`  | —                | Yes      |
| Remove by index            | `lst.remove_at(idx)`                     | removed value    | Yes      |
| Remove by value            | `lst.remove(value)`                      | —                | Yes      |
| Filter                     | `lst.filter(λ)`                          | new list         | No       |
| Map                        | `lst.map(λ)`                             | new list         | No       |
| Length                     | `lst.size()` or `lst.length()`           | integer          | No       |
| Iterate                    | `for item in lst do …`                   | —                | —        |
| Iterate with index         | `for item, idx in lst do …`              | —                | —        |

For the full list of built-in methods, see [List Builtins](builtins.md#list-builtins).

## Creating Lists

### Literal syntax

```kiwi
empty   = []
fruits  = ["kiwi", "mango", "lime"]
mixed   = [1, "hello", true, [2, 3], {a: 42}]
```

### From a range (very common idiom)

```kiwi
digits  = [0 to 9]           # [0,1,2,3,4,5,6,7,8,9]
reverse = [10 to 1]          # [10,9,8,...,1]
```

See [Ranges](ranges.md).

## Accessing & Modifying Elements

Indices start at **0**..

```kiwi
colors = ["red", "green", "blue", "yellow"]

println colors[0]    # "red"
println colors[2]    # "blue"
println colors[3]    # "yellow" (last)

# Modify in place
colors[1] = "emerald"
println colors        # ["red", "emerald", "blue", "yellow"]
```

**Out-of-bounds behavior**  
Reading past the end throws `IndexError`.  
Writing past the end throws an error (use `.push()` to grow).

## Adding Elements

### Append (most common)

```kiwi
tasks = ["write docs", "refactor"]
tasks.push("add tests")
tasks.push(42)                # lists can be heterogeneous

println tasks.size()          # 4
```

### Append multiple

```kiwi
more = ["deploy", "celebrate"]
tasks.push(more)              # adds the whole list as one element
# ["write docs", "refactor", "add tests", 42, ["deploy", "celebrate"]]

tasks.concat(more)            # use concat to combine two lists
# ["write docs", "refactor", "add tests", 42, "deploy", "celebrate"]
```

## Removing Elements

### By index

```kiwi
letters = "abcdef".chars()     # ["a","b","c","d","e","f"]
letters.remove_at(3)           # removes "d"
println letters                # ["a","b","c","e","f"]
```

### By value (removes first occurrence)

```kiwi
numbers = [1, 2, 3, 2, 4]
numbers.remove(2)
println numbers                # [1, 3, 2, 4]  (only first 2 removed)
```

### Remove all occurrences

```kiwi
while numbers.contains(2) do
  numbers.remove(2)
end
```

## Transforming Lists (non-destructive)

### Filter

```kiwi
words = ["kiwi", "apple", "banana", "mango", "lime"]
short = words.filter(do (w) => w.size() <= 4)
println short          # ["kiwi", "lime"]
```

### Map

```kiwi
nums = [1, 2, 3, 4]
squares = nums.map(do (n) => n * n)
println squares        # [1, 4, 9, 16]
```

### Chaining

```kiwi
result = [1 to 10]
  .filter(do (n) => n % 2 == 1)     # odds
  .map(do (n) => n * 10)            # ×10
println result                      # [10, 30, 50, 70, 90]
```

A list literal can also be used directly as a statement — calling a method on it without assigning the result, or combined with a `when` guard:

```kiwi
[].size()                           # valid statement (result discarded)
["a", "b"].size() when some_flag    # guarded call
```

> **Note on line breaks and `[`:** A `[` at the start of a new line is always parsed as a new list literal, never as an index operator on the expression above it. Same-line indexing (`lst[i]`) is unaffected.
>
> ```kiwi
> x = my_list      # x gets the list
> [0].size()        # separate statement — NOT my_list[0].size()
> ```

## Iterating Lists

### Basic iteration

```kiwi
planets = ["Mercury", "Venus", "Earth", "Mars"]

for p in planets do
  println p
end
```

### With index

```kiwi
for planet, i in planets do
  println "${i + 1}. ${planet}"
end
# 1. Mercury
# 2. Venus
# ...
```

### With guard clauses (idiomatic)

```kiwi
for n in [1 to 20] do
  next when n % 3 == 0     # skip multiples of 3
  println n
end
```

## Full Example – Word frequency

```kiwi
text = "kiwi is fun kiwi is awesome kiwi wins"
words = text.split(" ")

counts = {}

for word in words do
  word = word.lowercase()
  next when word.empty()

  counts[word] = counts.get(word, 0) + 1
end

println counts
# { "kiwi": 3, "is": 2, "fun": 1, "awesome": 1, "wins": 1 }
```

## Mutating Nested Collections

**Direct chained mutation** on an indexed element works correctly — the change is reflected in the original container:

```kiwi
matrix = [[1, 2, 3], [4, 5, 6]]
matrix[0].push(99)
println matrix    # [[1, 2, 3, 99], [4, 5, 6]]
```

However, **extracting a collection to a variable** gives you a copy. Mutations to that copy are not reflected in the original — you must write it back.

```kiwi
matrix = [[1, 2, 3], [4, 5, 6]]

row = matrix[0]   # row is a copy
row.push(99)      # modifies only the copy
println matrix    # [[1, 2, 3], [4, 5, 6]] — unchanged!

row = matrix[0]
row.push(99)
matrix[0] = row   # write back — required
println matrix    # [[1, 2, 3, 99], [4, 5, 6]]
```

The write-back rule applies to lists-of-hashmaps, nested lists, or any other extracted collection. When in doubt, prefer direct chained operations, or extract-mutate-reassign explicitly.

## Best Practices & Tips

- Chain `.filter()`, `.map()`, `.take()`, etc. for clean data pipelines
- Avoid modifying a list while iterating over it with `for` — use indices or `.clone()`
- For very large lists, prefer iterators [`ListIterator`](lib/iter.md) when you only need sequential access

See also:
- [List Builtins](builtins.md#list-builtins)
- [Ranges](ranges.md)
- [Hashmaps](hashmaps.md)
- [Loops – `for`](loops.md#3-for--iteration-over-collections--ranges)