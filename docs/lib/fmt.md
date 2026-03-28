# `fmt`

The `fmt` package provides `sprintf`-style string formatting with `%` format specifiers, along with `printf` and `printlnf` convenience functions.

---

## Format Specifiers

| Specifier | Description                           |
|-----------|---------------------------------------|
| `%s`      | String                                |
| `%d`, `%i`| Signed decimal integer                |
| `%f`      | Floating-point (default 6 decimal places) |
| `%e`      | Scientific notation                   |
| `%x`      | Lowercase hexadecimal integer         |
| `%X`      | Uppercase hexadecimal integer         |
| `%o`      | Octal integer                         |
| `%b`      | Binary integer                        |
| `%c`      | Character from codepoint              |
| `%%`      | Literal `%`                           |

### Flags and Width

Flags appear between `%` and the specifier:

| Flag    | Description                                          |
|---------|------------------------------------------------------|
| `N`     | Minimum field width (e.g. `%10s` right-aligns in 10 chars) |
| `-`     | Left-align within the field width                    |
| `0`     | Zero-pad numbers to the field width                  |
| `+`     | Always show sign for numeric values                  |
| `.N`    | Precision: decimal places for `%f`/`%e`, max chars for `%s` |

---

## Package Functions

### `sprintf(format_str, args?)`

Formats a string using `%` specifiers.

**Parameters**

| Type     | Name         | Description                              | Default |
|----------|--------------|------------------------------------------|---------|
| `string` | `format_str` | Format string with `%` specifiers        | —       |
| `list`   | `args`       | Values to substitute, in order           | `[]`    |

**Returns**

| Type     | Description         |
|----------|---------------------|
| `string` | Formatted string    |

---

### `printf(format_str, args?)`

Formats and prints to stdout without a trailing newline.

**Parameters**
Same as `sprintf`.

---

### `printlnf(format_str, args?)`

Formats and prints to stdout with a trailing newline.

**Parameters**
Same as `sprintf`.

---

## Examples

```kiwi
import "fmt"

# Basic substitution
println fmt::sprintf("Hello, %s!", ["World"])          # Hello, World!

# Integers and floats
println fmt::sprintf("%d + %d = %d", [1, 2, 3])        # 1 + 2 = 3
println fmt::sprintf("Pi: %.4f", [3.14159265])          # Pi: 3.1416

# Width and alignment
println fmt::sprintf("|%-10s|%10s|", ["left", "right"]) # |left      |     right|
println fmt::sprintf("%08d", [42])                      # 00000042

# Number bases
println fmt::sprintf("0x%x  0%o  %b", [255, 255, 255]) # 0xff  0377  11111111

# Scientific notation
println fmt::sprintf("%.2e", [123456.789])              # 1.23e+05

# printf convenience
fmt::printlnf("User: %s, Score: %d", ["Alice", 99])     # User: Alice, Score: 99
```
