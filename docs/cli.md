# Command-Line Interface

The Kiwi CLI provides an interface for working with Kiwi files.

## Basic Usage

```
kiwi [--flags] <script|args>
```

## Options

### `-h`, `--help`

Displays help information with a summary of all available commands and their purposes.

```
kiwi -h
```

### `-v`, `--version`

Prints the current version.

```
kiwi -v
```

### `-s`, `--settings`

Prints the interpreter settings (for debugging).

```
kiwi -s
```

### `-i`, `--interactive`

Starts an interactive Read-Evaluate-Print Loop (REPL) where you can type and execute Kiwi code directly in the terminal.

The REPL also starts automatically when you run `kiwi` with no script and stdin is a terminal — `-i` is only needed when you want to force interactive mode explicitly (e.g. after loading a script).

To learn more about the REPL, please see [REPL](repl.md).

```
kiwi          # starts REPL automatically when stdin is a terminal
kiwi -i       # explicit
```

### `-e`, `--execute <code>`

Executes a string of Kiwi code directly, without needing a file. The standard library is loaded as normal.

```
kiwi -e 'println "Hello!"'
kiwi --execute 'println "Hello!"'
```

For multi-line code, use shell `$'...'` quoting with `\n`:

```bash
kiwi -e $'x = 6 * 7\nprintln x'
# 42
```

Key-value arguments can be passed alongside:

```bash
 kiwi -e 'println env::argv()' -name=world
# {"name": "world"}
```

### `-d`, `--debug <input_file_path>`

Runs a script under the `kdb` interactive debugger. Execution pauses at the first statement and presents a `(kdb)` prompt where you can step through the program, inspect variables, set breakpoints, and evaluate expressions.

To learn more, see [kdb — Kiwi Debugger](kdb.md).

```
kiwi -d filename         # Debug the file with kdb.
```

### `-c`, `--check <input_file_path>`

Checks a script for syntax errors without executing it. All errors in the file are reported — not just the first one. Exits with code `0` if the file is clean, `1` if any errors were found.

```
kiwi -c script.kiwi      # Check for syntax errors.
kiwi --check script.kiwi
```

Example output for a file with two errors:

```
[SyntaxError]: Unexpected token `*`.
File: script.kiwi:1:9
x = 1 + *
        ^
[SyntaxError]: Unexpected token `/`.
File: script.kiwi:2:9
y = 2 + /
        ^
```

Example output for a clean file:

```
No syntax errors found in: script.kiwi
```

### `-a`, `--ast <input_file_path>`

Prints the abstract syntax tree of the input file.

```
kiwi -a filename         # Prints AST of the file.
```

### `-t`, `--tokens <input_file_path>`

Tokenizes a file with the Kiwi lexer and prints the results to the standard output stream.

```
kiwi -t filename         # Prints tokens by line from the file
```

### `-n`, `--new <file_path>`

Creates a new file with the `.kiwi` extension. If you don't include the extension in `<file_path>`, it will be appended automatically.

```
kiwi -n filename         # Creates ./filename.kiwi
kiwi -n ../app           # Creates ../app.kiwi in the parent directory
kiwi -n ./somedir/app    # Creates ./somedir/app.kiwi in the specified subdirectory
```

Note: If a file with the same name already exists, the CLI will notify you to prevent accidental overwriting.<br><br>

### `-sm`, `--safemode`

Runs Kiwi in safe mode. This disables access to web, filesystem, and system shell functionality.

```
kiwi -sm filename        # Runs the file in safe mode.
```

### `-ns`, `--no-stdlib`

Runs Kiwi without loading its standard library.

```
kiwi -ns filename.k        # Runs `filename.k` without loading the Kiwi standard library.
```

### `-p`, `--stdlib-path <path>`

Overrides the standard library path. Kiwi will load `.kiwi` files from the specified directory instead of the default stdlib location.

```
kiwi -p ~/my-stdlib script.kiwi
```

### `-cd`, `--crash-dump`

Enables crash dump logging. When set, errors and unhandled exceptions are appended to `kiwi_crash.log` in the current directory. Without this flag, errors are only printed to stderr and no file is written.

```
kiwi -cd script.kiwi
kiwi --crash-dump script.kiwi
```

The log path can also be set permanently via `"crashdump_path"` in `kiwi-settings.json`.

### `--tree-walker`

Runs a script using the tree-walking interpreter instead of the default bytecode VM. This flag must appear before the script path. A script is required — running `kiwi --tree-walker` without one is an error.

```
kiwi --tree-walker script.kiwi
```

### `-<key>=<value>`

Sets a specific argument as a key-value pair, which can be used for various configuration purposes or to pass parameters into scripts.

```
kiwi myapp -theme=dark
```

Key-value arguments can also be passed without a script. When stdin is a terminal, the REPL starts and the arguments are available via `env::argv()`:

```
kiwi -name=scotty
```

If stdin is piped, the piped code is run with the arguments available as usual:

```
echo 'println env::argv()' | kiwi -name=scotty
# {"name": "scotty"}
```

### Bare positional arguments

Arguments with no leading `-`/`--`/`/` and no `=` are **positional arguments** — e.g. the numbers in `kiwi bubblesort.kiwi 5 3 8 1`. They are not dropped or special-cased: each one becomes its own key in the `env::argv()` hashmap, mapped to itself (`argv["5"] == "5"`).

```
kiwi -e 'println env::argv()' 5 3 8 1
# {"5": "5", "3": "3", "8": "8", "1": "1"}
```

To recover them as an ordered list (the common case for a script that takes `script.kiwi item1 item2 ...`), use `env::args()` — the raw, unparsed argument list:

```kiwi
data = env::args()   # ["5", "3", "8", "1"], in argument order
```

**Don't use `env::argv().keys()` for this.** `argv()` is a hashmap, so duplicate positional values collapse into one entry before your script sees them — `kiwi script.kiwi 5 3 8 3 1` gives `argv().keys() == ["5","3","8","1"]`, silently dropping the repeated `3`. `env::args()` has no such issue since it isn't backed by a hashmap. See [`env::args()`](lib/env.md#args) for details, and [`cli::CliParser`](lib/cli.md#cliparser) if you need to mix flags/options with positional arguments — it separates parsed flags/options from a plain `"_args"` list of positionals (note: `CliParser`'s `"_args"` is currently derived from `argv()` internally and inherits the same duplicate-collapsing limitation).