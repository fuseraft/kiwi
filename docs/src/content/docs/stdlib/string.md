---
title: string
description: String formatting and inspection helpers, available as string methods.
---

The `string` package adds formatting and inspection helpers to strings. Because the package [extends the `string` type](/kiwi/language/packages/#extending-built-in-types), every function can be called either way:

```kiwi
println string::titleize("the quick fox")   # The Quick Fox
println "the quick fox".titleize()           # The Quick Fox
```

The core string methods, such as `.split`, `.trim` and `.replace`, are [built in](/kiwi/reference/builtin-methods/#strings); this package covers the rest.

## Formatting

```kiwi
println "hELLO wORLD".capitalize()        # Hello world
println "hELLO wORLD".titleize()          # Hello World
println "  too   much   space ".squish()  # too much space
println "42".padstart(6, "0")             # 000042
println "ab".padend(5, ".")               # ab...
println "hi".center(8, "*")               # ***hi***
println "Hello, World!".slug()            # hello-world
```

`dedent` removes the indentation shared by every line, which tidies multi-line string literals:

```kiwi
fn usage()
  "
    Usage: tool [options]
      -h  show help
  ".dedent().trim()
end

println usage()
```

## Inspecting

```kiwi
println "12345".is_numeric()        # true
println "abc".is_alpha()            # true
println "abc123".is_alphanumeric()  # true
println "ABC".is_upper()            # true
println "abc".is_lower()            # true
println "hello world".contains_any(["xyz", "wor"])   # true
```

## Encoding

```kiwi
println "kiwi".base64encode()       # a2l3aQ==
println "a2l3aQ==".base64decode()   # kiwi
println "a b&c".urlencode()         # a+b%26c
println "a+b%26c".urldecode()       # a b&c
```

## Odds and ends

```kiwi
println "ab".echo(3)            # aaabbb
println "abc".interweave("123") # a1b2c3
println "kiwi".mirror()         # kiwiiwik
println "report.txt".to_path().ext()   # .txt (see the path package)
```

`shuffle` returns the characters in random order.

<!-- api: string -->
