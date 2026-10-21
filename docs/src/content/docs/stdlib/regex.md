---
title: regex
description: Regular expression matching, searching, splitting and replacing.
---

The `regex` package wraps the .NET regular expression engine. Every function takes the **pattern first**, then the text. Write patterns as single-quoted raw strings so backslashes reach the engine unchanged.

```kiwi
println regex::test('^\d{3}-\d{4}$', "555-1234")                # true
println regex::find('\d+', "order 66 of 99")                     # 66
println regex::scan('\d+', "1 22 333")                           # ["1", "22", "333"]
println regex::match('(\w+)@(\w+)\.com', "mail ada@example.com") # ["ada", "example"]
println regex::replace('\s+', "too   many    spaces", " ")       # too many spaces
println regex::split('[,;]\s*', "a, b;c")                        # ["a", "b", "c"]
```

## Groups

`match` returns the capture groups of the first match. `scan_groups` returns them for every match, and `named_captures` returns named groups as a hashmap:

```kiwi
println regex::scan_groups('(\w)=(\d)', "a=1 b=2")
# [["a", "1"], ["b", "2"]]

println regex::named_captures('(?<user>\w+)@(?<host>[\w.]+)', "ada@example.com")
# {"user": "ada", "host": "example.com"}
```

In a replacement, `$1`, `$2` and so on refer to groups:

```kiwi
println regex::replace('(\d{4})-(\d{2})-(\d{2})', "2024-06-15", "$3/$2/$1")   # 15/06/2024
```

## Patterns

Patterns use [.NET regular expression syntax](https://learn.microsoft.com/en-us/dotnet/standard/base-types/regular-expression-language-quick-reference). Inline options such as `(?i)` for case-insensitive matching work:

```kiwi
println regex::test('(?i)^kiwi$', "KIWI")   # true
```

To match text literally, escape it first:

```kiwi
println regex::escape("1+1=2?")   # 1\+1=2\?
```

## As string methods

Strings have the same operations as [built-in methods](/kiwi/reference/builtin-methods/#regular-expressions), with the text first: `text.matches(pattern)`, `text.find(pattern)`, `text.scan(pattern)`, `text.rreplace(pattern, replacement)`, `text.rsplit(pattern)` and so on.

<!-- api: regex -->
