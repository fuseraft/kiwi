# Packages

Use the `package` keyword to define a package. Use `export` to make a package available, and `import` to bring a package into scope. Methods defined in a package are accessed using a fully qualified name (`package::method`).

---

## Defining and Using a Package

```kiwi
package greeter
  fn greet(name)
    println("Hello, ${name}.")
  end
end

package excited_greeter
  fn greet(name)
    println("Hello, ${name}!")
  end
end

# Make the packages available.
export "greeter"
export "excited_greeter"

# Invoke their methods.
greeter::greet("World")
excited_greeter::greet("World")
```

Output:
```
Hello, World.
Hello, World!
```

## File-Based Packages

To organize packages into separate files, define the package in its own `.kiwi` file and call `export` at the end of the file. Then use `include` in consuming scripts to load the file.

**`lib/greeter.kiwi`**
```kiwi
package greeter
  fn greet(name)
    println("Hello, ${name}!")
  end
end

export "greeter"
```

**`main.kiwi`**
```kiwi
include "lib/greeter"

greeter::greet("World")
```

## `export` is required

Defining a `package` block alone does **not** make its functions callable with `::`. The package must be activated with `export` (or `import`) before use. Without it, calling `pkg::fn()` raises an error.

```kiwi
package greeter
  fn hello(name)
    println "Hello, ${name}!"
  end
end

# greeter::hello("world")  ← error: package not exported yet

export "greeter"

greeter::hello("world")   # ✓ Hello, world!
```

Standard library packages (e.g. `math`, `fio`, `http`) are pre-exported automatically — you never need to export them yourself.

## `import` vs `export`

Both `import` and `export` activate a package and make its members accessible. By convention:

- Use `export "name"` inside the package file (self-export).
- Use `import "name"` in a consuming script when the package is already in scope.

```kiwi
package math_utils
  fn square(n)
    return n * n
  end
end

export "math_utils"

println math_utils::square(5)  # 25
```

## Import as a Variable

Assigning the result of `import` to a variable gives you a **package reference** — an object you can call methods on using dot syntax instead of `::`.

```kiwi
m = import "math"

println m.sqrt(16.0)      # 4.0
println m.abs(-7)         # 7
println m.pow(2.0, 10.0)  # 1024.0
```

This is exactly equivalent to calling `math::sqrt(16.0)` etc. — the `::` form still works alongside the variable reference and neither affects the other.

```kiwi
csv = import "csv"

rows = csv.parse("name,age\nAlice,30\nBob,25")

# :: syntax still works
rows2 = csv::parse("x,y\n1,2")
```

Package references can be stored in variables, passed to functions, and returned from functions:

```kiwi
fn make_parser(fmt)
  return case fmt
    when "csv": import "csv"
    when "xml": import "xml"
    else:       null
  end
end

parser = make_parser("csv")
result = parser.parse(data)
```

Overwriting the variable has no effect on the `::` namespace:

```kiwi
m = import "math"
m = "something else"        # variable reassigned
println math::sqrt(9.0)     # 3.0 — still works
```

Re-importing is idempotent — the package is loaded once; subsequent `import` calls for the same package return a new reference without re-executing the package body.

## Nested Packages

There are two ways to define a nested package.

### Inline `::` name

Declare the full qualified name directly in the `package` keyword:

```kiwi
package app::utils
  fn greet(name)
    println("Hello, ${name}!")
  end
end

export "app::utils"

app::utils::greet("World")
```

### Nested `package` blocks

Define a `package` block inside another `package` block. The inner package is automatically qualified under the outer one:

```kiwi
package app
  fn version()
    return "1.0"
  end

  package utils
    fn greet(name)
      println("Hello, ${name}!")
    end
  end
end

export "app"
export "app::utils"

app::version()              # 1.0
app::utils::greet("World")  # Hello, World!
```

### Importing a nested package

`import` accepts a fully-qualified package name and returns a package reference. Call its functions using dot syntax:

```kiwi
utils = import "app::utils"
utils.greet("World")  # Hello, World!
```

The `::` static call form is always available alongside the imported reference:

```kiwi
app::utils::greet("World")  # also works
```

---

## Package-Level Variables

Variables declared at the top level of a `package` body (outside any function) are **package-scoped**. After the package is exported, they are accessible via the fully-qualified `package::VAR` syntax.

```kiwi
package config
  HOST    = "localhost"
  PORT    = 8080
  VERSION = "2.1.0"

  fn url()
    "http://${config::HOST}:${config::PORT}"
  end
end

export "config"

println config::HOST     # localhost
println config::PORT     # 8080
println config::url()    # http://localhost:8080
```

Package variables are also reachable through an imported package reference:

```kiwi
cfg = import "config"
println cfg.HOST     # localhost
println cfg.VERSION  # 2.1.0
```

### Scope rules

Variables defined inside a package **function** remain local to that call and are never registered as package-level variables:

