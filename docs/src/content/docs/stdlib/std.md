---
title: std
description: Small general-purpose helpers for type checks and ranges.
---

The `std` package holds a few general helpers, mostly function-style versions of built-in methods.

```kiwi
println std::len("kiwi")        # 4
println std::type([1, 2])       # list
println std::is_int(3)          # true
println std::is_float(3)        # false
println std::is_string("x")     # true
println std::is_hashmap({})     # true
println std::range(1, 5)        # [1, 2, 3, 4]
```

`std::range(x, y)` is end-exclusive, unlike the `[x to y]` [range literal](/kiwi/language/lists/#ranges), which includes `y`.

Most code uses the built-in equivalents directly: `.size()`, `typeof`, and `.is_a(type)`.

<!-- api: std -->
