# `@kiwi/env`

The `ENV` module contains functionality for working with environment variables.

## Module Functions

### `get(_varname)`
- **Summary**: Get an environment variable.
- **Parameters**:
  - `_varname`: String containing the environment variable name to retrieve.
- **Returns**: String containing the environment variable value, an empty string if not found.

### `set(_varname, _varvalue)`
- **Summary**: Set an environment variable.
- **Parameters**:
  - `_varname`: String containing the environment variable name.
  - `_varvalue`: String containing the environment variable value.
- **Returns**: Boolean indicating success or failure.