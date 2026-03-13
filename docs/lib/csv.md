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
