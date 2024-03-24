# `@astral/fs`

The `fs` module contains functionality for working with files and directories.

## Table of Contents

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

Get the absolute path of a file.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_path` | The relative path to a file or a filename. |

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The absolute path. |

### `append(_path, _text)`

Append text to a file.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_path` | The relative path to a file or a filename. |
| `String` | `_text` | The text to append. |

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | Indicates success or failure. |

### `chdir(_path)`

Changes the current directory.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_path` | The path to a directory. |

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | Indicates success or failure. |

### `combine(_first, _second)`

Combine two path strings.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_source` | The source path. |
| `String` | `_dest` | The destination path. |

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The combined path. |

### `copy(_source, _dest)`

Copy a file.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_source` | The source path. |
| `String` | `_dest` | The destination path. |

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | Indicates success or failure. |

### `copyr(_source, _dest)`

Copy a directory recursively.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_source` | The source path. |
| `String` | `_dest` | The destination path. |

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | Indicates success or failure. |

### `cwd()`

Gets the path of the current working directory.

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The path of the current working directory. |

### `exists(_path)`

Check if a path exists.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_path` | The path to check. |

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | Indicates whether the file exists. |

### `ext(_path)`

Get a file extension.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_path` | The path to check. |

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The file extension. |

### `filename(_path)`

Get the filename from a path.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_path` | The path to a file. |

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The filename. |

### `filesize(_path)`

Get size of a file in bytes.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_path` | The path to a file. |

**Returns**
| Type | Description |
| :--- | :---|
| `Double` | The file size in number of bytes. |

### `glob(_path)`

Get a list of files from a glob pattern.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_glob` | The glob pattern. |

**Returns**
| Type | Description |
| :--- | :---|
| `List` | File paths matched by glob pattern. |

### `isdir(_path)`

Check if a path exists and is a directory.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_path` | The path to check. |

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | Indicates whether the path exists and is a directory. |

### `listdir(_path)`

Retrieve a list of directory entries.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_path` | The directory path. |

**Returns**
| Type | Description |
| :--- | :---|
| `List` | Directory entries. |

### `mkdir(_path)`

Create a directory.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_path` | The directory path. |

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | Indicates success or failure. |

### `mkdirp(_path)`

Creates a directory and all subdirectories specified in the path.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_path` | The directory path. |

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | Indicates success or failure. |

### `move(_source, _dest)`

Move or rename a file.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_source` | The source path. |
| `String` | `_dest` | The destination path. |

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | Indicates success or failure. |

### `parentdir(_path)`

Get the parent directory of an absolute path.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_path` | The path to a file. |

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The parent directory. |

### `read(_path)`

Get the content of a file as a string.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_path` | The path to a file. |

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The file content. |

### `readlines(_path)`

Get the content of a file as a list of strings.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_path` | The path to a file. |

**Returns**
| Type | Description |
| :--- | :---|
| `List` | The file lines. |

### `readbytes(_path, _offset, _size)`

Get the content of a file as a list of bytes.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_path` | The path to a file. |
| `Integer` | `_offset` | The position in the file to read from.
| `Integer` | `_size` | The number of bytes to read from the file.

**Returns**
| Type | Description |
| :--- | :---|
| `List` | Bytes from a file. |

### `remove(_path)`

Delete a file.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_path` | The file path. |

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | Indicates success or failure. |

### `rmdir(_path)`

Delete a directory.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_path` | The directory path. |

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | Indicates success or failure. |

### `rmdirf(_path)`

Delete a directory and all of its contents recursively.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_path` | The directory path. |

**Returns**
| Type | Description |
| :--- | :---|
| `Integer` | Number of objects deleted. |

### `touch(_path)`

Create an empty file.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_path` | The file path. |

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | Indicates success or failure. |

### `tmpdir()`

Gets the path to the temporary directory.

**Returns**
| Type | Description |
| :--- | :---|
| `String` | The temporary directory path. |

### `write(_path, _text)`

Write text to a file. This overwrites the file if it exists.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_path` | The file path. |
| `String` | `_text` | The text to write. |

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | Indicates success or failure. |

### `writeln(_path, _text)`

Write a line of text to a file. This always appends to a file.

**Parameters**
| Type | Name | Description |
| :--- | :--- | :--- |
| `String` | `_path` | The file path. |
| `String` | `_text` | The text to append. |

**Returns**
| Type | Description |
| :--- | :---|
| `Boolean` | Indicates success or failure. |