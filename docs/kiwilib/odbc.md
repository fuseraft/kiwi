# `odbc`

The `odbc` contains functionality for connecting to databases with ODBC to execute queries.

## Importing the Module

To use the `odbc` module, import it at the beginning of your Kiwi script.

```ruby
import "@kiwi/odbc"
```

## Module Functions

### `begin_transaction(@_connection)`
- **Summary**: Begins a transaction.
- **Parameters**:
  - `@_connection`: Hash containing a `connection_string` property.
- **Returns**: Hash containing an ODBC response.

### `commit(@_connection)`
- **Summary**: Commit a transaction.
- **Parameters**:
  - `@_connection`: Hash containing a `connection_string` property.
- **Returns**: Hash containing an ODBC response.

### `connect(@_connection)`
- **Summary**: Connect to a database engine.
- **Parameters**:
  - `@_connection`: Hash containing a `connection_string` property.
- **Returns**: Hash containing an ODBC response.

### `exec_sql(@_connection, @_sql)`
- **Summary**: Connect to a database engine.
- **Parameters**:
  - `@_connection`: Hash containing a `connection_string` property.
  - `@_sql`: The SQL to execute.
- **Returns**: Hash containing an ODBC response.

### `exec_sp(@_connection, @_sp, @_params)`
- **Summary**: Connect to a database engine.
- **Parameters**:
  - `@_connection`: Hash containing a `connection_string` property.
  - `@_sp`: A string containing the SQL stored procedure name to execute.
  - `@_params`: A list of parameter values.
- **Returns**: Hash containing an ODBC response.

### `in_transaction(@_connection)`
- **Summary**: Check if in a transaction.
- **Parameters**:
  - `@_connection`: Hash containing a `connection_string` property.
- **Returns**: Hash containing an ODBC response.

### `is_connected(@_connection)`
- **Summary**: Check if a connection is active.
- **Parameters**:
  - `@_connection`: Hash containing a `connection_string` property.
- **Returns**: Hash containing an ODBC response.

### `rollback(@_connection)`
- **Summary**: Rollback a transaction.
- **Parameters**:
  - `@_connection`: Hash containing a `connection_string` property.
- **Returns**: Hash containing an ODBC response.