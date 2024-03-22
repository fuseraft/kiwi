# `@astral/argv`

The `ARGV` module contains functionality for working with command-line arguments.

## Module Functions

### `get()`
- **Summary**: Get the list of command-line arguments.
- **Returns**: List of command-line arguments.

### `xarg(_xarg_key)`
- **Summary**: Get an XArg value by name.
- **Parameters**:
  - `_xarg_key`: The XArg key.
- **Returns**: String containing the XArg value.

## XArgs

An `XArg` is a named command-line argument in the form of a key-value pair.

```bash
/bin/astral -Xkey=value
```

You can pull these values using this module.

```ruby
println ARGV.xarg("key") # prints: value
```