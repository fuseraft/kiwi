---
title: list
description: Predicates and helpers for lists, also callable as list methods.
---

The `list` package adds predicate helpers and searches to lists. Because the package [extends the `list` type](/kiwi/language/packages/#extending-built-in-types), most functions can also be called as methods:

```kiwi
nums = [3, 8, 1, 8, 5]

println nums.reject(do (n) => n > 4)          # [3, 1]
println nums.first_where(do (n) => n > 4)     # 8
println nums.last_where(do (n) => n > 4)      # 5
println nums.one(do (n) => n == 1)            # true
println nums.is_unique()                      # false
println nums.index_of(8)                      # 1
println nums.last_index_of(8)                 # 3
println [1, 2.5, "x", 3].sum_integer()        # 4
```

A few functions share their names with [built-in methods](/kiwi/reference/builtin-methods/#lists) or type keywords. The built-in wins when called as a method, so call these with the package prefix:

```kiwi
nums = [3, 8, 1, 8, 5]

println list::any(nums, do (n) => n > 7)      # true
println list::none(nums, do (n) => n > 9)     # true
println list::all(nums, do (n) => n > 0)      # true
println list::count(nums, do (n) => n == 8)   # 2
println list::find(nums, do (n) => n == 8)    # 1 (the index)
```

`list::iterator(xs)`, or `xs.iterator()`, returns a [`ListIterator`](/kiwi/stdlib/iter/).

<!-- api: list -->
