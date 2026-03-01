# Console I/O

Kiwi provides four core built-in functions for basic console input and output:

| Function     | Output stream | Adds newline? | Typical use case                          |
|--------------|---------------|---------------|-------------------------------------------|
| `print`      | stdout        | No            | Building partial lines, progress bars     |
| `println`    | stdout        | Yes           | Normal output, logging, user messages     |
| `eprint`     | stderr        | No            | Error messages, warnings, diagnostics     |
| `eprintln`   | stderr        | Yes           | Fatal errors, panic messages, CLI feedback|

These four functions are **builtins** (not part of any package) and are always available.

For advanced features (colors, cursor control, clearing screen, reading single keys, etc.) see the [`console` package](./lib/console.md).

For file I/O, see the [`fio` package](./lib/fio.md) or the higher-level [`path` module](./lib/path.md).

## Table of Contents

- [Reading input](#reading-input)
  - [`input(prompt = "")`](#inputprompt--)

## Reading input

### `input(prompt = "")`

Reads a line from **stdin** until newline (Enter). 
Returns the line **without** the trailing newline character.

- If a `prompt` string is provided, it is printed **without** a newline before waiting for input.
- Blocks until the user presses Enter (or EOF is reached).

```kiwi
# Simple prompt
name = input("What is your name? ")
println("Hello, ${name}!")

# Validation loop (common pattern)
choice = ""
while !["y", "n", "yes", "no"].contains(choice.lowercase()) do
  choice = input("Continue? (y/n): ").trim()
end
```