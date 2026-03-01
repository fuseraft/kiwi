# Packages

Use the `package` keyword to define a package. Use `export` to make a package available, and `import` to bring a package into scope. Methods defined in a package are accessed using a fully qualified name (`package::method`).

## Table of Contents

- [Defining and Using a Package](#defining-and-using-a-package)
- [File-Based Packages](#file-based-packages)
- [`import` vs `export`](#import-vs-export)
- [Nested Packages](#nested-packages)

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

## Nested Packages

Packages support namespaced names using `::`.

```kiwi
package app::utils
  fn greet(name)
    println("Hello, ${name}!")
  end
end

export "app::utils"

app::utils::greet("World")
```
