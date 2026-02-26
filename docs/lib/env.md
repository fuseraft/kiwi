# `env`

The `env` package contains functionality for working with environment variables.

## Table of Contents

- [Package Functions](#package-functions)
  - [`all()`](#all)
  - [`argv()`](#argv)
  - [`opt(_key)`](#opt_key)
  - [`parse_args(args, value_opts)`](#parse_argsargs-value_opts)
  - [`get(_varname)`](#get_varname)
  - [`set(_varname, _varvalue)`](#set_varname-_varvalue)
  - [`bin()`](#bin)
  - [`lib()`](#lib)

## Package Functions

### `all()`

Get environment variables as a hashmap.

**Returns**
| Type | Description |
| :--- | :---|
| `hashmap` | A hashmap representing system environment variables. |

### `argv()`

Get the list of command-line arguments supplied to the program.

**Returns**
| Type | Description |
| :--- | :---|
| `list` | A list of command-line arguments. |

### `opt(_key)`
Get a KVP command-line option value by key.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_key` | The option key.|

**Returns**
| Type | Description |
| :--- | :---|
| `string` | The option value. |

## KVP Command-Line Options

You can pass a named command-line argument in the form of a key-value pair.

```bash
kiwi -key=value
kiwi --key=value
kiwi /key=value
```

You can pull these values using this package.

```kiwi
println(env::opt("key")) # prints: value
```

### `parse_args(args, value_opts)`

Parses a list of command-line argument strings into a hashmap of key-value pairs. Arguments listed in `value_opts` consume the next argument as their value; all other arguments are mapped to `true`.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `list` | `args` | A list of argument strings. Defaults to `env::argv().keys()` if empty. |
| `list` | `value_opts` | A list of option names that expect a following value argument. |

**Returns**
| Type | Description |
| :--- | :---|
| `hashmap` | A hashmap of parsed options. |

**Example**
```kiwi
# kiwi myscript.kiwi --output result.txt --verbose
cfg = env::parse_args([], ["--output"])
println cfg["--output"]   # prints: result.txt
println cfg["--verbose"]  # prints: true
```

**Throws**
If a value option is listed in `value_opts` but no following argument exists.

---

### `get(_varname)`

Get an environment variable.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_varname` | The environment variable name to retrieve. |

**Returns**
| Type | Description |
| :--- | :---|
| `string` | The environment variable value, an empty string if not found. |

### `set(_varname, _varvalue)`

Set an environment variable.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `string` | `_varname` | The environment variable name to set. |
| `string` | `_varvalue` | The environment variable value. |

**Returns**
| Type | Description |
| :--- | :---|
| `boolean` | `true` on success. |

### `bin()`

Returns a string containing the path to the Kiwi executable.

**Returns**
| Type | Description |
| :--- | :---|
| `string` | Path to Kiwi. |

### `lib()`

Returns a string containing the path to the Kiwi Standard Library.

**Returns**
| Type | Description |
| :--- | :---|
| `string` | Path to Kiwi Standard Library. |