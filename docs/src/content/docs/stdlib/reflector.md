---
title: reflector
description: Inspect functions and program state at runtime.
---

The `reflector` package offers runtime introspection: looking up functions by name, examining the program's state, and wrapping callables.

```kiwi
fn square(n)
  n * n
end

f = reflector::getfunc("square")   # look a function up by name
println f(4)                        # 16

println reflector::state().keys()   # ["packages", "structs", "functions", "stack"]
```

`state()` returns the names of the loaded packages, structs and functions, and the variables in each stack frame, which is useful for debugging tools and REPL-style helpers.

## Callable

`Callable` wraps a function together with a name, and exposes its parameters:

```kiwi
adder = Callable.new("adder", do (a, b) => a + b)

println adder.call([3, 7])                           # 10
println adder.parameters().map(do (p) => p["name"])  # ["a", "b"]
```

Every function value also has `.call(arguments)` and `.parameters()` directly; see [Built-in Methods](/kiwi/reference/builtin-methods/#functions-and-lambdas).

:::note
`reflector::callstack()` currently returns an empty list.
:::

<!-- api: reflector -->
