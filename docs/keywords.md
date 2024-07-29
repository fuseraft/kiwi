# Reserved Keywords in Kiwi

This document provides an overview of reserved keywords in Kiwi, along with explanations and usage examples.

## Table of Contents
- [`abstract`](#abstract)
- [`as`](#as)
- [`async`](#async)
- [`await`](#await)
- [`break`](#break)
- [`catch`](#catch)
- [`class`](#class)
- [`def`](#def)
- [`delete`](#delete)
- [`do`](#do)
- [`else`](#else)
- [`elsif`](#elsif)
- [`end`](#end)
- [`export`](#export)
- [`false`](#false)
- [`for`](#for)
- [`global`](#global)
- [`go`](#go)
- [`if`](#if)
- [`import`](#import)
- [`in`](#in)
- [`input`](#input)
- [`package`](#package)
- [`new`](#new)
- [`next`](#next)
- [`null`](#null)
- [`override`](#override)
- [`parse`](#parse)
- [`pass`](#pass)
- [`print`](#print)
- [`println`](#println)
- [`private`](#private)
- [`return`](#return)
- [`static`](#static)
- [`then`](#then)
- [`this`](#this)
- [`throw`](#throw)
- [`true`](#true)
- [`try`](#try)
- [`when`](#when)
- [`while`](#while)
- [`with`](#with)

## `abstract`
The `abstract` keyword is used to declare a [class](#class) as being abstract, meaning it cannot be instantiated directly, but is intended to be used as a base class.

See [Abstract Classes](abstract_classes.md).

## `as`
The `as` keyword is used to specify an alias for a package.

See [Packages](packages.md).

## `async`
The `async` keyword is used to define an asynchronous method.

See [Concurrency](concurrency.md).

## `await`
The `await` keyword is used to retrieve the result of an asynchronous method invocation.

See [Concurrency](concurrency.md).

## `break`
The `break` keyword is used to exit a loop.

See [Loops](loops.md) and [Control Structures](control_structures.md).

## `catch`
The `catch` keyword is used to define a catch-block in a `try-catch`.

See [Error Handling](error_handling.md).

## `class`
The `class` keyword is used to define a class.

See [Classes](classes.md).

## `def`
The `def` keyword is used to define a method.

See [Functions and Methods](functions.md).

## `delete`
The `delete` keyword is used to remove elements from collections.

It can also be used to delete objects.

See [Hashes](hashes.md) and [Lists](lists.md).

## `do`
The `do` keyword is used to define loop and lambda blocks.

See [Lambdas](lambdas.md) and [Loops](loops.md).

## `else`
The `else` keyword is used to define the `else` branch in a conditional statement.

See [Conditionals](conditionals.md) and [Control Structures](control_structures.md).

## `elsif`
The `elsif` keyword is used to define an `elsif` branch in a conditional statement.

See [Conditionals](conditionals.md) and [Control Structures](control_structures.md).

## `end`
The `end` keyword is used end a code-block.

## `export`
The `export` keyword is used to export a package to a calling script.

See [Packages](packages.md).

## `exit`
The `exit` keyword is used to exit the program.

See [Control Structures](control_structures.md)

## `false`
The `false` keyword is inverse of `true` and is a Boolean value.

See [Types](types.md).

## `for`
The `for` keyword is used to define a for-loop.

See [Loops](loops.md).

## `fn`
The `fn` keyword is used to define a function.

See [Functions and Methods](functions.md).

## `global`
The `global` variable is a hash used to share data between scripts.

## `go`
The `go` keyword is used in the Kiwi REPL to execute statements.

## `if`
The `if` keyword is used to define the `if` branch in a conditional statement.

See [Conditionals](conditionals.md) and [Control Structures](control_structures.md).

## `import`
The `import` keyword is used to import a script or a package.

See [Packages](packages.md).

## `in`
The `in` keyword is used to specify the collection to iterate in a for-loop.

See [Loops](loops.md).

## `input`
The `input` keyword is used to request user input from a console.

See [Console I/O](console_io.md).

## `package`
The `package` keyword is used to define a package.

See [Packages](packages.md).

## `new`
The `new` keyword is used to instantiate a class.

See [Classes](classes.md).

## `next`
The `next` keyword is used to skip to the next iteration of a loop.

See [Loops](loops.md) and [Control Structures](control_structures.md).

## `null`
The `null` keyword is a None value.

See [Types](types.md).

## `override`
The `override` keyword is used to provide an implementation to an abstract method.

See [Abstract Classes](abstract_classes.md).

## `parse`
The `parse` keyword is used to parse a string expression as Kiwi. 

```ruby
parse('println("hello, world!")') # prints: hello, world!
```

## `pass`
The `pass` keyword is used as a placeholder/no-op.

## `print`
The `print` keyword is used to print output to a console.

See [Console I/O](console_io.md).

## `println`
The `println` keyword is used to print output to a console. The output is terminated with a newline.

See [Console I/O](console_io.md).

## `private`
The `private` keyword is used to declare a method with private access (can only be accessed within the class).

See [Classes](classes.md).

## `return`
The `return` keyword is used to return a value from a method, or to exit a method early.

See [Functions and Methods](functions.md) and [Control Structures](control_structures.md).

## `static`
The `static` keyword is used to declare a method with static access (can be accessed without instantiation).

See [Classes](classes.md).

## `then`
The `then` keyword is used to specify code to execute after the completion of an asychronous method invocation.

See [Concurrency](concurrency.md).

## `this`
The `this` keyword is used for class member-access.

See [Classes](classes.md).

## `throw`
The `throw` keyword is used for throwing errors.

See [Error Handling](error_handling.md) and [Control Structures](control_structures.md).

## `true`
The `true` keyword is inverse of `false` and is a Boolean value.

See [Types](types.md).

## `try`
The `try` keyword is used to define a try-block in a `try-catch`.

See [Error Handling](error_handling.md).

## `when`
The `when` keyword is used to define a condition for [`break`](#break), [`exit`](#exit), [`next`](#next), [`return`](#return), and [`throw`](#throw).

See [Conditionals](conditionals.md) and [Control Structures](control_structures.md).

## `while`
The `while` keyword is used to define a while-loop.

See [Loops](loops.md#while), [Conditionals](conditionals.md) and [Control Structures](control_structures.md).

## `with`
The `with` keyword is used to define a lambda.

See [Lambdas](lambdas.md).