```kiwi
package counter
  total = 0   # package-level — accessible as counter::total

  fn increment()
    step = 1        # local to this call — not accessible as counter::step
    total += step
  end
end

export "counter"
```

### `const` vs regular package variables

Use `const` when the value must not be reassigned. Attempting to redefine a `const` raises an error; regular package variables can be updated freely.

```kiwi
package limits
  const MAX = 100   # immutable — redeclaring raises IllegalNameError
  default = 10      # mutable package variable
end

export "limits"

println limits::MAX      # 100
println limits::default  # 10
```

---

## Extending Built-in Types

When a package's name matches a built-in type name **and** a function's first parameter is typed as that type, Kiwi automatically makes the function callable as a method on any value of that type.

```kiwi
package list
  fn sum_positive(_list: list): integer
    _list.filter(do (n) => n > 0).sum()
  end
end

export "list"

nums = [-3, 1, -1, 4, 2]
println nums.sum_positive()   # 7
```

The function is reachable both ways:

```kiwi
nums.sum_positive()              # method syntax
list::sum_positive(nums)         # package syntax
```

### How it works

The dispatch rule has three parts:

1. **Package name** matches a built-in type: `list`, `string`, `date`, `integer`, `float`, `boolean`, `hashmap`, or `bytes`.
2. **First parameter** carries a matching type hint (e.g. `_s: string`).
3. **Caller** invokes it as `value.func(args...)` — Kiwi routes this to `package::func(value, args...)`.

If the first parameter has no type hint, or the package name does not match a type, the function is only reachable via the `package::func(...)` form.

### Extending `string`

```kiwi
package string
  fn word_count(_s: string): integer
    _s.trim().split(" ").size()
  end

  fn shout(_s: string): string
    _s.uppercase() + "!!!"
  end
end

export "string"

println "hello world".word_count()   # 2
println "kiwi".shout()               # KIWI!!!
```

### Extending `integer`

```kiwi
package integer
  fn factorial(_n: integer): integer
    return 1 when _n <= 1
    _n * (_n - 1).factorial()
  end
end

export "integer"

println (5).factorial()   # 120
println (10).factorial()  # 3628800
```

### Extending `date`

The standard library's `date` package uses this pattern to add arithmetic and accessors to every `date` value:

```kiwi
# lib/date.kiwi (stdlib)
package date
  fn add_days(_dt: date, n: integer): date
    time::add_days(_dt, n)
  end
end

export "date"
```

After loading, any `date` value gains `.add_days()`:

```kiwi
today     = time::now()
next_week = today.add_days(7)
```

### Standard library type extensions

The following stdlib packages use this pattern:

| Package  | Extends   | Key additions |
|----------|-----------|---------------|
| `list`   | `list`    | `all`, `_any`, `_none`, `one`, `reject`, `find`, `first_where`, `last_where`, `sum_integer`, `sum_float`, `iterator` |
| `string` | `string`  | `base64encode`, `base64decode`, `slug`, `padstart`, `padend`, `center`, `mirror`, `isalpha`, `isnumeric`, `islower`, `isupper` |
| `date`   | `date`    | `add_days`, `add_months`, `add_years`, `add_hours`, `year`, `month`, `day`, `weekday`, and more |

### Notes

- The first parameter name is conventional — it is not special. `_list`, `lst`, `self` all work equally.
- If two loaded packages extend the same type with the same function name, the last one loaded wins.
- Type extension only applies to built-in types. Struct methods are defined inside the struct body using `fn`.
- Functions with no type hint on the first parameter are still callable as `pkg::func(value)` but will **not** be dispatched as instance methods.

---

## Calling Sibling Functions Inside a Package

Functions defined inside the same package can call each other using their **unqualified** name — no `pkg::` prefix needed.

```kiwi
package math_utils
  fn double(x)
    x * 2
  end

  fn quadruple(x)
    double(double(x))   # calls sibling — no math_utils:: prefix needed
  end

  fn triple(x)
    double(x) + x
  end
end

export "math_utils"

println math_utils::quadruple(5)  # 20
println math_utils::triple(4)     # 12
```

This also works inside struct methods and lambdas defined within the package.

### Type-keyword naming caveat

The following identifiers are **reserved type keywords** in Kiwi and are parsed as string literals when used as bare names — even inside a package:

`any`, `none`, `string`, `integer`, `float`, `boolean`, `hashmap`, `list`, `lambda`, `date`, `generator`, `object`, `pointer`

If a package function shares its name with one of these keywords, it **must** be called using the fully-qualified `pkg::name(...)` form, even from within the same package:

```kiwi
package mylib
  fn any(lst, pred)   # shares name with type keyword "any"
    for x in lst do
      return true when pred(x)
    end
    false
  end

  fn none(lst, pred)
    not mylib::any(lst, pred)   # must qualify — bare `any(...)` parses as the string "any"
  end
end
```

Choosing names that don't collide with type keywords (e.g. `find_any`, `has_match`) avoids this entirely.
