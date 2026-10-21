# console

> Terminal colors, clearing, cursor control, window size and keyboard input.

The `console` package controls the terminal: text and background colors, clearing the screen, showing or hiding the cursor, reading single keys, and reading passwords without echoing them.

## Colors

`fg(color)` sets the text color and `bg(color)` the background; `reset()` restores the defaults. Color names are case-insensitive:

`black`, `dark_blue`, `dark_green`, `dark_cyan`, `dark_red`, `dark_magenta`, `dark_yellow`, `gray`, `dark_gray`, `blue`, `green`, `cyan`, `red`, `magenta`, `yellow`, `white`

```kiwi
console::fg("green")
println "Build passed"
console::reset()

console::fg("white")
console::bg("dark_red")
println " FAILED "
console::reset()
```

For labeled status messages, [`cli::success`](cli.md) and friends handle the colors for you.

## Screen and cursor

```kiwi
console::clear()
console::title("Kiwi monitor")
console::cursor_visible(false)

size = console::size()
println "Terminal is ${size["width"]}x${size["height"]}"

repeat 5 as i do
  print "\rWorking... ${i * 20}%"
  task::sleep(200)
end
println ""
console::cursor_visible(true)
```

To print at a specific position, use the [`printxy`](../reference/global-functions.md#output) statement.

## Input

```kiwi
name = console::read("Name: ")
password = console::read_secret("Password: ", "*")   # echoes * for each key
key = console::readkey()                             # one key press, no Enter needed
```

## Functions

### `console::bg`

```kiwi
console::bg(color: string)
```

Sets the terminal background color.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `color` | `string` |  | The color name (e.g., `"RED"`, `"BLUE"`). See `console::get_colors()`. |

### `console::fg`

```kiwi
console::fg(color: string)
```

Sets the terminal foreground (text) color.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `color` | `string` |  | The color name (e.g., `"GREEN"`, `"WHITE"`). See `console::get_colors()`. |

### `console::map_color`

```kiwi
console::map_color(name: string): integer
```

Maps a color name to its integer code.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | The color name (case-insensitive). |

**Returns** `integer` — The color code, or `0` (black) if the name is not recognized.

### `console::get_colors`

```kiwi
console::get_colors(): hashmap
```

Returns a hashmap of supported color names to their integer codes.

**Returns** `hashmap` — A map of color name strings to integer codes.

```kiwi
colors = console::get_colors()
println(colors["RED"])  # 12
```

### `console::reset`

```kiwi
console::reset()
```

Resets the terminal foreground and background colors to their defaults.

### `console::clear`

```kiwi
console::clear()
```

Clears the terminal screen.

### `console::cursor_visible`

```kiwi
console::cursor_visible(is_visible: boolean)
```

Shows or hides the terminal cursor.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `is_visible` | `boolean` |  | Pass `true` to show the cursor, `false` to hide it. |

### `console::size`

```kiwi
console::size()
```

Returns the current terminal window size.

**Returns** `hashmap` — A hashmap with `width` and `height` keys.

### `console::title`

```kiwi
console::title(new_title: string)
```

Sets the terminal window title.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `new_title` | `string` |  | The title to display in the terminal window. |

### `console::read`

```kiwi
console::read(msg: string = "")
```

Reads a line of input from the console with an optional prompt.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `""` | An optional prompt to display before reading input. |

**Returns** `string` — The input entered by the user.

### `console::readline`

```kiwi
console::readline(msg: string = "")
```

Reads a line of input from the console with an optional prompt.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `""` | An optional prompt to display before reading input. |

**Returns** `string` — The input entered by the user.

### `console::read_secret`

```kiwi
console::read_secret(msg: string = "", mask: string = "")
```

Reads a line of secret input (e.g., a password) without echoing characters.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `""` | An optional prompt to display. |
| `mask` | `string` | `""` | An optional mask character to display in place of input (default none). |

**Returns** `string` — The secret input entered by the user.

### `console::readkey`

```kiwi
console::readkey(intercept: boolean = true)
```

Reads a single keypress from the console.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `intercept` | `boolean` | `true` | If `true`, the key is not echoed to the console (default `true`). |

**Returns** `hashmap` — A hashmap with key info (e.g., `key`, `modifiers`).

### `console::read_key`

```kiwi
console::read_key(intercept: boolean = true)
```

Reads a single keypress from the console.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `intercept` | `boolean` | `true` | If `true`, the key is not echoed to the console (default `true`). |

**Returns** `hashmap` — A hashmap with key info (e.g., `key`, `modifiers`).

### `console::write`

```kiwi
console::write(msg: string = "")
```

Writes a message to the console without a trailing newline.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `""` | The message to write. |

### `console::writeln`

```kiwi
console::writeln(msg: string = "")
```

Writes a message to the console followed by a newline.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `""` | The message to write. |

### `console::writeline`

```kiwi
console::writeline(msg: string = "")
```

Writes a message to the console followed by a newline.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `""` | The message to write. |
