---
title: REPL
description: "The interactive Kiwi prompt: evaluating expressions, multi-line input and key bindings."
---

The REPL (read-eval-print loop) runs Kiwi code as you type it. Start it by running `kiwi` with no script, or with `kiwi -i`:

```text
$ kiwi
kiwi 1.6.2 - type 'exit' or press Ctrl+C to quit.
>>
```

The standard library is loaded, so every package is ready to use. Named arguments given on the command line are available through `env::argv()`:

```text
$ kiwi -name=ada
>> env::argv()
=> {"name": "ada"}
```

## Evaluating code

Each input runs as soon as you press <kbd>Enter</kbd>. When the input produces a value other than `null`, the REPL prints it after `=>`, so you rarely need `println`:

```text
>> 1 + 2
=> 3
>> "hello".uppercase()
=> "HELLO"
>> x = 10
=> 10
>> [1 to 5].map(do (n) => n * x)
=> [10, 20, 30, 40, 50]
>> println "printed output"
printed output
```

Variables, functions and structs you define stay available for the rest of the session.

## Multi-line input

When a line opens a block (`fn`, `if`, `for`, `struct` and so on) the prompt changes to `..` and the REPL waits until the block is complete:

```text
>> fn square(n)
..   n * n
.. end
>> square(7)
=> 49
```

End a line with `\` to continue a statement on the next line without opening a block:

```text
>> total = 1 + \
..   2 + 3
=> 6
```

## Key bindings

| Keys | Action |
| :-- | :-- |
| <kbd>←</kbd> / <kbd>→</kbd> | Move the cursor |
| <kbd>Home</kbd> / <kbd>Ctrl</kbd>+<kbd>A</kbd> | Start of line |
| <kbd>End</kbd> / <kbd>Ctrl</kbd>+<kbd>E</kbd> | End of line |
| <kbd>Backspace</kbd> / <kbd>Delete</kbd> | Delete before / at the cursor |
| <kbd>Ctrl</kbd>+<kbd>K</kbd> | Delete to the end of the line |
| <kbd>Ctrl</kbd>+<kbd>U</kbd> | Delete to the start of the line |
| <kbd>Ctrl</kbd>+<kbd>W</kbd> | Delete the word before the cursor |
| <kbd>↑</kbd> / <kbd>↓</kbd> | Previous / next history entry |

## Leaving

Type `exit` or `.exit`, press <kbd>Ctrl</kbd>+<kbd>D</kbd> on an empty line, or press <kbd>Ctrl</kbd>+<kbd>C</kbd>.
