# `@kiwi/sys`

The `sys` module contains functionality for working with external processes.

## Importing the Module

To use the `sys` module, import it at the beginning of your Kiwi script.

```ruby
import "@kiwi/sys"
```

## Module Functions

### `euid()`
- **Summary**: Get the effective user ID.
- **Returns**: Integer containing effective user ID.

### `exec(@_command)`
- **Summary**: Run an external process and capture its exit code.
- **Parameters**:
  - `@_command`: String containing the command text.
- **Returns**: Integer containing exit code of external process.

### `execout(@_command)`
- **Summary**: Run an external process and capture its standard output.
- **Parameters**:
  - `@_command`: String containing the command text.
- **Returns**: String containing standard output from external process.