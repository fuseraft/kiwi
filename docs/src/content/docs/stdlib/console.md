---
title: console
description: Terminal colors, clearing, cursor control, window size and keyboard input.
---

The `console` package controls the terminal: text and background colors, clearing the screen, showing or hiding the cursor, reading single keys, and reading passwords without echoing them.

## Colors

`fg(color)` sets the text color and `bg(color)` the background; `reset()` restores the defaults. Color names are case-insensitive:

`black`, `dark_blue`, `dark_green`, `dark_cyan`, `dark_red`, `dark_magenta`, `dark_yellow`, `gray`, `dark_gray`, `blue`, `green`, `cyan`, `red`, `magenta`, `yellow`, `white`

```kiwi nocheck
console::fg("green")
println "Build passed"
console::reset()

console::fg("white")
console::bg("dark_red")
println " FAILED "
console::reset()
```

For labeled status messages, [`cli::success`](/kiwi/stdlib/cli/) and friends handle the colors for you.

## Screen and cursor

```kiwi nocheck
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

To print at a specific position, use the [`printxy`](/kiwi/reference/global-functions/#output) statement.

## Input

```kiwi nocheck
name = console::read("Name: ")
password = console::read_secret("Password: ", "*")   # echoes * for each key
key = console::readkey()                             # one key press, no Enter needed
```

<!-- api: console -->
