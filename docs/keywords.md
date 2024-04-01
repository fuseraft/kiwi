# Reserved Keywords in Astral

This document provides an overview of reserved keywords in Astral, along with explanations and usage examples.

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
- [`go`](#go)
- [`if`](#if)
- [`import`](#import)
- [`in`](#in)
- [`input`](#input)
- [`lambda`](#lambda)
- [`module`](#module)
- [`new`](#new)
- [`next`](#next)
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
- [`while`](#while)

## `abstract`
The `abstract` keyword is used to declare a [class](#class) as being abstract, meaning it cannot be instantiated directly, but is intended to be used as a base class.

See [Abstract Classes](abstract_classes.md).

## `as`
The `as` keyword is used to specify an alias for a module.

See [Modules](modules.md).

## `async`
The `async` keyword is used to define an asynchronous method.

See [Concurrency](concurrency.md).

## `await`
The `await` keyword is used to retrieve the result of an asynchronous method invocation.

See [Concurrency](concurrency.md).

## `break`
The `break` keyword is used to exit a loop.

See [Loops](loops.md).

## `catch`
The `catch` keyword is used to define a catch-block in a `try-catch`.

See [Error Handling](error_handling.md).

## `class`
The `class` keyword is used to define a class.

See [Classes](classes.md).

## `def`
The `def` keyword is used to define a method.

See [Methods](methods.md).

## `delete`
The `delete` keyword is used to remove elements from collections.

It can also be used to delete objects.

See [Hashes](hashes.md) and [Lists](lists.md).

## `do`
The `do` keyword is used to define loop and lambda blocks.

See [Lambdas](lambdas.md) and [Loops](loops.md).

## `else`
The `else` keyword is used to define the `else` branch in a conditional statement.

See [Conditionals](conditionals.md).

## `elsif`
The `elsif` keyword is used to define an `elsif` branch in a conditional statement.

See [Conditionals](conditionals.md).

## `end`
The `end` keyword is used end a code-block.

## `export`
The `export` keyword is used to export a module to a calling script.

See [Modules](modules.md).

## `false`
The `false` keyword is inverse of `true` and is a Boolean value.

See [Types](types.md).

## `for`
The `for` keyword is used to define a for-loop.

See [Loops](loops.md).

## `go`
The `go` keyword is used in the Astral REPL to execute statements.

## `if`
The `if` keyword is used to define the `if` branch in a conditional statement.

See [Conditionals](conditionals.md).

## `import`
The `import` keyword is used to import a script or a module.

See [Modules](modules.md).

## `in`
The `in` keyword is used to specify the collection to iterate in a for-loop.

See [Loops](loops.md).

## `input`
The `input` keyword is used to request user input from a console.

See [Console I/O](console_io.md).

## `lambda`
The `lambda` keyword is used to define a lambda.

See [Lambdas](lambdas.md).

## `module`
The `module` keyword is used to define a module.

See [Modules](modules.md).

## `new`
The `new` keyword is used to instantiate a class.

See [Classes](classes.md).

## `next`
The `next` keyword is used to skip to the next iteration of a loop.

See [Loops](loops.md).

## `override`
The `override` keyword is used to provide an implementation to an abstract method.

See [Abstract Classes](abstract_classes.md).

## `parse`
The `parse` keyword is used to parse a string expression as Astral. 

```ruby
parse "println \"hello, world!\""
# prints: hello, world!
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

See [Methods](methods.md).

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

See [Error Handling](error_handling.md).

## `true`
The `true` keyword is inverse of `false` and is a Boolean value.

See [Types](types.md).

## `try`
The `try` keyword is used to define a try-block in a `try-catch`.

See [Error Handling](error_handling.md).

## `while`
The `while` keyword is used to define a while-loop.

See [Loops](loops.md).
