# text

> Line-oriented text processing pipelines.

The `text` package processes text line by line, like a small `grep`/`awk` pipeline. Add a source, chain steps, and call `run()` to get the resulting lines.

```kiwi
log = "time level message
10:01 INFO started
10:02 ERROR disk full

10:03 WARN slow request
10:04 ERROR timeout"

errors = text::pipeline()
  .from_string(log)
  .skip(1)                                       # drop the header
  .filter(do (line) => line.contains("ERROR") end)
  .run()

println errors   # ["10:02 ERROR disk full", "10:04 ERROR timeout"]

levels = text::pipeline()
  .from_string(log)
  .skip(1)
  .reject(do (line) => line.trim().empty() end)  # drop blank lines
  .split_on(" ")                                 # each line becomes a list of fields
  .map(do (fields) => fields[1] end)
  .run()

println levels   # ["INFO", "ERROR", "WARN", "ERROR"]
```

| Method | Does |
| :-- | :-- |
| `from_file(path)`, `from_string(text)` | Sets the input. |
| `skip(n)` | Drops the first `n` lines. |
| `filter(lambda)`, `reject(lambda)` | Keeps, or drops, lines for which the lambda is truthy. |
| `match(pattern)` | Keeps lines that match a regular expression. |
| `split_on(delimiter)` | Turns each line into a list of fields. |
| `map(lambda)` | Transforms each line, or each list of fields. |
| `run()` | Runs the steps in order and returns the result list. |

For a log file:

```kiwi
slow = text::pipeline()
  .from_file("access.log")
  .match('\s\d{4,}ms$')
  .run()
```

For structured data, [`csv`](csv.md) and [`json`](json.md) are a better fit.

## Functions

### `text::pipeline`

```kiwi
text::pipeline(): TextPipeline
```

Creates a new TextPipeline builder for fluent line-oriented text processing.

**Returns** `TextPipeline` — A new pipeline builder instance.

## `TextStep` struct

Represents a single processing step in a TextPipeline.

### `TextStep.new`

```kiwi
TextStep.new(kind: string, action)
```

Creates a new step.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `kind` | `string` |  | The step kind ("skip", "filter", "reject", "map", "match", "split_on"). |
| `action` | `any` |  | A lambda for transform/filter steps; a value for skip/match/split_on. |

## `TextPipeline` struct

A fluent pipeline for processing text line by line.

```kiwi
# Read an error log, extract ERROR lines, split on space
rows = text::pipeline()
  .from_file("app.log")
  .skip(1)
  .match('ERROR')
  .split_on(" ")
  .run()

# Process an in-memory string
words = text::pipeline()
  .from_string("hello world\nfoo bar")
  .map(do (line) => line.uppercase() end)
  .run()
```

### `TextPipeline.new`

```kiwi
TextPipeline.new()
```

Initialises an empty pipeline builder.

### `.from_file`

```kiwi
from_file(path: string): TextPipeline
```

Sets a text file as the source (one line per element).

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path` | `string` |  | Path to the text file. |

**Returns** `TextPipeline` — self, for chaining.

**Throws** `string` — If the file does not exist at run time.

### `.from_string`

```kiwi
from_string(input: string): TextPipeline
```

Sets a string as the source, split on newlines.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `input` | `string` |  | A multi-line string. |

**Returns** `TextPipeline` — self, for chaining.

### `.skip`

```kiwi
skip(n: integer): TextPipeline
```

Drops the first `n` lines.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `n` | `integer` |  | Number of lines to skip. |

**Returns** `TextPipeline` — self, for chaining.

### `.filter`

```kiwi
filter(action: lambda): TextPipeline
```

Keeps only lines for which `action(line)` is truthy.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `action` | `lambda` |  | A lambda accepting a line string. |

**Returns** `TextPipeline` — self, for chaining.

### `.reject`

```kiwi
reject(action: lambda): TextPipeline
```

Drops lines for which `action(line)` is truthy.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `action` | `lambda` |  | A lambda accepting a line string. |

**Returns** `TextPipeline` — self, for chaining.

### `.map`

```kiwi
map(action: lambda): TextPipeline
```

Transforms each line by passing it through `action`.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `action` | `lambda` |  | A lambda accepting a line and returning a new value. |

**Returns** `TextPipeline` — self, for chaining.

### `.match`

```kiwi
match(pattern: string): TextPipeline
```

Keeps only lines that match the given regex pattern.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `pattern` | `string` |  | A regex pattern string (passed to `regex::test`). |

**Returns** `TextPipeline` — self, for chaining.

### `.split_on`

```kiwi
split_on(delimiter: string): TextPipeline
```

Splits each line into a list using `delimiter`.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `delimiter` | `string` |  | The delimiter string. |

**Returns** `TextPipeline` — self, for chaining.

### `.run`

```kiwi
run()
```

Executes the pipeline and returns the resulting list.

**Returns** `list` — Processed lines (strings, or lists if `split_on` was used).

**Throws** `string` — If no source was configured or the file is missing.
