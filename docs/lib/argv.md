# `@kiwi/argv`

The `argv` package contains functionality for working with command-line arguments.

## Table of Contents

- [Package Functions](#package-functions)
  - [`get()`](#get)
  - [`opt(_key)`](#opt_key)

## Package Functions

### `get()`
Get the list of command-line arguments.

**Returns**
| Type | Description |
| :--- | :--- |
| `List` | The command-line arguments. |

### `opt(_key)`
Get a KVP command-line option value by key.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_key` | The option key.|

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The option value. |

## KVP Command-Line Options

You can pass a named command-line argument in the form of a key-value pair.

```bash
/bin/kiwi -key=value
/bin/kiwi --key=value
/bin/kiwi /key=value
```

You can pull these values using this package.

```kiwi
println(argv::opt("key")) # prints: value
```