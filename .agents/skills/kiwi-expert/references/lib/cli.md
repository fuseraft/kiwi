# cli

> Building command-line tools: labeled messages, tables, progress bars, prompts and argument parsing.

The `cli` package has the pieces most command-line tools need: labeled status messages, aligned output, tables, a progress bar, prompts, and `CliParser` for flags and options.

## Output

```kiwi
cli::header("Deploy")
cli::info("Connecting to server")
cli::warn("Disk usage at 85%")
cli::success("Deployed v1.2.0")
cli::error("Health check failed")
cli::hr(20)
cli::kv("version", "1.2.0")
```

```text
Deploy
======
  info   Connecting to server
  warn   Disk usage at 85%
    ok   Deployed v1.2.0
 error   Health check failed
--------------------
version          1.2.0
```

On a color terminal the labels are colored. `cli::table` draws a bordered table, sizing each column to fit:

```kiwi
cli::table(["Service", "Status"], [["web", "up"], ["db", "down"]])
```

```text
+---------+--------+
| Service | Status |
+---------+--------+
| web     | up     |
| db      | down   |
+---------+--------+
```

`cli::progress_bar(current, total, width)` returns a progress bar as a string. Print it with a carriage return to redraw it in place:

```kiwi
repeat 10 as i do
  print "\r" + cli::progress_bar(i, 10, 20)
end
println ""
```

## Prompts

`cli::prompt(message)` asks for a line of input, and `cli::confirm(message)` asks a yes/no question:

```kiwi
name = cli::prompt("Project name: ")
exit 0 when !cli::confirm("Create ${name}?")
```

## Parsing arguments

`CliParser` turns command-line arguments into a hashmap. Register boolean flags with `flag(name, short, description)` and options that take a value with `option(name, short, description, default)`, then call `parse()`:

```kiwi
parser = CliParser.new("report", "Summarize a CSV file")
parser.flag("verbose", "v", "Print progress")
parser.option("format", "f", "Output format", "text")
parser.option("limit", "n", "Rows to show", "10")

# parse() reads the script's arguments; a list can be passed for testing.
args = parser.parse(["--verbose", "-f", "json", "data.csv"])

println args["verbose"]   # true
println args["format"]    # json
println args["limit"]     # 10
println args["_args"]     # ["data.csv"]
```

- Flags are `true` when given and `false` otherwise.
- Options take the next argument as their value, or their default when absent. Values are strings.
- Everything that isn't a flag or option is collected, in order, under `_args`.
- Both `--name` and the short `-n` form work.

`parser.help()` prints usage generated from the registered flags and options:

```text
Usage: report
Summarize a CSV file

Flags:
  -v, --verbose  Print progress

Options:
  -f, --format  Output format (default: text)
  -n, --limit  Rows to show (default: 10)
```

A typical script prints help and exits when asked:

```kiwi
parser.flag("help", "h", "Show this help")
args = parser.parse()

if args["help"]
  parser.help()
  exit 0
end
```

For raw access to arguments, see [`env::args()` and `env::argv()`](../tools/cli.md#passing-arguments-to-scripts).

## Functions

### `cli::info`

```kiwi
cli::info(msg: string)
```

Prints an info message.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` |  | The message to display. |

### `cli::warn`

```kiwi
cli::warn(msg: string)
```

Prints a warning message.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` |  | The message to display. |

### `cli::error`

```kiwi
cli::error(msg: string)
```

Prints an error message.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` |  | The message to display. |

### `cli::success`

```kiwi
cli::success(msg: string)
```

Prints a success message.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` |  | The message to display. |

### `cli::header`

```kiwi
cli::header(title: string)
```

Prints a section header.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `title` | `string` |  | The header text. |

### `cli::hr`

```kiwi
cli::hr(width: integer = 40)
```

Prints a horizontal rule.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `width` | `integer` | `40` | The width (default: 40). |

### `cli::kv`

```kiwi
cli::kv(key: string, val, width: integer = 16)
```

Prints a key-value pair with alignment.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `key` | `string` |  | The label. |
| `val` | `any` |  | The value. |
| `width` | `integer` | `16` | The column width for the key (default: 16). |

### `cli::table`

```kiwi
cli::table(headers: list, rows: list)
```

Prints a formatted table.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `headers` | `list` |  | Column header strings. |
| `rows` | `list` |  | List of row lists. |

### `cli::progress_bar`

```kiwi
cli::progress_bar(current: integer, total: integer, width: integer = 30): string
```

Returns a progress bar string.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `current` | `integer` |  | Current progress value. |
| `total` | `integer` |  | Total value. |
| `width` | `integer` | `30` | Bar width in characters (default: 30). |

**Returns** `string` — The progress bar string.

### `cli::prompt`

```kiwi
cli::prompt(msg: string = ""): string
```

Prompts the user for input.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `""` | The prompt message. |

**Returns** `string` — The user's input.

### `cli::confirm`

```kiwi
cli::confirm(msg: string = ""): boolean
```

Prompts the user for a yes/no confirmation.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `msg` | `string` | `""` | The prompt message. |

**Returns** `boolean` — `true` if the user answered yes.

## `CliParser` struct

A struct for defining and parsing command-line arguments.

### `CliParser.new`

```kiwi
CliParser.new(name: string, desc: string = "")
```

Creates a new CliParser.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | The program name. |
| `desc` | `string` | `""` | A short description (optional). |

### `CliParser.is_flag_token`

```kiwi
static CliParser.is_flag_token(tok: string): boolean
```

Whether a token is a flag (`-x`, `--name`, `--`) rather than a value.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `tok` | `string` |  | The token. |

**Returns** `boolean` — `false` for values, including `-` and negative numbers.

### `.flag`

```kiwi
flag(name: string, short: string = "", desc: string = "")
```

Registers a boolean flag.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | The long flag name (e.g., `"verbose"`). |
| `short` | `string` | `""` | The short flag character (e.g., `"v"`). |
| `desc` | `string` | `""` | A description of the flag. |

### `.option`

```kiwi
option(name: string, short: string = "", desc: string = "", dval = null)
```

Registers an option that takes a value.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | The long option name (e.g., `"output"`). |
| `short` | `string` | `""` | The short option character (e.g., `"o"`). |
| `desc` | `string` | `""` | A description of the option. |
| `dval` | `any` | `null` | The default value (default: `null`). |

### `.help`

```kiwi
help()
```

Prints the help text for this parser.

### `.parse`

```kiwi
parse(args = null): hashmap
```

Parses command-line arguments.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `args` | `list` | `null` | The tokens to parse, in order (default: `env::args()`, the raw command line). |

**Returns** `hashmap` — A hashmap of parsed values, plus `_args` for positional arguments.
