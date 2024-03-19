# `@kiwi/conf`

The `conf` module contains functionality for working with configuration files.

Common file extensions are: `.conf`, `.ini`, `.cfg`, `.env`, and `.json`

*Note: this module is likely to evolve over time.* 

## Module Functions

### `read(_conf_path, _delim = "=")`
- **Summary**: Reads settings from a `conf` file into a hash.
- **Parameters**:
  - `_conf_path`: The path to a `conf` file.
  - `_delim`: The delimiter for splitting key-value pairs.
- **Returns**: Hash containing settings read from a `conf` file. 

### `readjson(_json_path)`
- **Summary**: Reads settings from a `json` file into a hash.
- **Parameters**:
  - `_json_path`: The path to a `json` file.
- **Returns**: Hash containing settings read from a `json` file. 
