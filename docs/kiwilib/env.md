# `@kiwi/env`

The `env` module contains functionality for working with environment variables.

## Importing the Module

To use the `env` module, import it at the beginning of your Kiwi script.

```ruby
import "@kiwi/env"
```

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