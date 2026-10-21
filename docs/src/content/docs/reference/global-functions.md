---
title: Global Functions
description: Functions and values available everywhere without a package prefix.
---

These are available in every script without a package prefix. Some are keywords built into the language (`println`, `typeof`, `eval`); others are defined in the standard library's global file.

## Output

| Statement | Writes to | Newline |
| :-- | :-- | :-- |
| `print value` | stdout | no |
| `println value` | stdout | yes |
| `eprint value` | stderr | no |
| `eprintln value` | stderr | yes |

Parentheses are optional: `println "hi"` and `println("hi")` are the same.

```kiwi
print "Loading"
print "..."
println " done"
println [1, 2, 3]
```

`printxy(text, x, y)` writes text at column `x`, row `y` of the terminal. For colors, clearing the screen and cursor control, see [`console`](/kiwi/stdlib/console/).

## Input

`input(prompt)` prints an optional prompt and returns the next line from stdin without its newline. At the end of input it returns an empty string.

```kiwi nocheck
name = input("What is your name? ")
println "Hello, ${name}!"
```

To read all of stdin, or to read it line by line, use [`stdin`](/kiwi/stdlib/stdin/).

## Types and values

### `typeof`

`typeof(value)` returns the type name of a value, or the struct name for an instance. See [Types](/kiwi/language/types/#checking-types).

```kiwi
println typeof(3.14)         # float
println typeof([1])          # list
```

### `serialize`

`serialize(value)` converts a value to its string form, compatible with JSON.

### `deserialize`

`deserialize(text)` parses a string produced by `serialize`, or JSON, back into a value.

```kiwi
text = serialize({id: 7, tags: ["a"]})
println text                    # {"id": 7, "tags": ["a"]}
println deserialize(text)["id"] # 7
```

### `eval`

`eval code` runs a string of Kiwi code and returns the value of its last expression.

```kiwi
println eval "6 * 7"   # 42
```

## Dates

`d(year, month, day, hour, minute, second, millisecond)` builds a date. Every argument after the year defaults to the start of its range. Passing a single string parses it instead.

```kiwi
println d(2024, 6, 15).to_string("yyyy-MM-dd")   # 2024-06-15
println d("2024-06-15T09:30:00").hour()          # 9
```

See [Types: Dates](/kiwi/language/types/#dates) and the [`time`](/kiwi/stdlib/time/) package.

## Characters

`char(code)` returns the character for a Unicode code point given as a hex string, such as `"U+2764"` or `"1F95D"`.

```kiwi
println char("U+1F95D")   # 🥝
println char("2764")      # ❤
```

## Tasks

`spawn(lambda, args)` runs a lambda on a background task and returns a task id, and `await(id)` waits for it and returns its result. They are shortcuts for [`task::spawn`](/kiwi/stdlib/task/) and `task::await`.

```kiwi
id = spawn(do (a, b) => a + b, [2, 3])
println await(id)   # 5
```

## Shared state

`global` is a hashmap that every part of a program, including every loaded file and package, sees as the same object.

```kiwi
global.request_count = 0
global.request_count += 1
println global.request_count   # 1
```

## Runtime information

| Function | Returns |
| :-- | :-- |
| `__entrypath__()` | The path of the script that was run. |
| `__execpath__()` | The path of the script being executed. |
| `__memusage__()` | A hashmap of memory statistics: `working_set`, `gc_heap`, `gc_total_allocated`, and `gen0_collections` through `gen2_collections`. |
| `__tokenize__(code)` | A hashmap with a `tokens` list, each token a hashmap with `token`, `text`, `type` and `span`; or an `error` key if lexing failed. |

```kiwi
for t in __tokenize__("x = 1")["tokens"] do
  println "${t["type"]}: ${t["text"]}"
end
```

## Constants

| Constant | Value |
| :-- | :-- |
| `KIWI_STDLIB_VERSION` | The standard library version, as a string. |
| `STDLIB_PACKAGES` | The names of the standard library packages. |

## Source reference

The global functions and constants defined in `lib/__global__.kiwi`:

<!-- api: __global__ level=3 -->
