---
title: cli
description: "Building command-line tools: labeled messages, tables, progress bars, prompts and argument parsing."
---

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

```kiwi nocheck
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

```kiwi nocheck
parser.flag("help", "h", "Show this help")
args = parser.parse()

if args["help"]
  parser.help()
  exit 0
end
```

For raw access to arguments, see [`env::args()` and `env::argv()`](/kiwi/tools/cli/#passing-arguments-to-scripts).

<!-- api: cli -->
