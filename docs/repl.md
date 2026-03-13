# REPL

The REPL starts automatically when you run `kiwi` with no script and stdin is a terminal. You can also start it explicitly with `-i` or `--interactive`.

```
$ kiwi
kiwi 1.4.0 — type 'exit' or press Ctrl+C to quit.
>>

$ kiwi -i
kiwi 1.4.0 — type 'exit' or press Ctrl+C to quit.
>>
```

Key-value arguments can be passed alongside and will be available inside the session via `env::argv()`:

```
$ kiwi -name=scotty
kiwi 1.4.0 — type 'exit' or press Ctrl+C to quit.
>> env::argv()
=> {"name": "scotty"}
```

## Expressions are printed automatically

The REPL evaluates each input and prints the result when it is non-null. You do not need `println` for interactive exploration.

```
>> 1 + 2
=> 3
>> "hello"
=> "hello"
>> [1, 2, 3]
=> [1, 2, 3]
>> x = 10
=> 10
>> x * x
=> 100
```

Output from `println` and other print statements is still written directly — the auto-print only fires when there is a leftover value.

```
>> println "Hello, World!"
Hello, World!
>>
```

## Multi-line input

### Automatic block detection

The REPL detects incomplete blocks automatically. Type the opening line of a block and press Enter — the prompt switches to `.. ` indicating the REPL is waiting for the rest of the block.

```
>> for i in [1 to 3] do
..   println "i = ${i}"
.. end
i = 1
i = 2
i = 3
>>
```

This works for all block constructs: `for`, `while`, `if`, `fn`, `struct`, `try`, etc.

### Explicit continuation with `\`

End a line with `\` to join it with the next line. The prompt changes to `.. ` as long as continuation is active.

```
>> for v, i in ["hello", "world"] do \
..   println "${i}: ${v}" \
.. end
0: hello
1: world
```

## Key bindings

### Cursor movement

| Key | Action |
| :-- | :----- |
| **←** / **→** | Move cursor left / right |
| **Home** / **Ctrl+A** | Jump to start of line |
| **End** / **Ctrl+E** | Jump to end of line |

### Editing

| Key | Action |
| :-- | :----- |
| **Backspace** | Delete character before cursor |
| **Delete** | Delete character at cursor |
| **Ctrl+K** | Delete from cursor to end of line |
| **Ctrl+U** | Delete from cursor to start of line |
| **Ctrl+W** | Delete word before cursor |

### History

| Key | Action |
| :-- | :----- |
| **↑** | Previous history entry |
| **↓** | Next history entry (or restore in-progress input) |

### Exiting

| Action | Effect |
| :----- | :----- |
| Type `exit` or `.exit` and press **Enter** | Clean exit |
| **Ctrl+C** | Interrupt and exit |
| **Ctrl+D** on an empty line | EOF — exits cleanly |
