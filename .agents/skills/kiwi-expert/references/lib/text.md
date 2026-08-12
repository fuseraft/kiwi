# `text`

The `text` package provides a fluent pipeline for processing text line by line. It is automatically loaded by the Kiwi runtime — no explicit import is needed.

---

## Package Functions

### `text::pipeline()`

Creates a `TextPipeline` builder. Set a source with `.from_file()` or `.from_string()`, chain processing steps, then call `.run()` to execute.

**Returns**

| Type | Description |
| :--- | :--- |
| `TextPipeline` | A new pipeline builder instance. |

---

## `TextPipeline`

A fluent builder returned by `text::pipeline()`. Processing steps are applied in the order they are added. Call `.run()` to execute all steps and return the result list.

All builder methods return `self` so calls can be chained.

### Source methods

#### `.from_file(path)`

Reads the source text from a file, split on newlines.

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `path` | Path to the text file. |

**Throws** — if the file does not exist at run time.

#### `.from_string(input)`

Uses a string as the source, split on newlines.

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `input` | A multi-line string. |

### Step methods

Steps are applied in the order they are registered.

#### `.skip(n)`

Drops the first `n` lines.

| Type | Name | Description |
| :--- | :--- | :--- |
| `integer` | `n` | Number of lines to skip. |

#### `.filter(action)`

Keeps only lines for which `action(line)` is truthy.

| Type | Name | Description |
| :--- | :--- | :--- |
| `lambda` | `action` | A lambda accepting a line string and returning a boolean. |

#### `.reject(action)`

Drops lines for which `action(line)` is truthy (the inverse of `.filter()`).

| Type | Name | Description |
| :--- | :--- | :--- |
| `lambda` | `action` | A lambda accepting a line string and returning a boolean. |

#### `.map(action)`

Transforms each line by passing it through `action`.

| Type | Name | Description |
| :--- | :--- | :--- |
| `lambda` | `action` | A lambda accepting a line and returning a new value. |

#### `.match(pattern)`

Keeps only lines that match the given regex pattern (uses `regex::test` internally).

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `pattern` | A regex pattern string. |

#### `.split_on(delimiter)`

Splits each line into a list using `delimiter`. After this step the pipeline contains lists, not strings — subsequent steps receive lists.

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `delimiter` | The delimiter string. |

### Terminal method

#### `.run()`

Executes all steps in order and returns the resulting list.

**Returns** `list` — strings, or lists if `.split_on()` was applied.

**Throws** — if no source was configured.

---

## Examples

### Filter and extract from a log file

```kiwi
errors = text::pipeline()
  .from_file("app.log")
  .skip(1)                                          # skip header line
  .filter(do (line) => line.contains("ERROR") end)
  .run()

for e in errors do
  println e
end
```

### Regex match

```kiwi
# Keep only lines containing an IP address
hits = text::pipeline()
  .from_file("access.log")
  .match('\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}')
  .run()
```

### Split into fields

```kiwi
# Parse a CSV-like log without the csv package
rows = text::pipeline()
  .from_file("data.log")
  .skip(1)
  .split_on(",")
  .run()

for row in rows do
  println "name=${row[0]}, value=${row[1]}"
end
```

### Chain: filter → split → map

```kiwi
messages = text::pipeline()
  .from_file("events.log")
  .match('ERROR|WARN')
  .split_on(" ")
  .map(do (fields) => fields[2] end)   # extract the message field
  .run()
```

### From string

```kiwi
words = text::pipeline()
  .from_string("hello world\nfoo bar\nbaz qux")
  .map(do (line) => line.uppercase() end)
  .run()

println words[0]  # HELLO WORLD
```

### Reject blank lines

```kiwi
lines = text::pipeline()
  .from_file("notes.txt")
  .reject(do (line) => line.trim().empty() end)
  .run()
```
