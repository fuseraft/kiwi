# `@astral/fs`

The `fs` module contains functionality for working with files and directories.

# Table of Contents

- [Importing the Module](#importing-the-module)
- [Module Functions](#module-functions)
  - [`abspath(_path)`](#abspath_path)
  - [`append(_path, _text)`](#append_path-_text)
  - [`chdir(_path)`](#chdir_path)
  - [`combine(_first, _second)`](#combine_first-_second)
  - [`copy(_source, _dest)`](#copy_source-_dest)
  - [`copyr(_source, _dest)`](#copyr_source-_dest)
  - [`cwd()`](#cwd)
  - [`exists(_path)`](#exists_path)
  - [`ext(_path)`](#ext_path)
  - [`filename(_path)`](#filename_path)
  - [`filesize(_path)`](#filesize_path)
  - [`glob(_path)`](#glob_path)
  - [`isdir(_path)`](#isdir_path)
  - [`listdir(_path)`](#listdir_path)
  - [`mkdir(_path)`](#mkdir_path)
  - [`mkdirp(_path)`](#mkdirp_path)
  - [`move(_source, _dest)`](#move_source-_dest)
  - [`parentdir(_path)`](#parentdir_path)
  - [`read(_path)`](#read_path)
  - [`readlines(_path)`](#readlines_path)
  - [`readbytes(_path, _offset, _size)`](#readbytes_path-_offset-_size)
  - [`remove(_path)`](#remove_path)
  - [`rmdir(_path)`](#rmdir_path)
  - [`rmdirf(_path)`](#rmdirf_path)
  - [`touch(_path)`](#touch_path)
  - [`tmpdir()`](#tmpdir)
  - [`write(_path, _text)`](#write_path-_text)
  - [`writeln(_path, _text)`](#writeln_path-_text)


## Importing the Module

To use the `fs` module, import it at the beginning of your Astral script.

```ruby
import "@astral/fs" as fs
```

## Module Functions

### `abspath(_path)`

- **Summary**: Get the absolute path of a file.
- **Parameters**:
  - `_path`: The path to a file or a filename.
- **Returns**: String representing the absolute path.

### `append(_path, _text)`

- **Summary**: Append text to a file.
- **Parameters**:
  - `_path`: The path to a file or a filename.
  - `_text`: The text to append.
- **Returns**: Boolean indicating success or failure.

### `chdir(_path)`

- **Summary**: Changes the current directory.
- **Parameters**:
  - `_path`: The path to a directory.
- **Returns**: Boolean indicating success or failure.

### `combine(_first, _second)`
- **Summary**: Combine two paths.
- **Parameters**:
  - `_source`: The first path.
  - `_dest`: The second path.
- **Returns**: String containing combined path.

### `copy(_source, _dest)`

- **Summary**: Copy a file.
- **Parameters**:
  - `_source`: The source path.
  - `_dest`: The destination path.
- **Returns**: Boolean indicating success or failure.

### `copyr(_source, _dest)`

- **Summary**: Copy a directory and all of its contents.
- **Parameters**:
  - `_source`: The source path.
  - `_dest`: The destination path.
- **Returns**: Boolean indicating success or failure.

### `cwd()`

- **Summary**: Gets the path of the current working directory.
- **Returns**: String containing the path of the current working directory.

### `exists(_path)`

- **Summary**: Check if a path exists.
- **Parameters**:
  - `_path`: The path to a file or directory.
- **Returns**: Boolean indicating whether the path exists or not.

### `ext(_path)`

- **Summary**: Get a file extension.
- **Parameters**:
  - `_path`: The path to a file or a filename.
- **Returns**: String representing the file extension.

### `filename(_path)`

- **Summary**: Get the filename from a file path.
- **Parameters**:
  - `_path`: The path to a file or a filename.
- **Returns**: String representing the filename.

### `filesize(_path)`

- **Summary**: Get size of a file in bytes.
- **Parameters**:
  - `_path`: The path to a file or a filename.
- **Returns**: Double representing the file size in bytes.

### `glob(_path)`

- **Summary**: Get a list of files from a glob pattern.
- **Parameters**:
  - `_glob`: The glob pattern.
- **Returns**: List of file paths matched by glob pattern.

### `isdir(_path)`
- **Summary**: Check if a path is a directory.
- **Parameters**:
  - `_path`: The path to a directory.
- **Returns**: Boolean indicating whether the path is a directory.

### `listdir(_path)`

- **Summary**: Retrieve a List of directory entries.
- **Parameters**:
  - `_path`: The directory path.
- **Returns**: List of directory entries.

### `mkdir(_path)`

- **Summary**: Creates a directory.
- **Parameters**:
  - `_path`: The path to a directory.
- **Returns**: Boolean indicating success or failure.

### `mkdirp(_path)`

- **Summary**: Creates a directory and all subdirectories specified in the path.
- **Parameters**:
  - `_path`: The path to a directory.
- **Returns**: Boolean indicating success or failure.

### `move(_source, _dest)`

- **Summary**: Move or rename a file.
- **Parameters**:
  - `_source`: The source path.
  - `_dest`: The destination path.
- **Returns**: Boolean indicating success or failure.

### `parentdir(_path)`

- **Summary**: Get the parent directory of an absolute path.
- **Parameters**:
  - `_path`: The path to a file.
- **Returns**: String representing the parent directory.

### `read(_path)`

- **Summary**: Get the content of a file as a string.
- **Parameters**:
  - `_path`: The path to a file.
- **Returns**: String containing the file content.

### `readlines(_path)`

- **Summary**: Get the content of a file as a list of strings.
- **Parameters**:
  - `_path`: The path to a file.
- **Returns**: List containing the file lines.

### `readbytes(_path, _offset, _size)`

- **Summary**: Get the content of a file as a list of strings.
- **Parameters**:
  - `_path`: The path to a file.
  - `_offset`: The position in the file to read from.
  - `_size`: The number of bytes to read from the file.
- **Returns**: List containing bytes from a file.

### `remove(_path)`

- **Summary**: Delete a file.
- **Parameters**:
  - `_path`: The path to a file or a filename.
- **Returns**: Boolean indicating success or failure.

### `rmdir(_path)`

- **Summary**: Delete a directory.
- **Parameters**:
  - `_path`: The path to a directory.
- **Returns**: Boolean indicating success or failure.

### `rmdirf(_path)`

- **Summary**: Delete a directory and all of its contents recursively.
- **Parameters**:
  - `_path`: The path to a directory.
- **Returns**: Integer indicating how many objects were deleted.

### `touch(_path)`

- **Summary**: Create an empty file.
- **Parameters**:
  - `_path`: The path to a file or a filename.
- **Returns**: Boolean indicating success or failure.

### `tmpdir()`

- **Summary**: Gets the path to the temporary directory.
- **Returns**: String containing the path to the temporary directory.

### `write(_path, _text)`

- **Summary**: Write text to a file. This overwrites the file if it exists.
- **Parameters**:
  - `_path`: The path to a file or a filename.
  - `_text`: The text to write.
- **Returns**: Boolean indicating success or failure.

### `writeln(_path, _text)`

- **Summary**: Write a line of text to a file. This always appends to a file.
- **Parameters**:
  - `_path`: The path to a file or a filename.
  - `_text`: The text to append.
- **Returns**: Boolean indicating success or failure.