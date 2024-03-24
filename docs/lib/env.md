# `@astral/env`

The `ENV` module contains functionality for working with environment variables.

# Table of Contents

- [Importing the Module](#importing-the-module)
- [Module Functions](#module-functions)
  - [`get(_varname)`](#get_varname)
  - [`set(_varname, _varvalue)`](#set_varname-_varvalue)

## Importing the Module

To use the `env` module, import it at the beginning of your Astral script.

```ruby
import "@astral/env" as ENV
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