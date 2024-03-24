# `@astral/conf`

The `conf` module contains functionality for working with configuration files.

Common file extensions are: `.conf`, `.ini`, `.cfg`, `.env`, and `.json`

*Note: this module is likely to evolve over time.* 

# Table of Contents

- [Importing the Module](#importing-the-module)
- [Module Functions](#module-functions)
  - [`read(_conf_path, _delim = "=")`](#read_conf_path-_delim--)
  - [`readjson(_json_path)`](#readjson_json_path)

## Importing the Module

To use the `conf` module, import it at the beginning of your Astral script.

```ruby
import "@astral/conf" as conf
```

## Module Functions

### `read(_conf_path, _delim = "=")`

Reads settings from a `conf` file into a hash.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_conf_path` | The path to the file. |
| `String` | `_delim` | The delimiter for splitting key-value pairs. |

**Returns**
| Type | Description |
| :--- | :---|
| `Hash` | Settings read from a file. |

### `readjson(_json_path)`

Reads settings from a `json` file into a hash.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_json_path` | The path to a `json` file. |
| `String` | `_delim` | The delimiter for splitting key-value pairs. |

**Returns**
| Type | Description |
| :--- | :---|
| `Hash` | Settings read from a `json` file. |
