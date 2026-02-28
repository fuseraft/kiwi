# kdb — Kiwi Debugger

`kdb` is Kiwi's built-in command-line debugger. It lets you step through a program statement by statement, inspect variables, set breakpoints, and evaluate expressions interactively — similar to `gdb` or Python's `pdb`.

### Table of Contents
- [Starting the Debugger](#starting-the-debugger)
- [The Prompt](#the-prompt)
- [Stepping](#stepping)
  - [`s` / `step` — Step In](#s--step--step-in)
  - [`n` / `next` — Step Over](#n--next--step-over)
  - [`f` / `finish` — Step Out](#f--finish--step-out)
  - [`r` / `run` — Run to Breakpoint](#r--run--run-to-breakpoint)
- [Breakpoints](#breakpoints)
  - [`b` — Set Breakpoint](#b--set-breakpoint)
  - [`d` — Delete Breakpoint](#d--delete-breakpoint)
  - [`info b` — List Breakpoints](#info-b--list-breakpoints)
- [Inspection](#inspection)
  - [`p` — Print Expression](#p--print-expression)
  - [`l` / `locals` — Local Variables](#l--locals--local-variables)
  - [`bt` / `backtrace` — Call Stack](#bt--backtrace--call-stack)
  - [`list` — Show Source](#list--show-source)
- [Other Commands](#other-commands)
- [Workflow Example](#workflow-example)

---

## Starting the Debugger

Pass `-d` (or `--debug`) followed by a script path:

```
kiwi -d myscript.kiwi
```

The debugger prints a banner, pauses at the first statement in your script, and shows the surrounding source lines:

```
kdb - Kiwi Debugger
Debugging: myscript.kiwi
Type 'h' for help, 's' to step, 'r' to run.

=> /home/user/myscript.kiwi:3
      1: #!/usr/bin/env kiwi
      2:
=>    3: fn main()
      4:   println "hello"
      5: end
(kdb)
```

The `=>` arrow and the highlighted line number show the statement that is about to execute (not yet run).

---

## The Prompt

All debugger commands are entered at the `(kdb)` prompt. Pressing <kbd>Enter</kbd> on an empty line does nothing. Type `h` or `help` at any time to see a summary of available commands.

---

## Stepping

### `s` / `step` — Step In

Execute the current statement and pause at the very next one, stepping *into* any function calls.

```
(kdb) s
```

### `n` / `next` — Step Over

Execute the current statement and pause at the next one at the same call depth, *skipping over* function calls (they run but you don't trace into them).

```
(kdb) n
```

### `f` / `finish` — Step Out

Run until the current function returns, then pause at the next statement in the calling function.

```
(kdb) f
```

### `r` / `run` — Run to Breakpoint

Resume execution without pausing, stopping only when a breakpoint is hit (or when the program ends).

```
(kdb) r
```

---

## Breakpoints

### `b` — Set Breakpoint

Set a breakpoint at a line number in the **current file**:

```
(kdb) b 42
Breakpoint 1 at /home/user/myscript.kiwi:42
```

Or in a **specific file** using `file:line` format:

```
(kdb) b lib/helpers.kiwi:15
Breakpoint 2 at lib/helpers.kiwi:15
```

Breakpoints are hit when execution reaches that line in `run` mode.

### `d` — Delete Breakpoint

Delete breakpoint by its index (shown in `info b`):

```
(kdb) d 1
Deleted breakpoint 1
```

### `info b` — List Breakpoints

List all active breakpoints with their indices:

```
(kdb) info b
  1: /home/user/myscript.kiwi:42
  2: lib/helpers.kiwi:15
```

---

## Inspection

### `p` — Print Expression

Evaluate any Kiwi expression in the **current scope** and print the result:

```
(kdb) p x
42
(kdb) p x * 2 + 1
85
(kdb) p fib
[1, 2, 3, 5, 8, 13]
```

The expression is evaluated on the live call stack, so it can read and modify variables just like regular Kiwi code. Side effects apply.

### `l` / `locals` — Local Variables

Show all variables visible in the current scope (local frame plus any enclosing scopes):

```
(kdb) l
  n = 100000
  fib = [1, 2, 3, 5, 8]
  t1 = 3
  t2 = 5
  nextTerm = 8
  global = {}
```

### `bt` / `backtrace` — Call Stack

Print the current call stack, most recent frame at the bottom:

```
(kdb) bt
Stack:
at main in /home/user/myscript.kiwi:30:1
at ifib in /home/user/myscript.kiwi:5:37
```

### `list` — Show Source

Show ±5 source lines around the current statement:

```
(kdb) list
     15:
     16: fn ifib(n)
=>   17:   fib = [], t1 = 0, t2 = 1, nextTerm = 0
     18:   nextTerm = t1 + t2
     19:
     20:   while nextTerm <= n do
     21:     fib.push(nextTerm)
     22:     t1 = t2
```

You can also jump to a specific line number:

```
(kdb) list 42
```

---

## Other Commands

| Command | Description |
|---------|-------------|
| `h`, `help` | Show the command reference |
| `q`, `quit` | Exit the debugger immediately |

---

## Workflow Example

Given this script `examples/fibonacci.kiwi`:

```kiwi
fn main()
  println "iterative fibonacci: " + ifib(100000)
end

fn ifib(n)
  fib = [], t1 = 0, t2 = 1, nextTerm = 0
  nextTerm = t1 + t2

  while nextTerm <= n do
    fib.push(nextTerm)
    t1 = t2
    t2 = nextTerm
    nextTerm = t1 + t2
  end

  fib
end

main()
```

A typical debug session:

```
$ kiwi -d examples/fibonacci.kiwi

kdb - Kiwi Debugger
Debugging: examples/fibonacci.kiwi
Type 'h' for help, 's' to step, 'r' to run.

=> examples/fibonacci.kiwi:1
=>    1: fn main()
(kdb) b 6
Breakpoint 1 at examples/fibonacci.kiwi:6

(kdb) r
=> examples/fibonacci.kiwi:6
=>    6:   fib = [], t1 = 0, t2 = 1, nextTerm = 0

(kdb) bt
Stack:
at main in examples/fibonacci.kiwi:18:1
at ifib in examples/fibonacci.kiwi:2:37

(kdb) n

=> examples/fibonacci.kiwi:7
=>    7:   nextTerm = t1 + t2

(kdb) l
  n = 100000
  fib = []
  t1 = 0
  t2 = 1
  nextTerm = 0

(kdb) p nextTerm
0

(kdb) f
iterative fibonacci: [1, 2, 3, 5, ...]
```
