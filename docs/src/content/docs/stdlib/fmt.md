---
title: fmt
description: printf-style string formatting with % specifiers.
---

The `fmt` package formats strings with `%` specifiers, like `printf` in C. Pass the values as a list.

```kiwi
println fmt::sprintf("Hello, %s!", ["World"])            # Hello, World!
println fmt::sprintf("%d + %d = %d", [1, 2, 3])          # 1 + 2 = 3
println fmt::sprintf("Pi: %.4f", [3.14159265])           # Pi: 3.1416
println fmt::sprintf("|%-10s|%10s|", ["left", "right"])  # |left      |     right|
println fmt::sprintf("%08d", [42])                       # 00000042
println fmt::sprintf("0x%x 0%o %b", [255, 255, 255])     # 0xff 0377 11111111

fmt::printlnf("User: %s, score: %d", ["Alice", 99])      # User: Alice, score: 99
```

`printf` prints without a trailing newline and `printlnf` with one.

## Specifiers

| Specifier | Formats |
| :-- | :-- |
| `%s` | A string, or any value converted to a string |
| `%d`, `%i` | An integer |
| `%f` | A float, with 6 decimal places unless a precision is given |
| `%x`, `%X` | Hexadecimal, lower or upper case |
| `%o` | Octal |
| `%b` | Binary |
| `%c` | The character for a code point |
| `%%` | A literal `%` |

Between the `%` and the letter you can add:

| Flag | Meaning | Example |
| :-- | :-- | :-- |
| A number | Minimum width, right-aligned | `%10s` |
| `-` | Left-align within the width | `%-10s` |
| `0` | Pad numbers with zeros | `%08d` |
| `+` | Always show the sign | `%+d` |
| `.N` | Decimal places for `%f`, or maximum length for `%s` | `%.2f`, `%.3s` |

:::caution
`%e` (scientific notation) currently fails with an error.
:::

For most formatting, string interpolation and [`.to_string(format)`](/kiwi/language/types/#number-formats) are simpler. For templates with named placeholders, see [`template`](/kiwi/stdlib/template/).

<!-- api: fmt -->
