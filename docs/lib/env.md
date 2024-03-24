# `@astral/env`

The `ENV` module contains functionality for working with environment variables.

## Table of Contents

- [Importing the Module](#importing-the-module)
- [Module Functions](#module-functions)
  - [`get(_varname)`](#get_varname)
  - [`set(_varname, _varvalue)`](#set_varname-_varvalue)

## Importing the Module

To use the `env` module, import it at the beginning of your Astral script.

```ruby
import "@astral/env" as ENV
```

## Module Functions

### `get(_varname)`

Get an environment variable.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_varname` | The environment variable name to retrieve. |

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The environment variable value, an empty string if not found. |

### `set(_varname, _varvalue)`

Set an environment variable.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_varname` | The environment variable name to set. |
| `String` | `_varvalue` | The environment variable value. |

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | `true` on success. |
