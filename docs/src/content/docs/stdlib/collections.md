---
title: collections
description: Heap (priority queue) and Set data structures.
---

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

<!-- api: collections -->
