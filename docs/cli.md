# Command-Line Interface

The Kiwi CLI provides an interface for working with Kiwi files.

### Basic Usage

```
kiwi [--flags] <script|args>
```

If no arguments are supplied, Kiwi enters an interactive Read-Evaluate-Print Loop (REPL) where you can type and execute Kiwi code directly in the terminal. To learn more about the REPL, please see [REPL](repl.md).

### Table of Contents
- [Options](#options)
  - [`-h`, `--help`](#-h---help)
  - [`-v`, `--version`](#-v---version)
  - [`-p`, `--parse <kiwi_code>`](#-p---parse-kiwi_code)
  - [`-n`, `--new <file_path>`](#-n---new-file_path)
  - [`-a`, `--ast <input_file_path>`](#-a---ast-input_file_path)
  - [`-m`, `--minify <input_file_path>`](#-m---minify-input_file_path)
  - [`-t`, `--tokenize <input_file_path>`](#-t---tokenize-input_file_path)
  - [`-s`, `--safemode`](#-s---safemode)
  - [`-ns`, `--no-std-lib`](#-ns---no-std-lib)
  - [`-<key>=<value>`](#-keyvalue)

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

### `-p`, `--parse <kiwi_code>`

Parses kiwi code as an argument.

```
kiwi -p 'println "hello".chars()' 
# Prints: ["h", "e", "l", "l", "o"]

kiwi -p 'println math::random(0, 100)' 
# Prints a random number between 0 and 100
```

Note: If a file with the same name already exists, the CLI will notify you to prevent accidental overwriting.<br><br>

### `-n`, `--new <file_path>`

Creates a new file with the `.🥝` extension (`.kiwi` on Windows). If you don't include the extension in `<file_path>`, it will be appended automatically.

```
kiwi -n filename         # Creates ./filename.🥝
kiwi -n ../app           # Creates ../app.🥝 in the parent directory
kiwi -n ./somedir/app    # Creates ./somedir/app.🥝 in the specified subdirectory
```

Note: If a file with the same name already exists, the CLI will notify you to prevent accidental overwriting.<br><br>

### `-a`, `--ast <input_file_path>`

Prints the abstract syntax tree of the input file.

```
kiwi -a filename         # Prints AST of the file.
```

### `-m`, `--minify <input_file_path>`

Creates a minified file with the `.min.🥝` extension. If you don't include the extension in `<file_path>`, it will be appended automatically.

```
kiwi -m filename         # Creates ./filename.min.🥝
```

### `-t`, `--tokenize <input_file_path>`

Tokenizes a file with the Kiwi lexer and prints the results to the standard output stream.

```
kiwi -t filename         # Prints tokens by line from the file
```

### `-s`, `--safemode`

Runs Kiwi in safe mode. This disables access to web, filesystem, and system shell functionality.

```
kiwi -t filename         # Prints tokens from the file in a minified way.
```

### `-ns`, `--no-std-lib`

Runs Kiwi without loading its standard library.

```
kiwi -ns filename.k        # Runs `filename.k` without loading the Kiwi standard library.
```

### `-<key>=<value>`

Sets a specific argument as a key-value pair, which can be used for various configuration purposes or to pass parameters into scripts.

```
kiwi -theme=dark -p 'print argv::opt("theme")' # Prints "dark"
```