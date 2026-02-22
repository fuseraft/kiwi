# `path`

The `path` package provides a clean, object-oriented way to work with filesystem paths in Kiwi, inspired by Python's `pathlib`.  
It defines a single main struct `Path` that wraps a path string and offers convenient methods for path manipulation, filesystem queries, and file/directory operations.

## Table of Contents

- [`Path` struct](#path-struct)
  - [Constructor](#constructor)
  - [Static Methods](#static-methods)
    - [`cwd()`](#cwd)
    - [`path_string(new_path)`](#path_stringnew_path)
    - [`temp_dir()`](#temp_dir)
    - [`temp_file()`](#temp_file)
  - [Instance Methods](#instance-methods)
    - [`to_string()`](#to_string)
    - [`/(new_path)`](#new_path)
    - [`abspath()`](#abspath)
    - [`combine(new_path)`](#combinenew_path)
    - [`contains(path_part)`](#containspath_part)
    - [`copy(new_path)`](#copynew_path)
    - [`exists()`](#exists)
    - [`ext()`](#ext)
    - [`find(path_part)`](#findpath_part)
    - [`is_dir()`](#is_dir)
    - [`is_file()`](#is_file)
    - [`joinpath(path_parts)`](#joinpathpath_parts)
    - [`name()`](#name)
    - [`parent()`](#parent)
    - [`parents()`](#parents)
    - [`parts()`](#parts)
    - [`remove()`](#remove)
    - [`rmdir()`](#rmdir)
    - [`root()`](#root)
    - [`read()`](#read)
    - [`readbytes()` / `read_bytes()`](#readbytes--read_bytes)
    - [`readlines()` / `read_lines()`](#readlines--read_lines)

## `Path` struct

Represents a filesystem path. Inherits from `Hashable`, so it has a `.to_hash()` method that returns `{ path: string }`.

### Constructor

Creates a new `Path` instance from a string.

**Parameters**  
| Type     | Name  | Description                      |
|----------|-------|----------------------------------|
| `string` | `path`| The filesystem path as a string  |

**Example**
```kiwi
p = Path.new("/home/user/documents/report.txt")
```

### Static Methods

#### `cwd()`
Returns a `Path` representing the current working directory.

**Parameters**  
_None_

**Returns**  
| Type  | Description                              |
|-------|------------------------------------------|
| `Path`| Absolute path to the current directory   |

#### `path_string(new_path)`
Helper that converts any path-like value (string or `Path`) to a plain string.

**Parameters**  
| Type     | Name       | Description                          |
|----------|------------|--------------------------------------|
| `any`    | `new_path` | String or `Path` object              |

**Returns**  
| Type     | Description      |
|----------|------------------|
| `string` | Path as string   |

**Throws**  
`TypeError` if `new_path` is neither string nor `Path`

#### `temp_dir()`
Returns a `Path` pointing to the system temporary directory.

**Returns**  
| Type  | Description                  |
|-------|------------------------------|
| `Path`| Path to temporary directory  |

#### `temp_file()`
Creates a new temporary file (0 bytes) and returns its `Path`.

**Returns**  
| Type  | Description                  |
|-------|------------------------------|
| `Path`| Path to the new temp file    |

### Instance Methods

#### `to_string()`
Returns the path as a plain string (used when printing or interpolating).

**Returns**  
| Type     | Description      |
|----------|------------------|
| `string` | The path string  |

#### `/(new_path)`
Convenience operator for joining paths.

**Parameters**  
| Type  | Name       | Description                  |
|-------|------------|------------------------------|
| `any` | `new_path` | String or `Path` to append   |

**Returns**  
| Type  | Description                  |
|-------|------------------------------|
| `Path`| New combined path            |

**Example**
```kiwi
base = Path.new("/etc")
conf = base / "nginx" / "nginx.conf"
```

#### `abspath()`
Returns the absolute version of the path as a string.

**Returns**  
| Type     | Description              |
|----------|--------------------------|
| `string` | Absolute path string     |

#### `combine(new_path)`
Joins this path with another path or path component.

**Parameters**  
| Type  | Name       | Description                  |
|-------|------------|------------------------------|
| `any` | `new_path` | String or `Path`             |

**Returns**  
| Type  | Description                  |
|-------|------------------------------|
| `Path`| Combined path                |

#### `contains(path_part)`
Checks whether the given path component appears in this path's parts.

**Parameters**  
| Type     | Name        | Description           |
|----------|-------------|-----------------------|
| `string` | `path_part` | Part to search for    |

**Returns**  
| Type      | Description                          |
|-----------|--------------------------------------|
| `boolean` | `true` if the part is present        |

#### `copy(new_path)`
Copies the file or directory (recursively) to the destination.

**Parameters**  
| Type  | Name       | Description                  |
|-------|------------|------------------------------|
| `any` | `new_path` | Destination (string or Path) |

**Returns**  
| Type      | Description                          |
|-----------|--------------------------------------|
| `boolean` | `true` if copy succeeded             |

**Behavior**
- Directories → recursive copy (`fio::copyr`)
- Files → single file copy (`fio::copy`)
- When copying file to directory, preserves filename

#### `exists()`
Checks if the path exists on the filesystem.

**Returns**  
| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | `true` if path exists        |

#### `ext()`
Returns the file extension (including the dot) or empty string.

**Returns**  
| Type     | Description              |
|----------|--------------------------|
| `string` | Extension (e.g. ".txt")  |

#### `find(path_part)`
Walks upward from this path until it finds a directory/file with the given name.

**Parameters**  
| Type     | Name        | Description           |
|----------|-------------|-----------------------|
| `string` | `path_part` | Name to search for    |

**Returns**  
| Type  | Description                              |
|-------|------------------------------------------|
| `Path`| Path where the part was found            |

#### `is_dir()`
Checks if the path points to a directory.

**Returns**  
| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | `true` if it is a directory  |

#### `is_file()`
Checks if the path points to a regular file.

**Returns**  
| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | `true` if it is a file       |

#### `joinpath(path_parts)`
Joins multiple path components to the current path.

**Parameters**  
| Type  | Name         | Description                      |
|-------|--------------|----------------------------------|
| `list`| `path_parts` | List of strings or `Path`s       |

**Returns**  
| Type  | Description                  |
|-------|------------------------------|
| `Path`| Combined path                |

#### `name()`
Returns the final component of the path (filename or last directory).

**Returns**  
| Type     | Description              |
|----------|--------------------------|
| `string` | Last path component      |

#### `parent()`
Returns the parent directory of this path.

**Returns**  
| Type  | Description                  |
|-------|------------------------------|
| `Path`| Parent path                  |

#### `parents()`
Returns a list of all ancestor directories, from closest to root.

**Returns**  
| Type  | Description                              |
|-------|------------------------------------------|
| `list`| List of `Path` objects (parent → root)   |

#### `parts()`
Splits the path into its components.

**Returns**  
| Type  | Description                      |
|-------|----------------------------------|
| `list`| List of path parts (strings)     |

#### `remove()`
Deletes the file or **empty** directory.

**Returns**  
| Type      | Description                          |
|-----------|--------------------------------------|
| `boolean` | `true` if removal succeeded          |

**Throws**  
Error if trying to remove non-empty directory (use `rmdir()` instead)

#### `rmdir()`
Recursively deletes a directory and all its contents.

**Returns**  
| Type      | Description                          |
|-----------|--------------------------------------|
| `boolean` | `true` if removal succeeded          |

#### `root()`
Returns the root directory of the filesystem this path belongs to.

**Returns**  
| Type  | Description          |
|-------|----------------------|
| `Path`| Root path (e.g. `/`) |

#### `read()`
Reads the entire file content as a string (empty string if not a file).

**Returns**  
| Type     | Description              |
|----------|--------------------------|
| `string` | File content             |

#### `readbytes()` / `read_bytes()`
Reads the entire file as bytes.

**Returns**  
| Type   | Description              |
|--------|--------------------------|
| `bytes`| File content as bytes    |

#### `readlines()` / `read_lines()`
Reads the file and returns its lines as a list of strings.

**Returns**  
| Type  | Description                  |
|-------|------------------------------|
| `list`| List of lines (strings)      |
