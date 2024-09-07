# `env`

The `env` package contains functionality for working with environment variables.

## Table of Contents

- [Package Functions](#package-functions)
  - [`get(_varname)`](#get_varname)
  - [`getvars()`](#getvars)
  - [`set(_varname, _varvalue)`](#set_varname-_varvalue)
  - [`kiwi()`](#kiwi)
  - [`kiwilib()`](#kiwilib)
  - [`unset(_varname)](#unset_varname)

## Package Functions

### `getvars()`

Returns a hash where each key-value pair is an environment variable and value.

**Returns**
| Type | Description |
| :--- | :---|
| `Hash` | A hash representing system environment variables. |

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

### `unset(_varname)`

Unset an environment variable.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_varname` | The environment variable name to 

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | `true` on success. |

### `kiwi()`

Returns a string containing the path to the Kiwi executable.

**Returns**
| Type | Description |
| :--- | :---|
| `String` | Path to Kiwi. |

### `kiwilib()`

Returns a string containing the path to the Kiwi Standard Library.

**Returns**
| Type | Description |
| :--- | :---|
| `String` | Path to Kiwi Standard Library. |