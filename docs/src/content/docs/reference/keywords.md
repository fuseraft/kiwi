---
title: Keywords
description: Every reserved word in Kiwi, with a one-line summary and a link to the details.
---

Keywords are reserved and can't be used as variable, function or parameter names. The [type names](#type-names) at the end are reserved too.

## Declarations

| Keyword | Meaning |
| :-- | :-- |
| `fn` | Defines a function or method. See [Functions](/kiwi/language/functions/). |
| `def` | Alias for `fn`. |
| `var` | Declares a variable, optionally with a type hint. See [Variables](/kiwi/language/variables/#var-declarations). |
| `const` | Declares a constant. See [Variables](/kiwi/language/variables/#constants). |
| `struct` | Defines a struct. See [Structs](/kiwi/language/structs/). |
| `abstract` | Marks a struct that can't be instantiated, or a method children must implement. |
| `static` | Declares a struct-level method or variable. |
| `private` | Marks a method as internal to its struct. |
| `override` | Marks a method that overrides a parent method. Informational. |
| `super` | Calls the parent struct's version of a method. |
| `enum` | Defines an enumeration. See [Enums](/kiwi/language/enums/). |
| `package` | Defines a package. See [Packages](/kiwi/language/packages/). |
| `interface` | Reserved for future use. |

## Control flow

| Keyword | Meaning |
| :-- | :-- |
| `if`, `elsif`, `else` | Conditional branches. See [Control Flow](/kiwi/language/control-flow/). |
| `case`, `when` | Multi-way branch expression. `when` also guards statements. |
| `end` | Closes a block. |
| `for`, `in` | Iterates over a collection. See [Loops](/kiwi/language/loops/). |
| `while` | Loops while a condition holds. |
| `repeat`, `as` | Loops a fixed number of times, optionally naming the counter. |
| `do` | Opens a loop body or a lambda. |
| `to` | Builds an inclusive range: `[1 to 10]`. |
| `break` | Leaves the innermost loop. |
| `next` | Skips to the next loop iteration. |
| `return` | Returns from a function. |
| `yield` | Produces a value from a [generator](/kiwi/language/generators/). |
| `exit` | Ends the program with an exit code. |

## Errors

| Keyword | Meaning |
| :-- | :-- |
| `try`, `catch`, `finally` | Handle errors. See [Error Handling](/kiwi/language/errors/). |
| `throw` | Raises an error. |

## Functions and lambdas

| Keyword | Meaning |
| :-- | :-- |
| `with` | Alternative way to open a lambda: `with (x) do ... end`. See [Lambdas](/kiwi/language/lambdas/). |

## Modules

| Keyword | Meaning |
| :-- | :-- |
| `import` | Activates a package or loads a file, and returns a package reference. |
| `include` | Loads a file. |
| `require` | Asserts that a package is available. |
| `export` | Activates a package. |

See [Packages](/kiwi/language/packages/#splitting-code-across-files).

## Events

| Keyword | Meaning |
| :-- | :-- |
| `on` | Registers an event handler. See [Events](/kiwi/language/events/). |
| `once` | Registers a handler that runs once. |
| `emit` | Triggers an event. |
| `off` | Removes event handlers. |

## Input and output

| Keyword | Meaning |
| :-- | :-- |
| `print`, `println` | Write to stdout, without or with a newline. |
| `eprint`, `eprintln` | Write to stderr. |
| `printxy` | Write at a terminal position. |
| `eval` | Run a string as Kiwi code. |

See [Global Functions](/kiwi/reference/global-functions/).

## Operators and values

| Keyword | Meaning |
| :-- | :-- |
| `and`, `or`, `not` | Logical operators, same as `&&`, `\|\|` and `!`. |
| `true`, `false` | Boolean values. |
| `null` | The absence of a value. |

## Symbols

| Symbol | Meaning |
| :-- | :-- |
| `@` | The current struct instance. `@name` is an instance variable. |
| `@@name` | A static struct variable. |
| `@name` above `fn` | A [decorator](/kiwi/language/decorators/). |
| `::` | Package access: `math::sqrt`. |
| `=>` | Separates a lambda's parameters from its expression. |
| `..` | Range, same as `to`. |
| `...` | An empty statement placeholder. |
| `#`, `/# #/` | Comments. |

## Type names

`integer`, `float`, `boolean`, `string`, `list`, `hashmap`, `lambda`, `object`, `date`, `bytes`, `pointer`, `generator`, `any` and `none` are type keywords used in [type hints](/kiwi/language/type-hints/). They can't be used as names; quote them when you need them as hashmap keys.
