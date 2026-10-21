---
title: stdin
description: Read piped or redirected standard input.
---

The `stdin` package reads standard input, which lets a Kiwi script work as a filter in a shell pipeline:

```sh
cat access.log | kiwi count-errors.kiwi
```

```kiwi nocheck
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

```kiwi nocheck
while !stdin::empty() do
  line = stdin::readline()
  println line.uppercase()
end
```

:::caution
`stdin::lines()` currently never returns. Use `stdin::read().rtrim().lines()` or the loop above instead.
:::

To prompt the user interactively, use [`input`](/kiwi/reference/global-functions/#input) or [`console::read`](/kiwi/stdlib/console/).

<!-- api: stdin -->
