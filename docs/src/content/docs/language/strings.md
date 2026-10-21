---
title: Strings
description: String literals, interpolation, escapes, raw strings, indexing and common operations.
---

## Literals

Strings are written in double or single quotes. They are different:

- **Double quotes** process backslash escapes such as `\n` and `\t`, and interpolate `${...}`.
- **Single quotes** are raw: backslashes and `${...}` are kept as written, which makes them the natural choice for regular expressions, Windows paths and templates.

```kiwi
name = "Kiwi"
println "Hello, ${name}!\tTabbed"   # Hello, Kiwi!	Tabbed
println 'Hello, ${name}!\tRaw'      # Hello, ${name}!\tRaw
println 'C:\Users\kiwi'             # C:\Users\kiwi
```

A raw string can't contain a single quote. Use a double-quoted string for that.

Both kinds can span multiple lines; the line breaks become part of the string.

```kiwi
poem = "Roses are red,
Kiwis are green."
println poem
```

To strip the indentation of a multi-line string, use `.dedent()` from the [`string`](/kiwi/stdlib/string/) package.

## Interpolation

Inside a double-quoted string, `${expression}` is replaced by the value of the expression. Any expression works, including method calls, arithmetic and nested strings.

```kiwi
x = 50
y = 10
println "${x} / ${y} = ${x / y}"            # 50 / 10 = 5
println "upper: ${"kiwi".uppercase()}"      # upper: KIWI
println "items: ${[1, 2, 3].size()}"        # items: 3
println "price: $5"                         # price: $5
```

A `$` that isn't followed by `{` is an ordinary character. To print a literal `${`, escape the dollar sign: `"\${"`.

## Escape sequences

These escapes work in double-quoted strings:

| Escape | Meaning |
| :-- | :-- |
| `\n` | Newline |
| `\t` | Tab |
| `\r` | Carriage return |
| `\b` | Backspace |
| `\f` | Form feed |
| `\\` | Backslash |
| `\"` | Double quote |
| `\$` | Dollar sign (prevents interpolation) |
| `\xHH` | Character with code 0x00 to 0xFF, 2 hex digits |
| `\uXXXX` | Unicode code point, 4 hex digits |
| `\UXXXXXXXX` | Unicode code point, 8 hex digits |

```kiwi
println "Quote: \"hi\""     # Quote: "hi"
println "\x4B\x69\x77\x69"  # Kiwi
println "café"         # café
println "\U0001F95D"        # 🥝
```

## Indexing and slicing

Strings are indexed by character from 0. A slice `s[start:end]` takes characters from `start` up to, but not including, `end`; either bound can be left out. A third value sets the step, and a step of `-1` reverses.

```kiwi
s = "kiwifruit"
println s[0]       # k
println s[0:4]     # kiwi
println s[4:]      # fruit
println s[::2]     # kwfut
println s[::-1]    # tiurfiwik
```

Negative indexes are not supported. Use `s[s.size() - 1]` for the last character.

Strings are immutable: methods like `.uppercase()` return a new string.

## Common operations

```kiwi
s = "  Hello, Kiwi!  "

println s.size()                    # 16
println s.trim()                    # Hello, Kiwi!
println s.lowercase()               #   hello, kiwi!
println s.contains("Kiwi")          # true
println s.trim().begins_with("He")  # true
println s.trim().ends_with("!")     # true
println s.index("Kiwi")             # 9
println s.replace("Kiwi", "World")  #   Hello, World!
println "a,b,c".split(",")          # ["a", "b", "c"]
println ["a", "b"].join("-")        # a-b
println "ab" * 3                    # ababab
println "abc".chars()               # ["a", "b", "c"]
```

Concatenate with `+`. If one side is not a string, it is converted:

```kiwi
println "Total: " + 42   # Total: 42
```

Compare strings with `==`, `<` and `>`. Ordering follows the current culture's alphabetical rules rather than raw character codes.

The [Built-in Methods](/kiwi/reference/builtin-methods/#strings) reference lists every string method.

## Regular expressions

Several built-in methods take a regular expression. Write patterns as raw strings so backslashes reach the regex engine untouched.

```kiwi
text = "Order 66 shipped on 2024-06-15"

println text.matches('.*\d{4}-\d{2}-\d{2}')    # true
println text.find('\d+')                       # 66
println text.scan('\d+')                       # ["66", "2024", "06", "15"]
println text.match('(\d{4})-(\d{2})-(\d{2})')  # ["2024", "06", "15"]
println text.rreplace('\d', "#")               # Order ## shipped on ####-##-##
println "a1b22c".rsplit('\d+')                 # ["a", "b", "c"]
```

| Method | Returns |
| :-- | :-- |
| `.matches(re)` | Whether the whole string matches |
| `.matches_all(re)` | Whether the pattern matches anywhere |
| `.find(re)` | The first match, or `""` |
| `.match(re)` | The capture groups of the first match |
| `.scan(re)` | Every match |
| `.scan_groups(re)` | The capture groups of every match |
| `.named_captures(re)` | A hashmap of named groups from the first match |
| `.rreplace(re, replacement)` | Every match replaced; `$1` refers to a group |
| `.rsplit(re, limit?)` | The string split on the pattern |
| `.regex_escape()` | The string with regex metacharacters escaped |

`.replace` and `.split` treat their argument as plain text, not a pattern. For more, see the [`regex`](/kiwi/stdlib/regex/) package.

## The `string` package

The [`string`](/kiwi/stdlib/string/) package adds formatting helpers. Each one can be called as a package function or as a method on any string:

```kiwi
println "the quick fox".titleize()           # The Quick Fox
println "hELLO".capitalize()                 # Hello
println "  too   many   spaces ".squish()    # too many spaces
println "42".padstart(6, "0")                # 000042
println "hi".center(6, "*")                  # **hi**
println string::slug("Hello, World!")        # hello-world
```

To format with `%` specifiers like `printf`, use [`fmt`](/kiwi/stdlib/fmt/). For `{{placeholder}}` templates, use [`template`](/kiwi/stdlib/template/).

## Bytes

`.to_bytes()` encodes a string as UTF-8 [bytes](/kiwi/language/types/#bytes), and `.to_string()` on bytes decodes them. `.ord()` returns the code point of the first character.

```kiwi
println "hi".to_bytes()               # [104, 105]
println "hi".to_bytes().to_string()   # hi
println "A".ord()                     # 65
```
