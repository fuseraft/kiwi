# Modules

Use the `module` keyword to define a module. Use the `import` keyword to import a module.

Methods defined in a module can only be accessed using a fully qualified name.

```ruby
# Defining modules.
module greeter
  def greet(name)
    println "Hello, ${name}."
  end
end

module excited_greeter
  def greet(name)
    println "Hello, ${name}!"
  end
end

# Importing modules.
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

## Module Homes

***This is an experimental feature.***

To specify a module home, use the `__home__(homename)` builtin within the `module` definition context.

This will register a module to a home.

To include a homed module, you can prefix the module name with `homename/` to specify the home.

### Example Homed Module `fuseraft/fun`

In this example, we have two Astral scripts:
1. **fun.🚀**: Defines the homed module `fuseraft/fun`.
2. **app.🚀**: Imports and uses functionality defined in the homed module `fuseraft/fun`.

#### `./fun.🚀`
```ruby
module fun
  __home__("fuseraft")

  def method()
    println "Astral is fun!"
  end
end

export "fun" # Exports the "fun" module to the calling script.
```

#### `./app.🚀`
```ruby
import "@fuseraft/fun"

fun::method() # prints: Astral is fun!
```

## Aliasing

You can specify an alias for an imported module. This creates a class that cannot be instantiated. The class will contain static methods matching the methods defined in the module. 

#### `./fun.🚀`
```ruby
module fun
  __home__("fuseraft")

  def method()
    println "Astral is fun!"
  end
end

export "fun" # Exports the "fun" module to the calling script.
```

#### `./app.🚀`
```ruby
import "@fuseraft/fun" as Fun

Fun.method() # prints: Astral is fun!
```