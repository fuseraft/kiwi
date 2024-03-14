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

- `-n`, `--new <filename>`: Creates a new file with the `.🥝` extension. If you don't include the extension in `<filename>`, it will be appended automatically.

  ```
  kiwi -n filename         # Creates ./filename.🥝
  kiwi -n ../app           # Creates ../app.🥝 in the parent directory
  kiwi -n ./somedir/app    # Creates ./somedir/app.🥝 in the specified subdirectory
  ```

  Note: If a file with the same name already exists, the CLI will notify you to prevent accidental overwriting.

- `-C`, `--config <conf_path>`: Allows you to specify a configuration file in `.conf` format to customize the Kiwi environment or project settings.

  Example:
  ```
  kiwi -C ./config/kiwi.conf
  ```

- `-X<key>:<value>`: Sets a specific argument as a key-value pair, which can be used for various configuration purposes or to pass parameters into scripts.

  Example:
  ```
  kiwi -Xtheme:dark -Xoptimization:2
  ```