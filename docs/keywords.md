# Reserved Keywords in Kiwi

This document provides an overview of reserved keywords in Kiwi, along with explanations and usage examples.

## Variables

### `var`
The `var` keyword is used to declare a variable.

See [Variables](variables.md) and [`var`](variables.md#var-keyword).

### `const`
The `const` keyword declares an immutable constant. Constant names must be all uppercase with underscores.

```kiwi
const MAX_RETRIES = 3
const API_URL = "https://example.com"

println(MAX_RETRIES)  # 3
```

## Control Structures

### `break`
The `break` keyword is used to exit a loop.

See [Loops](loops.md) and [Control Structures](control_structures.md).

### `do`
The `do` keyword is used to define loop and lambda blocks.

See [Lambdas](lambdas.md) and [Loops](loops.md).

### `else`
The `else` keyword is used to define the `else` branch in a conditional statement.

See [Conditionals](conditionals.md) and [Control Structures](control_structures.md).

### `elsif`
The `elsif` keyword is used to define an `elsif` branch in a conditional statement.

See [Conditionals](conditionals.md) and [Control Structures](control_structures.md).

### `end`
The `end` keyword is used to terminate a code-block.

### `exit`
The `exit` keyword is used to terminate the program. It accepts an integer parameter to represent the exit code.

See [Control Structures](control_structures.md).

### `for`
The `for` keyword is used to define a for-loop.

See [Loops](loops.md).

### `case`
The `case` keyword is used to define a `case` statement.

See [Conditionals](conditionals.md) and [Control Structures](control_structures.md#2-case--when--else).

### `if`
The `if` keyword is used to define the `if` branch in a conditional statement.

See [Conditionals](conditionals.md) and [Control Structures](control_structures.md#1-if--elsif--else).

### `in`
The `in` keyword is used to specify the collection to iterate in a for-loop.

See [Loops](loops.md).

### `next`
The `next` keyword is used to skip to the next iteration of a loop.

See [Loops](loops.md) and [Control Structures](control_structures.md).

### `repeat`
The `repeat` keyword is used to define a repeat-loop.

See [Loops](loops.md).

### `to`
The `to` keyword is used in range expressions to define an inclusive range of integers.

```kiwi
println [1 to 5]    # [1, 2, 3, 4, 5]
println [10 to 7]   # [10, 9, 8, 7]
```

See [Ranges](ranges.md).

### `while`
The `while` keyword is used to define a while-loop.

See [Loops](loops.md#2-while--condition-based-loop), [Conditionals](conditionals.md) and [Control Structures](control_structures.md).

### `when`
The `when` keyword is used to define a condition for `break`, `exit`, `next`, `return`, and `throw`.

See [Conditionals](conditionals.md) and [Control Structures](control_structures.md).

## Error Handling

### `catch`
The `catch` keyword is used to define a catch-block in a `try-catch`.

See [Error Handling](error_handling.md).

### `finally`
The `finally` keyword is used to define a finally-block in a `try-catch`.

See [Error Handling](error_handling.md).

### `throw`
The `throw` keyword is used for throwing errors.

See [Error Handling](error_handling.md) and [Control Structures](control_structures.md).

### `try`
The `try` keyword is used to define a try-block in a `try-catch`.

See [Error Handling](error_handling.md).

## Functions and Methods

### `def`
The `def` keyword is an alias for [`fn`](#fn).

### `fn`
The `fn` keyword is used to define a function or method.

See [Functions and Methods](functions.md).

### `private`
The `private` keyword is used to declare a method with private access (can only be accessed within the struct).

See [Structs](structs.md).

### `return`
The `return` keyword is used to return a value from a method, or to exit a method early.

See [Functions and Methods](functions.md) and [Control Structures](control_structures.md).

### `static`
The `static` keyword is used inside a struct to declare a static method or a static variable.

**Static methods** can be invoked directly on the struct without an instance.

```kiwi
struct MathUtils
  static fn square(n: integer): integer
    n * n
  end
end

println(MathUtils.square(4))  # 16
```

**Static variables** are shared across all instances of a struct. Declare with `static @name = value` (an optional type hint is supported). Read or write them inside any method using `@@name`, or externally using `StructName.name`.

```kiwi
struct Counter
  static @count: integer = 0

  fn new()
    @@count += 1
  end

  static fn get(): integer
    @@count
  end
end

Counter.new()
Counter.new()
println(Counter.get())   # 2
println(Counter.count)   # 2
Counter.count = 0
println(Counter.count)   # 0
```

See [Structs](structs.md#static-method-definition) and [Static Variables](structs.md#static-variables).

### `with`
The `with` keyword is used to define a lambda.

See [Lambdas](lambdas.md).

### `yield`
The `yield` keyword is used inside a generator function to produce a value and suspend execution until the next value is requested.

See [Generators](generators.md).

## Structs

### `abstract`
The `abstract` keyword has two uses:

**Abstract structs** — mark a struct as abstract so it cannot be instantiated directly. Other structs can inherit from it.

```kiwi
abstract struct Shape
  abstract fn area(): float
  abstract fn perimeter(): float
end

struct Circle : Shape
  fn new(r: float)
    @r = r
  end

  fn area(): float
    math::PI * @r * @r
  end

  fn perimeter(): float
    2.0 * math::PI * @r
  end
end

c = Circle.new(5.0)
println(c.area())  # 78.53981633974483
```

**Abstract methods** — declare a method signature in an abstract struct that derived structs must implement. Concrete structs that inherit from an abstract base are checked for completeness at definition time.

See [Structs](structs.md).

### `interface`
Reserved for future use.

### `struct`
The `struct` keyword is used to define a struct.

See [Structs](structs.md).

### `new`
The `new` keyword is used to instantiate a struct.

See [Structs](structs.md).

### `override`
The `override` keyword marks a method in a derived struct as intentionally overriding a method from its base struct. The keyword is accepted by the parser and recorded on the method, but override enforcement is not yet implemented at runtime.

### `@` (self)
The `@` symbol is used inside a struct method to access or assign instance variables.

```kiwi
struct Point
  fn new(x, y)
    @x = x
    @y = y
  end

  fn to_string(): string
    "(${@x}, ${@y})"
  end
end

p = Point.new(3, 4)
println(p.to_string())  # (3, 4)
```

See [Structs](structs.md).

## Packages

### `as`
The `as` keyword is used to specify an alias for a [`package`](#package) or to specify an iterator variable in a [`repeat`-loop](#repeat).

See [Packages](packages.md) and [Loops](loops.md).

### `export`
The `export` keyword is used to export a package to a calling script.

See [Packages](packages.md).

### `import`
The `import` keyword is used to import a script or a package.

See [Packages](packages.md).

### `include`
The `include` keyword executes another Kiwi file inline in the current scope, making its definitions immediately available.

```kiwi
include "lib/helpers"

# functions and variables from helpers.kiwi are now in scope
```

### `package`
The `package` keyword is used to define a package.

See [Packages](packages.md).

### `require`
The `require` keyword loads a package by name if it has not already been imported. Unlike `include`, it looks up the package by name rather than file path.

```kiwi
require "xml"

doc = xml::parse("<root><item>hello</item></root>")
```

## Events

### `emit`
The `emit` keyword fires a named event, optionally passing arguments to registered handlers.

```kiwi
emit "user.login", { name: "Scotty" }
```

### `off`
The `off` keyword removes all handlers registered for a named event.

```kiwi
off "user.login"
```

### `on`
The `on` keyword registers a handler that runs every time the named event is emitted.

```kiwi
on "user.login" with (user) do
  println("Welcome, ${user.name}!")
end
```

### `once`
The `once` keyword registers a handler that runs only the first time the named event is emitted.

```kiwi
once "app.start" do
  println("Application started.")
end
```

See [Events](events.md).

## Console I/O

### `input`
The `input` keyword is used to request user input from a console.

See [Console I/O](console_io.md).

### `eprint`
The `eprint` keyword is used to print to the standard error stream.

See [Console I/O](console_io.md).

### `eprintln`
The `eprintln` keyword is used to print to the standard error stream. The output is terminated with a newline.

### `print`
The `print` keyword is used to print output to the standard output stream.

See [Console I/O](console_io.md).

### `println`
The `println` keyword is used to print output to the standard output stream. The output is terminated with a newline.

See [Console I/O](console_io.md).

### `printxy`
The `printxy` keyword prints output at a specific terminal cursor position.

```kiwi
printxy(10, 5, "Hello!")  # prints "Hello!" at column 10, row 5
```

## Miscellaneous

### `delete`
The `delete` keyword is used to remove elements from collections.

It can also be used to delete objects.

See [Hashmaps](hashmaps.md) and [Lists](lists.md).

### `eval`
The `eval` keyword parses and executes a string as Kiwi code.

```kiwi
eval 'println("hello, world!")'  # prints: hello, world!

code = "1 + 2"
println(eval code)  # 3
```

### `global`
The `global` variable is a hashmap used to share data between scripts.

### `go`
The `go` keyword is used in the Kiwi REPL to execute statements.

### `null`
The `null` keyword is a None value.

See [Types](types.md).

### `...` (no-op)
The `...` keyword is used as a placeholder/no-op.

### `true`
The `true` keyword is inverse of `false` and is a Boolean value.

See [Types](types.md).

### `false`
The `false` keyword is inverse of `true` and is a Boolean value.

See [Types](types.md).
