# `cli`

The `cli` package provides tools for building command-line interface (CLI) applications. It includes colored output helpers, formatted tables, a progress bar, interactive prompts, and the `CliParser` struct for declarative flag/option parsing.

---

## Package Functions

### `info(msg)`
Prints an informational message with a cyan `info` label.

**Parameters**

| Type     | Name  | Description         | Default |
|----------|-------|---------------------|---------|
| `string` | `msg` | The message to print | —      |

**Returns** `null`

---

### `warn(msg)`
Prints a warning message with a yellow `warn` label.

**Parameters**

| Type     | Name  | Description         | Default |
|----------|-------|---------------------|---------|
| `string` | `msg` | The message to print | —      |

**Returns** `null`

---

### `error(msg)`
Prints an error message with a red `error` label.

**Parameters**

| Type     | Name  | Description         | Default |
|----------|-------|---------------------|---------|
| `string` | `msg` | The message to print | —      |

**Returns** `null`

---

### `success(msg)`
Prints a success message with a green `ok` label.

**Parameters**

| Type     | Name  | Description         | Default |
|----------|-------|---------------------|---------|
| `string` | `msg` | The message to print | —      |

**Returns** `null`

---

### `header(title)`
Prints a section header with the title underlined by `=` characters.

**Parameters**

| Type     | Name    | Description        | Default |
|----------|---------|--------------------|---------|
| `string` | `title` | The section title  | —       |

**Returns** `null`

---

### `hr(width = 40)`
Prints a horizontal rule made of `-` characters.

**Parameters**

| Type      | Name    | Description         | Default |
|-----------|---------|---------------------|---------|
| `integer` | `width` | Width of the rule   | `40`    |

**Returns** `null`

---

### `kv(key, val, width = 16)`
Prints a key-value pair with the key padded to a fixed column width.

**Parameters**

| Type      | Name    | Description                        | Default |
|-----------|---------|------------------------------------|---------|
| `string`  | `key`   | The label                          | —       |
| `any`     | `val`   | The value                          | —       |
| `integer` | `width` | Column width for the key           | `16`    |

**Returns** `null`

---

### `table(headers, rows)`
Prints a formatted table with auto-sized columns and box-drawing borders.

**Parameters**

| Type   | Name      | Description                              | Default |
|--------|-----------|------------------------------------------|---------|
| `list` | `headers` | Column header strings                    | —       |
| `list` | `rows`    | List of row lists (each row is a `list`) | —       |

**Returns** `null`

**Example**
```kiwi
cli::table(
  ["Name", "Age", "City"],
  [
    ["Alice", 30, "London"],
    ["Bob",   25, "Paris"]
  ]
)
```

Output:
```
+---------+-----+--------+
| Name    | Age | City   |
+---------+-----+--------+
| Alice   | 30  | London |
| Bob     | 25  | Paris  |
+---------+-----+--------+
```

---

### `progress_bar(current, total, width = 30)`
Returns a progress bar string suitable for printing in-place with `\r`.

**Parameters**

| Type      | Name      | Description                     | Default |
|-----------|-----------|---------------------------------|---------|
| `integer` | `current` | Current progress value          | —       |
| `integer` | `total`   | Total (100%) value              | —       |
| `integer` | `width`   | Bar width in characters         | `30`    |

**Returns** `string` — e.g. `[########......................] 27%`

**Example**
```kiwi
for i in [0 to 10] do
  print "\r" + cli::progress_bar(i, 10)
end
println ""
```

---

### `prompt(msg = "")`
Prints a prompt and reads a line of input from the user.

**Parameters**

| Type     | Name  | Description           | Default |
|----------|-------|-----------------------|---------|
| `string` | `msg` | Prompt text to display | `""`   |

**Returns** `string`

---

### `confirm(msg = "")`
Prints a `[y/N]` prompt and returns `true` if the user types `y` or `Y`.

**Parameters**

| Type     | Name  | Description           | Default |
|----------|-------|-----------------------|---------|
| `string` | `msg` | Prompt text to display | `""`   |

**Returns** `boolean`

---

## `CliParser`

A struct for declaratively defining and parsing command-line flags and options.

### `CliParser.new(name, desc = "")`
Creates a new parser instance.

**Parameters**

| Type     | Name   | Description                    | Default |
|----------|--------|--------------------------------|---------|
| `string` | `name` | Program name (shown in help)   | —       |
| `string` | `desc` | Short program description      | `""`    |

---

### `@.flag(name, short = "", desc = "")`
Registers a boolean flag. When present on the command line, it is `true`; otherwise `false`.

**Parameters**

| Type     | Name    | Description                          | Default |
|----------|---------|--------------------------------------|---------|
| `string` | `name`  | Long flag name (e.g. `"verbose"`)    | —       |
| `string` | `short` | Single-character alias (e.g. `"v"`)  | `""`    |
| `string` | `desc`  | Description shown in help output     | `""`    |

---

### `@.option(name, short = "", desc = "", dval = null)`
Registers an option that takes a value argument.

**Parameters**

| Type     | Name    | Description                             | Default  |
|----------|---------|-----------------------------------------|----------|
| `string` | `name`  | Long option name (e.g. `"output"`)      | —        |
| `string` | `short` | Single-character alias (e.g. `"o"`)     | `""`     |
| `string` | `desc`  | Description shown in help output        | `""`     |
| `any`    | `dval`  | Default value when option is not given  | `null`   |

---

### `@.help()`
Prints formatted help text listing all registered flags and options with their defaults.

**Returns** `null`

---

### `@.parse()`
Parses `env::argv()` against the registered flags and options.

**Returns** `hashmap` with one key per flag/option (using the long name), plus `"_args"` for any positional arguments.

| Key        | Value |
|------------|-------|
| `"<flag>"` | `boolean` — `true` if flag was passed |
| `"<option>"` | `any` — parsed value, or the registered default |
| `"_args"`  | `list` — positional arguments (not prefixed with `-`) |

Flags and options are accepted in both long form (`--name`) and short form (`-n`).

---

## Usage Example

```kiwi
p = CliParser.new("myapp", "A sample CLI application")
p.flag("verbose", "v", "Enable verbose output")
p.flag("dry-run", "n", "Simulate without writing files")
p.option("output", "o", "Output file path", "stdout")
p.option("format", "f", "Output format (text, json, csv)", "text")

p.help()

args = p.parse()

cli::kv("verbose", args["verbose"])
cli::kv("output",  args["output"])
cli::kv("_args",   args["_args"])
```

Running `kiwi myapp.kiwi --verbose -o result.txt data.csv` would produce:

```
verbose          true
output           result.txt
_args            [data.csv]
```
