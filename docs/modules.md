# Modules

Use the `module` keyword to define a module. Use the `import` keyword to import a module.

Methods defined in a module can only be accessed using a fully qualified name.

```
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