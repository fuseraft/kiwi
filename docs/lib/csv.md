# `csv`

The `csv` package provides functions for parsing CSV (comma-separated value) data.

The algorithm conforms to RFC 4180.

## Table of Contents

- [Package Functions](#package-functions)
  - [`parse(input, delimiter)`](#parseinput-delimiter)
  - [`parse_file(file_path, msg)`](#parse_filefile_path-delimiter)

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
