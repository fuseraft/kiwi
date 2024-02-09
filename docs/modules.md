# Modules

Use the `module` keyword to define a module. Use the `import` keyword to import a module.

Methods defined in a module can only be accessed using a fully qualified name.

```ruby
# Defining modules.
module greeter
    def greet(@name)
        println "Hello, ${@name}."
    end
end

module excited_greeter
    def greet(@name)
        println "Hello, ${@name}!"
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

To include a homed module, you can prefix the module name with `@homename/` to specify the home.

### Example Homed Module `@fuseraft/fun`

In this example, we have two Kiwi scripts:
1. **fun.kiwi**: Defines the homed module `@fuseraft/fun`.
2. **app.kiwi**: Imports and uses functionality defined in the homed module `@fuseraft/fun`.

#### `./fun.kiwi`
```ruby
module fun
  __home__("fuseraft")

  def method()
    println "Kiwi is fun!"
  end
end

import "fun" # Registers the "fun" module.
```

#### `./app.kiwi`
```ruby
import "@fuseraft/fun"

fun::method() # prints: Kiwi is fun!
```