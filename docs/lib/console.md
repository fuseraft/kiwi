# `console`

The `console` package provides idiomatic access to terminal/console manipulation features: colored output, screen clearing, cursor control, window properties, single-key input, and secure (masked) line input.

It wraps low-level console builtins with friendly names and sensible defaults, making interactive CLI scripts and tools easier to write in Kiwi.

## Table of Contents

- [Package Functions](#package-functions)
  - Color & Appearance
    - [`fg(color)`](#fgcolor)
    - [`bg(color)`](#bgcolor)
    - [`reset()`](#reset)
  - Screen Control
    - [`clear()`](#clear)
    - [`cursor_visible(is_visible)`](#cursor_visibleis_visible)
    - [`size()`](#size)
    - [`title(new_title)`](#titlenew_title)
  - Input
    - [`read(msg = "")` / `readline(msg = "")`](#readmsg----readlinemsg--)
    - [`read_secret(msg = "", mask = "")`](#read_secretmsg---mask--)
    - [`readkey(intercept = true)` / `read_key(intercept = true)`](#readkeyintercept--true--read_keyintercept--true)
  - Output Helpers
    - [`write(msg = "")`](#writemsg--)
    - [`writeln(msg = "")` / `writeline(msg = "")`](#writelnmsg----writelinemsg--)
- [Color Names](#color-names)
- [Usage Examples](#usage-examples)

## Package Functions

### `fg(color)`
Sets the **foreground** (text) color.

**Parameters**  
| Type     | Name    | Description                                      | Default |
|----------|---------|--------------------------------------------------|---------|
| `string` | `color` | Case-insensitive color name (see [Color Names](#color-names)) | —       |

**Returns** `none`

---

### `bg(color)`
Sets the **background** color.

**Parameters**  
Same as `fg()`.

**Returns** `none`

---

### `reset()`
Resets foreground and background colors to their default values.

**Parameters** None  
**Returns** `none`

---

### `clear()`
Clears the entire console screen.

**Parameters** None  
**Returns** `none`

---

### `cursor_visible(is_visible)`
Shows or hides the text cursor.

**Parameters**  
| Type      | Name         | Description                 | Default |
|-----------|--------------|-----------------------------|---------|
| `boolean` | `is_visible` | `true` = show, `false` = hide | —       |

**Returns** `none`

---

### `size()`
Returns the current console window dimensions.

**Parameters** None

**Returns**  
`hashmap` with keys:

| Key | Value |
| --- | ----- |
| `width`  | `integer` (columns) |
| `height` | `integer` (rows) |

---

### `title(new_title)`
Sets the title of the terminal window (supported on Windows and many Unix terminals).

**Parameters**  
| Type     | Name        | Description          | Default |
|----------|-------------|----------------------|---------|
| `string` | `new_title` | Window title text    | —       |

**Returns** `none`

---

### `read(msg = "")` / `readline(msg = "")`
Prints an optional prompt and reads a line of input (echoed normally).

**Parameters**  
| Type     | Name   | Description               | Default |
|----------|--------|---------------------------|---------|
| `string` | `msg`  | Prompt to display         | `""`    |

**Returns** `string` (input without the trailing newline)

**Aliases**: `readline`

---

### `read_secret(msg = "", mask = "")`
Reads a line of input without echoing characters (ideal for passwords).

**Parameters**  
| Type     | Name   | Description                                    | Default |
|----------|--------|------------------------------------------------|---------|
| `string` | `msg`  | Prompt to display                              | `""`    |
| `string` | `mask` | Single character to show instead of input (e.g. "*") | `""` (nothing shown) |

**Returns** `string` (input without trailing newline)

**Note**: Works cross-platform (uses termios on Unix-like systems).

---

### `readkey(intercept = true)` / `read_key(intercept = true)`
Reads a single key press and returns detailed information.

**Parameters**  
| Type      | Name        | Description                                            | Default |
|-----------|-------------|--------------------------------------------------------|---------|
| `boolean` | `intercept` | `true` = key is not echoed/displayed                   | `true`  |

**Returns** `hashmap` with keys:
| Key | Value |
| --- | ----- |
| `key`          | `string` (e.g. "A", "Enter", "F1", "LeftArrow") |
| `char`         | `string` (printable character or empty string) |
| `modifiers`    | `list` (e.g. `["Shift", "Control"]`) |
| `is_shift`     | `boolean` |
| `is_control`   | `boolean` |
| `is_alt`       | `boolean` |
| `is_printable` | `boolean` |

**Aliases**: `read_key`

---

### `write(msg = "")`
Writes text without adding a newline.

**Parameters**  
| Type     | Name   | Description     | Default |
|----------|--------|-----------------|---------|
| `string` | `msg`  | Text to output  | `""`    |

**Returns** `none`

---

### `writeln(msg = "")` / `writeline(msg = "")`
Writes text followed by a newline.

**Parameters** Same as `write()`  
**Returns** `none`

**Aliases**: `writeline`

## Color Names

These strings (case-insensitive) are accepted by [`fg()`](#fgcolor) and [`bg()`](#bgcolor):

- `black`, `dark_blue`, `dark_green`, `dark_cyan`, `dark_red`, `dark_magenta`, `dark_yellow`, `gray`
- `dark_gray`, `blue`, `green`, `cyan`, `red`, `magenta`, `yellow`, `white`

Internally mapped to 0–15 (standard ANSI console colors).

## Usage Examples

```kiwi
# Colored output
console::fg("cyan")
console::bg("black")
console::writeln("Welcome to Kiwi CLI!")
console::reset()
console::writeln()

# Progress indicator simulation
console::cursor_visible(false)
repeat 5 as i do
  console::write("\rWorking... ${i*20}%")
  task::sleep(400)
end
console::writeln("\nDone!")
console::cursor_visible(true)

# Secure password input
password = console::read_secret("Enter passphrase: ", "*")
console::writeln("Length: ${password.size()} characters")

console::writeln("\nGoodbye.")
```