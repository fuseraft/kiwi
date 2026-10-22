---
title: The kiwi Command
description: Running scripts, one-liners and piped code, command-line options, and passing arguments to scripts.
---

```text
kiwi [options] [script] [arguments]
```

## Ways to run code

| Command | What it does |
| :-- | :-- |
| `kiwi script.kiwi` | Run a script. |
| `kiwi` | Start the [REPL](/kiwi/tools/repl/) when stdin is a terminal. |
| `kiwi -e 'code'` | Run a string of code. |
| `echo 'code' \| kiwi` | Run code piped on stdin. |
| `kiwi -d script.kiwi` | Run a script in the [debugger](/kiwi/tools/debugger/). |
| `kiwi -c script.kiwi` | Check a script for syntax errors without running it. |

Scripts run from top to bottom. There is no entry-point function; if you define a `main` function, call it yourself. The standard library is loaded first, so every package is available without importing it.

A script can start with a shebang line and be run directly once it is executable:

```kiwi
#!/usr/bin/env kiwi
println "Hello from a script"
```

```sh
chmod +x hello.kiwi
./hello.kiwi
```

### One-liners

`-e` runs the code that follows it. Use shell `$'...'` quoting to include newlines:

```sh
kiwi -e 'println 6 * 7'
kiwi -e $'for i in [1 to 3] do\n  println i\nend'
```

### Piped input

When no script is given and stdin is not a terminal, Kiwi reads the whole of stdin as a program and runs it:

```sh
echo 'println "Hello from a pipe"' | kiwi
cat script.kiwi | kiwi -env=prod
```

To process piped *data* instead of code, run a script and read stdin with the [`stdin`](/kiwi/stdlib/stdin/) package.

## Options

| Option | Description |
| :-- | :-- |
| `-h`, `--help` | Print usage and the list of options. |
| `-v`, `--version` | Print the version. |
| `-i`, `--interactive` | Start the REPL, even when stdin isn't a terminal. |
| `-e`, `--execute <code>` | Run a string of code. |
| `-c`, `--check <file>` | Report every syntax error in a file without running it. Exits `0` if the file is clean and `1` otherwise. |
| `-d`, `--debug <file>` | Run a file in the [kdb debugger](/kiwi/tools/debugger/). |
| `-a`, `--ast <file>` | Print a file's syntax tree. |
| `-t`, `--tokens <file>` | Print a file's tokens. |
| `-n`, `--new <name>` | Create an empty script, adding `.kiwi` if needed. Refuses to overwrite an existing file. |
| `-sm`, `--safemode` | Disable file, network, process and system access (see below). |
| `-ns`, `--no-stdlib` | Don't load the standard library. |
| `-p`, `--stdlib-path <dir>` | Load the standard library from another directory. |
| `-cd`, `--crash-dump` | Append errors to `kiwi_crash.log` in the current directory. |
| `-s`, `--settings` | Print the interpreter settings. |
| `-<key>=<value>` | Pass a named argument to the script. |

### Checking syntax

`--check` parses the file (and the standard library) and reports every syntax error, not just the first, which makes it useful in editors and CI:

```text
$ kiwi -c broken.kiwi
[SyntaxError]: Unexpected token `*`.
File: broken.kiwi:1:9
x = 1 + *
        ^
[SyntaxError]: Unexpected token `+`.
File: broken.kiwi:2:7
```

```text
$ kiwi -c hello.kiwi
No syntax errors found in: hello.kiwi
```

### Safe mode

`--safemode` runs untrusted code without access to the outside world. These packages throw a `SafeModeError` when used: `fio`, `http`, `httpserver`, `socket`, `tls`, `udp`, `ipc`, `smtp`, `signal`, `sys`, `native::load` and `crypto::file_hash`, and `env::set` is blocked.

```text
$ kiwi -sm -e 'fio::read("secrets.txt")'
[SafeModeError]: 'fio' is not available in safe mode.
```

### Inspecting the parser

`--tokens` and `--ast` show how Kiwi reads a file, which helps when a syntax error is confusing:

```text
$ kiwi -a hello.kiwi
Generating AST: hello.kiwi

Program:
  PrintLine:
    Literal: "Hello, World!"
```

## Passing arguments to scripts

Everything after the script path is passed to the script. Read arguments with the [`env`](/kiwi/stdlib/env/) package:

- `env::args()` returns the raw arguments as a list, in order.
- `env::argv()` returns them parsed into a hashmap. `-key=value` becomes `key: "value"`, a bare `--flag` becomes `flag: "true"`, and a positional argument maps to itself.

```kiwi nocheck
# args.kiwi
println env::args()
println env::argv()
```

```text
$ kiwi args.kiwi report.csv -limit=10 --verbose
["report.csv", "-limit=10", "--verbose"]
{"report.csv": "report.csv", "limit": "10", "verbose": "true"}
```

Use `env::args()` for positional arguments. Because `env::argv()` is a hashmap, repeated positional values collapse into one key.

For anything beyond a couple of options, use `cli::CliParser` from the [`cli`](/kiwi/stdlib/cli/) package, which handles flags, typed options, defaults and help text.

## Exit codes

| Code | Meaning |
| :-- | :-- |
| `0` | The script finished normally. |
| `1` | An uncaught error, or a failed `--check`. |
| `n` | The script called `exit n`. |

## Settings

`kiwi-settings.json`, next to the `kiwi` binary, configures where the standard library lives, which file extensions count as Kiwi scripts, the default for safe mode, and the crash log path. Print the active settings with `kiwi -s`.
