# `@kiwi/env`

The `ENV` package contains functionality for working with environment variables.

## Table of Contents

- [Importing the Package](#importing-the-package)
- [Package Functions](#package-functions)
  - [`get(_varname)`](#get_varname)
  - [`set(_varname, _varvalue)`](#set_varname-_varvalue)

## Importing the Package

To use the `env` package, import it at the beginning of your Kiwi script.

```ruby
import "@kiwi/env" as ENV
```

## Package Functions

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
