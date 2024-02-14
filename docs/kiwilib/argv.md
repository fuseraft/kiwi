# `argv`

The `argv` module contains functionality for working with command-line arguments.

## Importing the Module

To use the `argv` module, import it at the beginning of your Kiwi script.

```ruby
import "@kiwi/argv"
```

## Module Functions

### `get()`
- **Summary**: Get the list of command-line arguments.
- **Returns**: List of command-line arguments.

### `xarg(@_xarg_key)`
- **Summary**: Get an XArg value by name.
- **Parameters**:
  - `@_xarg_key`: The XArg key.
- **Returns**: String containing the XArg value.

## XArgs

An `XArg` is a named command-line argument.

### XArg Syntax

```bash
/bin/kiwi -Xkey=value
```