---
title: Packages and Modules
description: Grouping code into packages, splitting programs across files, and extending built-in types.
---

A package groups related functions, structs and variables under a name. The standard library is a collection of packages, and your own code can be organized the same way.

## Declaring a package

```kiwi
package greeter
  fn hello(name)
    "Hello, ${name}."
  end

  fn excited(name)
    hello(name).replace(".", "!")
  end
end

println greeter::hello("World")     # Hello, World.
println greeter::excited("World")   # Hello, World!
```

Call package functions from outside with `package::name`. Inside the package, functions call each other by their plain names, as `excited` calls `hello` above. A plain name inside a package finds the package's own functions and constants first, so a global variable that happens to share the name doesn't get in the way.

A package is ready to use as soon as its declaration has run.

## Package variables

Assignments at the top level of a package body create package variables, read from anywhere as `package::NAME`. Use `const` for values that must not change.

```kiwi
package config
  const VERSION = "2.1.0"
  host = "localhost"
  port = 8080

  fn url()
    "http://${config::host}:${config::port}"
  end
end

println config::VERSION   # 2.1.0
println config::url()     # http://localhost:8080
```

A `const` belongs to its package: outside the package it is only reachable as `package::NAME`, so it never collides with a constant or variable of the same name elsewhere. Inside the package the plain name works too.

Variables assigned inside a package *function* are local to that call, like in any function.

## Nested packages

Qualify a package name with `::` to nest it, or declare one package inside another:

```kiwi
package app
  fn version()
    "1.0"
  end

  package text
    fn shout(s)
      s.uppercase()
    end
  end
end

package app::math
  fn double(n)
    n * 2
  end
end

println app::version()         # 1.0
println app::text::shout("hi") # HI
println app::math::double(21)  # 42
```

## Package references

`import "name"` returns a reference to a package. Call its functions with a dot, pass it around, or store it. The `::` form keeps working alongside it.

```kiwi
m = import "math"
println m.abs(-7)          # 7

package formats
  fn csv_line(values)
    values.join(",")
  end
end

fmt = import "formats"
println fmt.csv_line([1, 2, 3])   # 1,2,3
```

## Splitting code across files

Put a package in its own file and load it with `import` or `include`. Paths are relative to the file doing the loading, and the `.kiwi` extension is optional.

```kiwi nocheck
# lib/greeter.kiwi
package greeter
  fn hello(name)
    "Hello, ${name}!"
  end
end
```

```kiwi nocheck
# main.kiwi
import "lib/greeter"

println greeter::hello("World")
```

Each file is loaded at most once, however many times it is imported or included. Both keywords run the file's top-level code and make its packages, functions and structs available.

| Statement | Effect |
| :-- | :-- |
| `import "name"` | Activates the package `name`, or loads the file `name.kiwi` if there is no such package. Returns a package reference. |
| `include "path"` | Loads the file at `path`. |
| `require "name"` | Throws `PackageUndefinedError` unless the package `name` is available. Loads nothing. |
| `export "name"` | Activates the package `name`. Kept for compatibility; packages are active once declared. |

The whole standard library is available without importing it.

## Extending built-in types

A package named after a built-in type (`string`, `list`, `hashmap`, `integer`, `float`, `boolean`, `date` or `bytes`) adds methods to that type. Every function in the package becomes callable as a method on values of that type, with the value passed as the first argument.

```kiwi
package string
  fn word_count(text: string): integer
    text.split(" ").filter(do (w) => !w.empty()).size()
  end

  fn shout(text: string): string
    text.uppercase() + "!"
  end
end

println "hello big world".word_count()   # 3
println "kiwi".shout()                   # KIWI!
println string::shout("still works")     # STILL WORKS!
```

The standard library uses this for [`string`](/kiwi/stdlib/string/), [`list`](/kiwi/stdlib/list/) and [`date`](/kiwi/stdlib/date/). For example `"some title".titleize()`, `"42".padstart(5, "0")` and `today.add_days(7)` all come from stdlib packages.

Keep in mind:

- Hinting the first parameter (`text: string`) isn't required, but it documents the intent.
- [Built-in methods](/kiwi/reference/builtin-methods/) take precedence. An extension named `count` or `second` is shadowed by the built-in of the same name when called as a method, unless its last argument is a lambda; call it as `package::name(...)` instead, or pick another name.
- If two loaded packages add the same method name to a type, the last one loaded wins.

## Names that are type keywords

Function names that are also [type keywords](/kiwi/language/syntax/#reserved-words), such as `any` or `none`, can be declared in a package. Call them with the package prefix (even from inside the package) or as a method (`xs.none(fn)`); a bare `none(...)` doesn't parse. Choosing a different name avoids the problem.
