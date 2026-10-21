# stdin

> Read piped or redirected standard input.

The `stdin` package reads standard input, which lets a Kiwi script work as a filter in a shell pipeline:

```sh
cat access.log | kiwi count-errors.kiwi
```

```kiwi
# count-errors.kiwi
errors = stdin::read().rtrim().lines().filter(do (l) => l.contains("ERROR"))
println "${errors.size()} errors"
```

| Function | Returns |
| :-- | :-- |
| `read()` | Everything remaining on stdin, as a string. |
| `readline()` | The next line, without its newline; `""` at the end of input. |
| `readbytes()` | Everything remaining, as bytes. |
| `empty()` | Whether the input has been fully read. |

To process input line by line as it arrives, loop on `readline` until `empty()`:

```kiwi
while !stdin::empty() do
  line = stdin::readline()
  println line.uppercase()
end
```

To prompt the user interactively, use [`input`](../reference/global-functions.md#input) or [`console::read`](console.md).

## Functions

### `stdin::read`

```kiwi
stdin::read(): string
```

Reads all of standard input as a string.

**Returns** `string` — The full stdin contents.

### `stdin::readbytes`

```kiwi
stdin::readbytes(): bytes
```

Reads all of standard input as a byte array.

**Returns** `bytes` — The full stdin contents as bytes.

### `stdin::readline`

```kiwi
stdin::readline(): string
```

Reads a single line from standard input.

**Returns** `string` — The next line from stdin, without a trailing newline.

### `stdin::lines`

```kiwi
stdin::lines(): list
```

Reads all lines from standard input into a list.

**Returns** `list` — A list of strings, one per line.

### `stdin::empty`

```kiwi
stdin::empty(): boolean
```

Returns true if standard input has no data available.

**Returns** `boolean` — `true` if stdin is empty, `false` otherwise.
