---
title: Syntax Basics
description: Statements, blocks, comments, identifiers, literals and reserved words.
---

## Statements and blocks

A Kiwi program is a sequence of statements, usually one per line. There are no semicolons. Statements run from top to bottom; there is no required entry point.

Blocks start with a keyword and close with `end`. Indentation is not significant, but two spaces is the convention.

```kiwi
fn classify(n)
  if n % 2 == 0
    "even"
  else
    "odd"
  end
end

for i in [1 to 3] do
  println "${i} is ${classify(i)}"
end
```

Loops (`for`, `while`, `repeat`) and lambdas open their body with `do`. Conditionals, functions, structs, packages and `try` don't.

## Line breaks

A newline ends a statement unless the expression is clearly unfinished: an open bracket, parenthesis or brace, or a binary operator at the end of the line or the start of the next one.

```kiwi
total = 1 +
  2 +
  3

items = [
  "kiwi",
  "mango",
]

println total   # 6
println items   # ["kiwi", "mango"]
```

A binary operator can also start the continuation line:

```kiwi
ready = true
loaded = true
ok = ready
  && loaded
  && !false
println ok   # true
```

The exception is `-`: a line that starts with `-` is a new statement (a negative number), so put a subtraction's `-` at the end of the line.

Method chains can continue on the next line by starting it with `.`:

```kiwi
result = [1 to 10]
  .filter(do (n) => n % 2 == 0)
  .map(do (n) => n * 10)

println result   # [20, 40, 60, 80, 100]
```

A `[` at the start of a line always begins a new list literal. It is never an index into the expression on the line above.

## Comments

`#` starts a comment that runs to the end of the line. `/#` and `#/` delimit a block comment, which can span lines or sit in the middle of one.

```kiwi
# A line comment.
x = 5  # A trailing comment.

/#
  A block comment.
  It can span many lines.
#/

println "a" /# inline #/ + "b"   # ab
```

Block comments placed directly above a declaration are [docstrings](#docstrings).

## Identifiers

Names start with a letter or underscore, followed by letters, digits or underscores. They are case sensitive. The conventions are:

| Kind | Convention | Example |
| :-- | :-- | :-- |
| Variables, functions, methods | `snake_case` | `user_count`, `load_config` |
| Structs, enums | `PascalCase` | `HttpResponse`, `Color` |
| Constants | `UPPER_SNAKE_CASE` (required) | `MAX_RETRIES` |
| Private helpers | leading `_` | `_parse_header` |
| Packages | lower case | `json`, `my_app::utils` |

Kiwi has no `?`-suffixed predicate methods (`empty?`, `has_key?`). A `?` is always parsed as the ternary operator, and a stray one produces confusing syntax errors on the following lines.

### Reserved words

[Keywords](/kiwi/reference/keywords/) can't be used as names. That includes the type names `integer`, `float`, `boolean`, `string`, `list`, `hashmap`, `lambda`, `object`, `date`, `bytes`, `pointer`, `generator`, `any` and `none`. Using one as a variable, parameter or function name is a syntax error that suggests an alternative:

```kiwi fails
list = [1, 2, 3]
# SyntaxError: `list` is a type name and can't be used as a variable name. Rename it, e.g. `items`.
```

The same applies to bare hashmap keys and member access. Quote the key instead:

```kiwi
counts = {"list": 1, "string": 2}
println counts["list"]   # 1
```

## Literals

```kiwi
println 42          # integer
println -7          # integer
println 0xFF        # integer, hexadecimal: 255
println 0b1010      # integer, binary: 10
println 0o17        # integer, octal: 15
println 3.14        # float
println 1.5e3       # float: 1500
println true        # boolean
println null        # null
println "text"      # string with interpolation and escapes
println 'raw'       # raw string
println [1, 2]      # list
println {a: 1}      # hashmap
println b"hi"       # bytes: [104, 105]
```

Strings are covered in [Strings](/kiwi/language/strings/), lists in [Lists](/kiwi/language/lists/), and hashmaps in [Hashmaps](/kiwi/language/hashmaps/).

Integers are 64-bit. A hexadecimal, binary or octal literal is a 64-bit pattern, so `0xFFFFFFFF` is `4294967295` and `0xFFFFFFFFFFFFFFFF` is `-1`. A literal that doesn't fit in 64 bits is an error.

To call a method on a number literal, wrap the literal in parentheses: `(42).to_string()`, `(3.7).to_integer()`.

## The `...` placeholder

`...` is a statement that does nothing. Use it for bodies you haven't written yet.

```kiwi
fn todo()
  ...
end

println todo()   # null
```

## Docstrings

A block comment directly above a package, struct, function or constant documents it. The standard library uses `@`-tags, and the [standard library reference](/kiwi/stdlib/) is generated from them.

```kiwi
/#
@summary Converts a temperature from Celsius to Fahrenheit.
@param float celsius : Degrees Celsius.
@return float : Degrees Fahrenheit.
@example
---
println to_fahrenheit(100.0)  # 212
---
#/
fn to_fahrenheit(celsius: float): float
  celsius * 9 / 5 + 32
end
```

| Tag | Format | Use |
| :-- | :-- | :-- |
| `@summary` | `@summary Text` | One-sentence description. Every public item should have one. |
| `@param` | `@param type name : Description` | One per parameter, in order. Use `any` for dynamic types. |
| `@return` | `@return type : Description` | What the function returns. |
| `@throws` | `@throws ErrorType : Description` | Errors the function can throw. |
| `@example` | Code between `---` lines | A short, runnable example. |
| `@see` | `@see name : Note` | Related items. |
| `@deprecated` | `@deprecated : Use x instead` | Marks an item as obsolete. |

Keep summaries about *what* and *why*; the signature already says *how*.

## Evaluating code at runtime

`eval` parses and runs a string of Kiwi code and returns the value of its last expression.

```kiwi
eval "println 1 + 2"     # 3
println eval "3 * 3"     # 9
```
