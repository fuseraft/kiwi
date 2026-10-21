# csv

> Parse CSV text and files into lists or hashmaps, with typed columns.

The `csv` package parses comma-separated values following RFC 4180, including quoted fields that contain commas or line breaks.

## Quick parsing

`parse` returns a list of rows, each a list of strings. `to_maps` uses the first row as headers and returns a list of hashmaps.

```kiwi
data = "name,age\nAda,36\n\"Lovelace, A\",37"

println csv::parse(data)
# [["name", "age"], ["Ada", "36"], ["Lovelace, A", "37"]]

for person in csv::to_maps(data) do
  println "${person["name"]} is ${person["age"]}"
end
```

`parse_file` and `file_to_maps` do the same for a file. All four take an optional delimiter:

```kiwi
println csv::parse("a;b\n1;2", ";")   # [["a", "b"], ["1", "2"]]
```

```kiwi
rows = csv::file_to_maps("people.csv")
tsv = csv::parse_file("report.tsv", "\t")
```

## Pipelines with typed columns

`csv::pipeline()` builds a parse step by step. Every row comes back as a hashmap, and a schema converts columns from strings to the types you need.

```kiwi
data = "name,score,passed\nAlice,95,true\nBob,,false"

rows = csv::pipeline()
  .with_schema(["string", "integer", "boolean"])
  .from_string(data)
  .parse()

for r in rows do
  println "${r["name"]}: ${r["score"] ?? "no score"}"
end
# Alice: 95
# Bob: no score
```

| Method | Effect |
| :-- | :-- |
| `.from_file(path)`, `.from_string(text)` | Add a source. |
| `.with_schema(types)` | Convert each column to a type, by position. |
| `.with_headers(names)` | Use these column names; the source has no header row. |
| `.with_delimiter(text)` | Use a delimiter other than `,`. |
| `.as_dataset(name)` | Name the most recent source. |
| `.parse()` | Run the pipeline. |

### Schema types

| Type | Conversion | Empty field |
| :-- | :-- | :-- |
| `"string"` | Unchanged | `""` |
| `"string?"` | Unchanged | `null` |
| `"integer"` or `"int"` | Parsed as an integer | `null` |
| `"float"`, `"double"` or `"number"` | Parsed as a float | `null` |
| `"boolean"` or `"bool"` | `true` for `"true"`, `"1"` or `"yes"`; otherwise `false` | `false` |
| `"date"` | Parsed as a date | `null` |

Use `"string?"` when an empty text field should count as missing, so that `??` and `!= null` work on it.

### Several sources

With several unnamed sources, `parse()` returns a list with one result per source. Name every source with `.as_dataset()` to get a hashmap instead; mixing named and unnamed sources is an error.

```kiwi
data = csv::pipeline()
  .with_schema(["string", "integer"])
  .from_string("product,qty\nkiwi,3").as_dataset("orders")
  .with_schema(["string", "float"])
  .from_string("product,price\nkiwi,0.5").as_dataset("prices")
  .parse()

println data["orders"]   # [{"product": "kiwi", "qty": 3}]
println data["prices"]   # [{"product": "kiwi", "price": 0.5}]
```

For filtering, joining and grouping parsed rows, see [`table`](table.md).

## Functions

### `csv::parse_file`

```kiwi
csv::parse_file(file_path: string, delimiter: string = ","): list
```

Parses a CSV file into a list of rows.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `file_path` | `string` |  | The path to the CSV file. |
| `delimiter` | `string` | `","` | The field delimiter (default `","`). |

**Returns** `list` — A list of rows, each row being a list of string fields.

**Throws** `string` — If the file does not exist.

### `csv::parse`

```kiwi
csv::parse(input: string, delimiter: string = ","): list
```

Parses a CSV string into a list of rows.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `input` | `string` |  | The CSV-formatted string to parse. |
| `delimiter` | `string` | `","` | The field delimiter (default `","`). |

**Returns** `list` — A list of rows, each row being a list of string fields.

### `csv::to_maps`

```kiwi
csv::to_maps(input: string, delimiter: string = ","): list
```

Parses a CSV string into a list of hashmaps using the first row as headers.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `input` | `string` |  | The CSV-formatted string to parse. |
| `delimiter` | `string` | `","` | The field delimiter (default `","`). |

**Returns** `list` — A list of hashmaps, each mapping header names to field values.

### `csv::file_to_maps`

```kiwi
csv::file_to_maps(file_path: string, delimiter: string = ","): list
```

Parses a CSV file into a list of hashmaps using the first row as headers.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `file_path` | `string` |  | The path to the CSV file. |
| `delimiter` | `string` | `","` | The field delimiter (default `","`). |

**Returns** `list` — A list of hashmaps, each mapping header names to field values.

**Throws** `string` — If the file does not exist.

### `csv::pipeline`

```kiwi
csv::pipeline(): CsvPipeline
```

Creates a new CsvPipeline builder for fluent CSV parsing.

**Returns** `CsvPipeline` — A new pipeline builder instance.

```kiwi
# Single file — returns a list of hashmaps
rows = csv::pipeline()
  .with_headers(["name", "id", "active", "created"])
  .with_schema(["string", "integer", "boolean", "date"])
  .from_file("users.csv")
  .parse()

# Multiple named datasets — returns a hashmap keyed by dataset name
data = csv::pipeline()
  .with_schema(["string", "integer"])
  .from_file("orders.csv").as_dataset("orders")
  .with_schema(["string", "float"])
  .from_file("products.csv").as_dataset("products")
  .parse()

orders   = data["orders"]
products = data["products"]
```

## `CsvPipeline` struct

A fluent builder for parsing one or more CSV files into structured data.

### `CsvPipeline.new`

```kiwi
CsvPipeline.new()
```

Initialises an empty pipeline builder.

### `.with_headers`

```kiwi
with_headers(headers: list): CsvPipeline
```

Sets custom column headers for the next `from_file()` call.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `headers` | `list` |  | List of column header name strings. |

**Returns** `CsvPipeline` — self, for chaining.

### `.with_schema`

```kiwi
with_schema(schema: list): CsvPipeline
```

Sets a type-coercion schema for the next `from_file()` call.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `schema` | `list` |  | List of type-name strings. |

**Returns** `CsvPipeline` — self, for chaining.

### `.with_delimiter`

```kiwi
with_delimiter(delimiter: string): CsvPipeline
```

Sets the field delimiter for the next `from_file()` call.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `delimiter` | `string` |  | The delimiter character (default `","`). |

**Returns** `CsvPipeline` — self, for chaining.

### `.from_file`

```kiwi
from_file(path: string): CsvPipeline
```

Registers a CSV file as a source using the current pending settings.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path` | `string` |  | Path to the CSV file. |

**Returns** `CsvPipeline` — self, for chaining.

**Throws** `string` — If the file does not exist at parse time.

### `.from_string`

```kiwi
from_string(data: string): CsvPipeline
```

Registers an in-memory CSV string as a source using the current pending settings.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `data` | `string` |  | The CSV-formatted string to parse. |

**Returns** `CsvPipeline` — self, for chaining.

### `.as_dataset`

```kiwi
as_dataset(name: string): CsvPipeline
```

Names the most recently registered file as a dataset.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `name` | `string` |  | The dataset key name. |

**Returns** `CsvPipeline` — self, for chaining.

**Throws** `string` — If called before any `from_file()`.

### `.parse`

```kiwi
parse()
```

Executes the pipeline and returns the parsed results.

**Returns** `list|hashmap` — Parsed CSV data.

**Throws** `string` — On file-not-found or mixed dataset/non-dataset sources.
