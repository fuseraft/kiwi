### Using the Astral CLI

The Astral CLI provides an interface for working with Astral files.

#### Basic Usage

```
astral [--flags] <script|args>
```

If no arguments are supplied, Astral enters an interactive Read-Evaluate-Print Loop (REPL) where you can type and execute Astral code directly in the terminal. To learn more about the REPL, please see [REPL](repl.md).

#### Options

- `-h`, `--help`: Displays help information with a summary of all available commands and their purposes.

  ```
  astral -h
  ```

- `-v`, `--version`: Prints the current version.

  ```
  astral -v
  ```

- `-p`, `--parse <astral_code>`: Parses astral code as an argument.

  ```
  astral -p "println \"hello\"" 
  # prints: hello
  
  astral -p "import \"@astral/math\" as math println math.random(0, 100)" 
  # prints a random number between 0 and 100
  ```

  Note: If a file with the same name already exists, the CLI will notify you to prevent accidental overwriting.<br><br>

- `-n`, `--new <file_path>`: Creates a new file with the `.🚀` extension. If you don't include the extension in `<file_path>`, it will be appended automatically.

  ```
  astral -n filename         # Creates ./filename.🚀
  astral -n ../app           # Creates ../app.🚀 in the parent directory
  astral -n ./somedir/app    # Creates ./somedir/app.🚀 in the specified subdirectory
  ```

  Note: If a file with the same name already exists, the CLI will notify you to prevent accidental overwriting.<br><br>

- `-m`, `--minify <input_file_path>`: Creates a minified file with the `.min.🚀` extension.

  ```
  astral -m filename.🚀      # Creates ./filename.min.🚀
  ```

- `-C`, `--config <conf_path>`: Allows you to specify a configuration file in `.conf` format to customize the Astral environment or project settings.

  Example:
  ```
  astral -C ./config/astral.conf
  ```

- `-X<key>:<value>`: Sets a specific argument as a key-value pair, which can be used for various configuration purposes or to pass parameters into scripts.

  Example:
  ```
  astral -Xtheme:dark -Xoptimization:2
  ```