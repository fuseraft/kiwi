# Packages

Use the `package` keyword to define a package. Use the `import` keyword to import a package.

Methods defined in a package can only be accessed using a fully qualified name.

```kiwi
# Defining packages.
package greeter
  def greet(name)
    println("Hello, ${name}.")
  end
end

package excited_greeter
  def greet(name)
    println("Hello, ${name}!")
  end
end

# Importing packages.
import "greeter"
import "excited_greeter"

# Invoking their methods.
greeter::greet("World")
excited_greeter::greet("World")
```

The output will be:
```
Hello World.
Hello World!
```

## Package Homes

***This is an experimental feature.***

To specify a package home, use the `__home__(homename)` builtin within the `package` definition context.

This will register a package to a home.

To include a homed package, you can prefix the package name with `homename/` to specify the home.

### Example Homed Package `fuseraft/fun`

In this example, we have two Kiwi scripts:
1. **fun.🥝**: Defines the homed package `fuseraft/fun`.
2. **app.🥝**: Imports and uses functionality defined in the homed package `fuseraft/fun`.

#### `./lib/fun.🥝`
```kiwi
package fun
  __home__("fuseraft")

  def method()
    println("Kiwi is fun!")
  end
end

export "fun" # Exports the "fun" package to the calling script.
```

#### `./app.🥝`
```kiwi
import "@fuseraft/fun"

fun::method() # prints: Kiwi is fun!
```

## Aliasing

You can specify an alias for an imported package. This creates a class that cannot be instantiated. The class will contain static methods matching the methods defined in the package. 

#### `./lib/fun.🥝`
```kiwi
package fun
  __home__("fuseraft")

  def method()
    println("Kiwi is fun!")
  end
end

export "fun" # Exports the "fun" package to the calling script.
```

#### `./app.🥝`
```kiwi
import "@fuseraft/fun" as Fun

Fun.method() # prints: Kiwi is fun!
```