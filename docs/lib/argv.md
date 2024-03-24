# `@astral/argv`

The `argv` module contains functionality for working with command-line arguments.

# Table of Contents

- [Importing the Module](#importing-the-module)
- [Module Functions](#module-functions)
  - [`get()`](#get)
  - [`opt(_key)`](#opt_key)

## Importing the Module

To use the `argv` module, import it at the beginning of your Astral script.

```ruby
import "@astral/argv" as ARGV
```

## Module Functions

### `get()`
Get the list of command-line arguments.
- **Returns**: List of command-line arguments.

| Return Type | Description |
| --- | --- |
| `List` | The command-line arguments. |

### `opt(_key)`
Get a KVP command-line option value by key.

| Parameter | Description |
| --- | --- |
| `_key` |The option key.|

| Return Type | Description |
| --- | --- |
| `String` | The option value. |

## KVP Command-Line Options

You can pass a named command-line argument in the form of a key-value pair.

```bash
/bin/astral -key=value
/bin/astral --key=value
/bin/astral /key=value
```

You can pull these values using this module.

```ruby
import "@astral/argv" as ARGV
println ARGV.opt("key") # prints: value
```