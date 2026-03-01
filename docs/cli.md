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

To learn more about the REPL, please see [REPL](repl.md).


```
kiwi -i
```

### `-d`, `--debug <input_file_path>`

Runs a script under the `kdb` interactive debugger. Execution pauses at the first statement and presents a `(kdb)` prompt where you can step through the program, inspect variables, set breakpoints, and evaluate expressions.

To learn more, see [kdb — Kiwi Debugger](kdb.md).

```
kiwi -d filename         # Debug the file with kdb.
```

### `-a`, `--ast <input_file_path>`

Prints the abstract syntax tree of the input file.

```
kiwi -a filename         # Prints AST of the file.
```

### `-t`, `--tokenize <input_file_path>`

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

### `-<key>=<value>`

Sets a specific argument as a key-value pair, which can be used for various configuration purposes or to pass parameters into scripts.

```
kiwi myapp -theme=dark
```