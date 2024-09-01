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
