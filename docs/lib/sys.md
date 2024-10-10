# `sys`

The `sys` package contains functionality for working with external processes.

## Table of Contents

- [Package Functions](#package-functions)
  - [`euid()`](#euid)
  - [`exec(_command)`](#exec_command)
  - [`execout(_command)`](#execout_command)

## Package Functions

### `euid()`

Get the effective user ID.

**Returns**
| Type | Description |
| :--- | :---|
| `Integer` | The effective user ID. |


### `exec(_command)`

Run an external process and capture its exit code.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_command` | The command text. |

**Returns**
| Type | Description |
| :--- | :---|
| `Integer` | The exit code of an external process. |

### `execout(_command)`

Run an external process and capture its standard output.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_command` | The command text. |

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The standard output of an external process. |
