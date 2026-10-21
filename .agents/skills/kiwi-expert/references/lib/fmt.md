# fmt

> printf-style string formatting with % specifiers.

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
| `%e` | A float in scientific notation, with 6 decimal places unless a precision is given |
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
| `.N` | Decimal places for `%f` and `%e`, or maximum length for `%s` | `%.2f`, `%.3s` |

```kiwi
println fmt::sprintf("%e", [12345.678])   # 1.234568e+04
println fmt::sprintf("%.2e", [0.00012])   # 1.20e-04
```

For most formatting, string interpolation and [`.to_string(format)`](../language/types.md#number-formats) are simpler. For templates with named placeholders, see [`template`](template.md).

## Functions

### `fmt::sprintf`

```kiwi
fmt::sprintf(format_str, args = [])
```

Formats a string using printf-style format specifiers.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `format_str` | `string` |  | Format string with % specifiers. |
| `args` | `list` | `[]` | List of values to format. |

**Returns** `string` — The formatted string.

```kiwi
import fmt
println fmt::sprintf("Hello, %s! You are %d years old.", ["Alice", 30])
println fmt::sprintf("Pi is approximately %.4f", [3.14159265])
println fmt::sprintf("Hex: %x  Oct: %o  Bin: %b", [255, 255, 255])
println fmt::sprintf("%-10s|%10s", ["left", "right"])
```

### `fmt::printf`

```kiwi
fmt::printf(format_str, args = [])
```

Formats and prints to stdout (no newline).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `format_str` | `string` |  | Format string. |
| `args` | `list` | `[]` | Arguments list. |

### `fmt::printlnf`

```kiwi
fmt::printlnf(format_str, args = [])
```

Formats and prints to stdout with a newline.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `format_str` | `string` |  | Format string. |
| `args` | `list` | `[]` | Arguments list. |
