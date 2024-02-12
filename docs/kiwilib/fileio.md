# `fileio`

The `fileio` module contains functionality for working with files and directories.

## Importing the Module

To use the `fileio` module, import it at the beginning of your Kiwi script.

```ruby
import "@kiwi/fileio"
```

## Module Functions

### `abspath(@_path)`

- **Summary**: Get the absolute path of a file.
- **Parameters**:
  - `@_path`: The path to a file or a filename.
- **Returns**: String representing the absolute path.

### `append(@_path, @_text)`

- **Summary**: Append text to a file.
- **Parameters**:
  - `@_path`: The path to a file or a filename.
  - `@_text`: The text to append.
- **Returns**: Boolean indicating success or failure.

### `copy(@_source, @_dest)`

- **Summary**: Copy a file.
- **Parameters**:
  - `@_source`: The source path.
  - `@_dest`: The destination path.
- **Returns**: Boolean indicating success or failure.

### `create(@_path)`

- **Summary**: Create an empty file.
- **Parameters**:
  - `@_path`: The path to a file or a filename.
- **Returns**: Boolean indicating success or failure.

### `cwd()`

- **Summary**: Gets the path of the current working directory.
- **Returns**: String containing the path of the current working directory.

### `exists(@_path)`

- **Summary**: Check if a file exists.
- **Parameters**:
  - `@_path`: The path to a file or a filename.
- **Returns**: Boolean indicating whether the file exists or not.

### `getext(@_path)`

- **Summary**: Get a file extension.
- **Parameters**:
  - `@_path`: The path to a file or a filename.
- **Returns**: String representing the file extension.

### `glob(@_path)`

- **Summary**: Get a list of files from a glob pattern.
- **Parameters**:
  - `@_glob`: The glob pattern.
- **Returns**: List of file paths matched by glob pattern.

### `filename(@_path)`

- **Summary**: Get the filename from a file path.
- **Parameters**:
  - `@_path`: The path to a file or a filename.
- **Returns**: String representing the filename.

### `filesize(@_path)`

- **Summary**: Get size of a file in bytes.
- **Parameters**:
  - `@_path`: The path to a file or a filename.
- **Returns**: Double representing the file size in bytes.

### `move(@_source, @_dest)`

- **Summary**: Move or rename a file.
- **Parameters**:
  - `@_source`: The source path.
  - `@_dest`: The destination path.
- **Returns**: Boolean indicating success or failure.

### `parentdir(@_path)`

- **Summary**: Get the parent directory of an absolute path.
- **Parameters**:
  - `@_path`: The path to a file.
- **Returns**: String representing the parent directory.

### `read(@_path)`

- **Summary**: Get the content of a file as a string.
- **Parameters**:
  - `@_path`: The path to a file.
- **Returns**: String containing the file content.

### `readlines(@_path)`

- **Summary**: Get the content of a file as a list of strings.
- **Parameters**:
  - `@_path`: The path to a file.
- **Returns**: List containing the file content.

### `remove(@_path)`

- **Summary**: Delete a file.
- **Parameters**:
  - `@_path`: The path to a file or a filename.
- **Returns**: Boolean indicating success or failure.

### `write(@_path, @_text)`

- **Summary**: Write text to a file. This overwrites the file if it exists.
- **Parameters**:
  - `@_path`: The path to a file or a filename.
  - `@_text`: The text to write.
- **Returns**: Boolean indicating success or failure.

### `writeln(@_path, @_text)`

- **Summary**: Write a line of text to a file. This always appends to a file.
- **Parameters**:
  - `@_path`: The path to a file or a filename.
  - `@_text`: The text to append.
- **Returns**: Boolean indicating success or failure.