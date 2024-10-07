### Using the Kiwi CLI

The Kiwi CLI provides an interface for working with Kiwi files.

#### Basic Usage

```
kiwi [--flags] <script|args>
```

If no arguments are supplied, Kiwi enters an interactive Read-Evaluate-Print Loop (REPL) where you can type and execute Kiwi code directly in the terminal. To learn more about the REPL, please see [REPL](repl.md).

#### Options

- `-h`, `--help`: Displays help information with a summary of all available commands and their purposes.

  ```
  kiwi -h
  ```

- `-v`, `--version`: Prints the current version.

  ```
  kiwi -v
  ```

- `-p`, `--parse <kiwi_code>`: Parses kiwi code as an argument.

  ```
  kiwi -p 'println("hello")' 
  # prints: hello
  
  kiwi -p 'import "math" as math println(math.random(0, 100))' 
  # prints a random number between 0 and 100
  ```

  Note: If a file with the same name already exists, the CLI will notify you to prevent accidental overwriting.<br><br>

- `-n`, `--new <file_path>`: Creates a new file with the `.🥝` extension. If you don't include the extension in `<file_path>`, it will be appended automatically.

  ```
  kiwi -n filename         # Creates ./filename.🥝
  kiwi -n ../app           # Creates ../app.🥝 in the parent directory
  kiwi -n ./somedir/app    # Creates ./somedir/app.🥝 in the specified subdirectory
  ```

  Note: If a file with the same name already exists, the CLI will notify you to prevent accidental overwriting.<br><br>

- `-m`, `--minify <input_file_path>`: Creates a minified file with the `.min.🥝` extension. If you don't include the extension in `<file_path>`, it will be appended automatically.

  ```
  kiwi -m filename         # Creates ./filename.min.🥝
  ```

- `-t`, `--tokenize <input_file_path>`: Tokenizes a file with the Kiwi lexer and prints the results to the standard output stream.

  ```
  kiwi -t filename.🥝      # Prints tokens from the file in a minified way.
  ```

- `-X<key>=<value>`: Sets a specific argument as a key-value pair, which can be used for various configuration purposes or to pass parameters into scripts.

  Example:
  ```
  kiwi -Xtheme=dark -Xoptimization=2
  ```