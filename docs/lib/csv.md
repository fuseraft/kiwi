# `csv`

The `csv` package provides functions for parsing CSV (comma-separated value) data.

The algorithm conforms to RFC 4180.

---

## Package Functions

### `parse(input, delimiter)`

Parses a CSV string into a list of rows (each row is a list of string fields).

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `input` | The CSV data. |
| `string` | `delimiter` | The delimiter to use. (optional, defaults to `,`). |

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | A list of lists representing rows and columns. |

**Example**

```kiwi
import "csv"

var data = "name,age\nAlice,30\nBob,25"
var rows = csv::parse(data)

# rows[0] is the header row
println rows[0]
# ["name", "age"]

# iterate over data rows, skipping the header
for row in rows[1:]
  println row[0] + " is " + row[1] + " years old"
end
# Alice is 30 years old
# Bob is 25 years old
```

---

### `parse_file(file_path, delimiter)`

Parses a CSV file into a list of rows.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `file_path` | The path to a file. |
| `string` | `delimiter` | The delimiter to use. (optional, defaults to `,`). |

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | A list of lists representing rows and columns. |

**Example**

```kiwi
import "csv"

# Parse a comma-delimited file
var rows = csv::parse_file("/data/employees.csv")

var headers = rows[0]
println "Columns: " + headers.join(", ")

for row in rows[1:]
  println row.join(" | ")
end

# Parse a tab-delimited file
var tsv_rows = csv::parse_file("/data/report.tsv", "\t")
println "Row count: " + (tsv_rows.size() - 1).to_string()
```

---

### `to_maps(input, delimiter)`

Parses a CSV string into a list of hashmaps, using the first row as header keys.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `input` | The CSV data. |
| `string` | `delimiter` | The delimiter to use. (optional, defaults to `,`). |

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | A list of hashmaps mapping header names to field values. |

**Example**

```kiwi
import "csv"

var data = "name,age\nAlice,30\nBob,25"
var records = csv::to_maps(data)

for r in records do
  println r["name"] + " is " + r["age"] + " years old"
end
# Alice is 30 years old
# Bob is 25 years old
```

---

### `file_to_maps(file_path, delimiter)`

Parses a CSV file into a list of hashmaps, using the first row as header keys.

**Parameters**

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `file_path` | The path to a file. |
| `string` | `delimiter` | The delimiter to use. (optional, defaults to `,`). |

**Returns**

| Type | Description |
| :--- | :--- |
| `list` | A list of hashmaps mapping header names to field values. |

**Throws**

`string` — if the file does not exist.

**Example**

```kiwi
import "csv"

var records = csv::file_to_maps("/data/employees.csv")

for r in records do
  println r["name"] + " earns " + r["salary"]
end
```

---

### `pipeline()`

Creates a `CsvPipeline` builder for fluent, declarative CSV parsing. Chain configuration methods to describe your sources, then call `.parse()` to execute.

**Returns**

| Type | Description |
| :--- | :--- |
| `CsvPipeline` | A new pipeline builder instance. |

See [CsvPipeline](#csvpipeline) below for the full builder API and examples.

---

## `CsvPipeline`

A fluent builder returned by `csv::pipeline()`. Configure one or more CSV sources, then call `.parse()` to get structured data.

All builder methods return `self` so calls can be chained.

### Schema types

Pass schema types as strings to `.with_schema()`:

| String | Coercion |
| :--- | :--- |
| `"string"` | left as-is |
| `"integer"` / `"int"` | `.to_integer()` |
| `"float"` / `"double"` / `"number"` | `.to_float()` |
| `"boolean"` / `"bool"` | `true` for `"true"`, `"1"`, `"yes"`; `false` otherwise |
| `"date"` | parsed via `time::parse()` |
| `"null"` / `"none"` | always `null` |
| anything else | left as-is |

### `.with_headers(headers)`

Supplies custom column headers. When set, **every row in the file is treated as a data row** (no header row is skipped).

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `headers` | List of column name strings. |

### `.with_schema(schema)`

Sets per-column type coercion for the next `from_file()`. Each element maps to the column at the same index; extra or missing entries are ignored.

| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `schema` | List of type-name strings. |

### `.with_delimiter(delimiter)`

Overrides the field delimiter for the next `from_file()` (default `,`).

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `delimiter` | The delimiter character. |

### `.from_file(path)`

Registers a CSV file using the current pending settings, then resets them for the next file. Call `.as_dataset()` after this to name the result.

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `path` | Path to the CSV file. |

### `.as_dataset(name)`

Names the most recently registered file. The name becomes the key in the hashmap returned by `.parse()`. Must be called after `.from_file()`.

| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `name` | Dataset key name. |

**Throws** — if called before any `.from_file()`.

### `.parse()`

Executes the pipeline and returns the parsed results.

| Condition | Return type |
| :--- | :--- |
| Single unnamed file | `list` of hashmaps |
| Multiple unnamed files | `list` of lists of hashmaps |
| All files named as datasets | `hashmap` keyed by dataset name |
| Mixed named/unnamed | throws a pipeline error |

---

## Pipeline Examples

### Single file with schema

```kiwi
import "csv"

rows = csv::pipeline()
  .with_schema(["string", "integer", "boolean", "float"])
  .from_file("users.csv")
  .parse()

for r in rows do
  name   = r["name"]
  id     = r["id"]      # integer
  active = r["active"]  # boolean
  score  = r["score"]   # float
  println "${name} (${id}): active=${active}, score=${score}"
end
```

### Custom headers (no header row in file)

```kiwi
import "csv"

rows = csv::pipeline()
  .with_headers(["name", "id", "active", "score"])
  .with_schema(["string", "integer", "boolean", "float"])
  .from_file("users_noheader.csv")
  .parse()
```

### Tab-delimited file

```kiwi
import "csv"

rows = csv::pipeline()
  .with_delimiter("\t")
  .from_file("report.tsv")
  .parse()
```

### Multiple unnamed files

When no datasets are named, multiple files return a list of results — one per file.

```kiwi
import "csv"

all = csv::pipeline()
  .from_file("q1.csv")
  .from_file("q2.csv")
  .from_file("q3.csv")
  .parse()

q1 = all[0]
q2 = all[1]
q3 = all[2]
```

### Named datasets

Name every source with `.as_dataset()` and `.parse()` returns a single hashmap keyed by those names.

```kiwi
import "csv"

data = csv::pipeline()
  .with_schema(["string", "integer", "float"])
  .from_file("orders.csv").as_dataset("orders")
  .with_schema(["string", "float"])
  .from_file("products.csv").as_dataset("products")
  .parse()

orders   = data["orders"]
products = data["products"]

for o in orders do
  println "Order: ${o["product"]}, qty=${o["qty"]}"
end
```

### Error: mixing named and unnamed sources

If at least one source is named, all must be named — otherwise `.parse()` throws.

```kiwi
import "csv"

# This will throw a pipeline error
csv::pipeline()
  .from_file("a.csv").as_dataset("a")
  .from_file("b.csv")   # missing as_dataset()
  .parse()
```
