# Reserved Keywords in Kiwi

This document provides an overview of reserved keywords in Kiwi, along with explanations and usage examples.

## Table of Contents
- [Control Structures](#control-structures)
  - [`break`](#break)
  - [`do`](#do)
  - [`else`](#else)
  - [`elsif`](#elsif)
  - [`end`](#end)
  - [`for`](#for)
  - [`if`](#if)
  - [`in`](#in)
  - [`next`](#next)
  - [`repeat`](#repeat)
  - [`while`](#while)
  - [`when`](#when)
- [Error Handling](#error-handling)
  - [`catch`](#catch)
  - [`finally`](#finally)
  - [`throw`](#throw)
  - [`try`](#try)
- [Functions and Methods](#functions-and-methods)
  - [`def`](#def)
  - [`fn`](#fn)
  - [`private`](#private)
  - [`return`](#return)
  - [`static`](#static)
  - [`with`](#with)
- [Structs](#structs)
  - [`abstract`](#abstract)
  - [`struct`](#struct)
  - [`new`](#new)
  - [`override`](#override)
  - [`this`](#this)
- [Packages](#packages)
  - [`as`](#as)
  - [`export`](#export)
  - [`import`](#import)
  - [`package`](#package)
- [Concurrency](#concurrency)
  - [`fork`](#fork)
  - [`then`](#then)
- [Console I/O](#console-io)
  - [`input`](#input)
  - [`print`](#print)
  - [`println`](#println)
- [Miscellaneous](#miscellaneous)
  - [`delete`](#delete)
  - [`global`](#global)
  - [`go`](#go)
  - [`null`](#null)
  - [`parse`](#parse)
  - [`pass`](#pass)
  - [`true`](#true)
  - [`false`](#false)

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
The `end` keyword is used to end a code-block.

### `for`
The `for` keyword is used to define a for-loop.

See [Loops](loops.md).

### `if`
The `if` keyword is used to define the `if` branch in a conditional statement.

See [Conditionals](conditionals.md) and [Control Structures](control_structures.md).

### `in`
The `in` keyword is used to specify the collection to iterate in a for-loop.

See [Loops](loops.md).

### `next`
The `next` keyword is used to skip to the next iteration of a loop.

See [Loops](loops.md) and [Control Structures](control_structures.md).

### `repeat`
The `repeat` keyword is used to define a repeat-loop.

See [Loops](loops.md).

### `while`
The `while` keyword is used to define a while-loop.

See [Loops](loops.md#while), [Conditionals](conditionals.md) and [Control Structures](control_structures.md).

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
The `def` keyword is used to define a method.

See [Functions and Methods](functions.md).

### `fn`
The `fn` keyword is used to define a function.

See [Functions and Methods](functions.md).

### `private`
The `private` keyword is used to declare a method with private access (can only be accessed within the struct).

See [Structs](structs.md).

### `return`
The `return` keyword is used to return a value from a method, or to exit a method early.

See [Functions and Methods](functions.md) and [Control Structures](control_structures.md).

### `static`
The `static` keyword is used to declare a method with static access (can be accessed without instantiation).

See [Structs](structs.md).

### `with`
The `with` keyword is used to define a lambda.

See [Lambdas](lambdas.md).

## Structs

### `abstract`
The `abstract` keyword is used to declare a [struct](#struct) as being abstract, meaning it cannot be instantiated directly, but is intended to be used as a base struct.

See [Abstract Structs](abstract_structs.md).

### `struct`
The `struct` keyword is used to define a struct.

See [Structs](structs.md).

### `new`
The `new` keyword is used to instantiate a struct.

See [Structs](structs.md).

### `override`
The `override` keyword is used to provide an implementation to an abstract method.

See [Abstract Structs](abstract_structs.md).

### `this`
The `this` keyword is used for struct member-access.

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

### `package`
The `package` keyword is used to define a package.

See [Packages](packages.md).

## Concurrency

### `fork`
The `fork` keyword is used to spawn an asynchronous task.

See [`task`](lib/task.md#creating-tasks-with-fork).

### `then`
The `then` keyword is used to specify code to execute after the completion of an asynchronous task.

See [`task`](lib/task.md).

## Console I/O

### `input`
The `input` keyword is used to request user input from a console.

See [Console I/O](console_io.md).

### `print`
The `print` keyword is used to print output to a console.

See [Console I/O](console_io.md).

### `println`
The `println` keyword is used to print output to a console. The output is terminated with a newline.

See [Console I/O](console_io.md).

## Miscellaneous

### `delete`
The `delete` keyword is used to remove elements from collections.

It can also be used to delete objects.

See [Hashmaps](hashmaps.md) and [Lists](lists.md).

### `global`
The `global` variable is a hashmap used to share data between scripts.

### `go`
The `go` keyword is used in the Kiwi REPL to execute statements.

### `null`
The `null` keyword is a None value.

See [Types](types.md).

### `parse`
The `parse` keyword is used to parse a string expression as Kiwi. 

```kiwi
parse('println("hello, world!")') // prints: hello, world!
```

### `pass`
The `pass` keyword is used as a placeholder/no-op.

### `true`
The `true` keyword is inverse of `false` and is a Boolean value.

See [Types](types.md).

### `false`
The `false` keyword is inverse of `true` and is a Boolean value.

See [Types](types.md).
