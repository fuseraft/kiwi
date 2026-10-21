---
title: Debugger (kdb)
description: Stepping through code, breakpoints and inspecting variables with kdb.
---

`kdb` is Kiwi's command-line debugger. It pauses a program before each statement so you can step through it, inspect and change variables, and set breakpoints, much like `gdb` or Python's `pdb`.

Start it with `-d` (or `--debug`):

```sh
kiwi -d script.kiwi
```

The debugger pauses before the first statement, shows the surrounding source, and waits at the `(kdb)` prompt. The `=>` arrow marks the line that is about to run. Standard library code is skipped: kdb only stops in your files.

## Commands

| Command | Short | Action |
| :-- | :-- | :-- |
| `step` | `s` | Run the current statement, stepping into function calls. |
| `next` | `n` | Run the current statement, stepping over function calls. |
| `finish` | `f` | Run until the current function returns. |
| `run` | `r` | Continue until the next breakpoint or the end of the program. |
| `b <line>` | | Set a breakpoint at a line in the current file. |
| `b <file>:<line>` | | Set a breakpoint in another file. |
| `info b` | | List breakpoints with their numbers. |
| `d <n>` | | Delete breakpoint number `n`. |
| `p <expression>` | | Evaluate an expression in the current scope and print it. |
| `locals` | `l` | Show the variables in the current scope. |
| `backtrace` | `bt` | Show the call stack. |
| `list [line]` | | Show the source around the current line, or around `line`. |
| `help` | `h` | Show the command list. |
| `quit` | `q` | Stop debugging and exit. |

`p` runs real Kiwi code against the paused program, so it can also change state: `p total = 0` resets a variable before you continue.

## Example session

Given `cart.kiwi`:

```kiwi
fn total(items)
  sum = 0
  for item in items do
    sum += item
  end
  sum
end

prices = [3, 4, 5]
result = total(prices)
println "Total: ${result}"
```

Set a breakpoint inside the loop, run to it, and look around:

```text
$ kiwi -d cart.kiwi
kdb - Kiwi Debugger
Debugging: cart.kiwi
Type 'h' for help, 's' to step, 'r' to run.

=> cart.kiwi:1
=>    1: fn total(items)
      2:   sum = 0
      3:   for item in items do
(kdb) b 4
Breakpoint 1 at cart.kiwi:4
(kdb) r

=> cart.kiwi:4
      2:   sum = 0
      3:   for item in items do
=>    4:     sum += item
      5:   end
      6:   sum
(kdb) bt
  #1 at total in cart.kiwi:4:5
  #0 at <main> in cart.kiwi:10:10
(kdb) l
  items = [3, 4, 5]
  sum = 0
  item = 3
(kdb) p sum + item
3
(kdb) d 1
Deleted breakpoint 1
(kdb) r
Total: 12
```
