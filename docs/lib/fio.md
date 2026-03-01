# `fio`

The `fio` package (short for **file I/O**) provides low-level, procedural filesystem operations in Kiwi.  
It serves as the foundation for higher-level abstractions like the `path` module. Most functions accept plain strings as paths and return simple values (usually booleans for success/failure, or the expected data type).

---

## Package Functions

### Path manipulation

#### `abspath(_path)`
Returns the absolute (fully resolved) path.

**Parameters**

| Type     | Name     | Description                  |
|----------|----------|------------------------------|
| `string` | `_path`  | Relative or absolute path    |

**Returns**  

| Type     | Description                  |
|----------|------------------------------|
| `string` | Absolute path                |

#### `combine(_path_parts)`
Joins path components using the platform-appropriate separator.

**Parameters**

| Type  | Name            | Description                       |
|-------|-----------------|-----------------------------------|
| `list`| `_path_parts`   | List of path segments (strings)   |

**Returns**  

| Type     | Description                  |
|----------|------------------------------|
| `string` | Combined path                |

#### `ext(_path)`
Extracts the file extension (including the dot).

**Returns**  

| Type     | Description                  |
|----------|------------------------------|
| `string` | Extension or `""`            |

#### `filename(_path)`
Returns only the final component (filename or last directory name).

**Returns**  

| Type     | Description                  |
|----------|------------------------------|
| `string` | Basename                     |

#### `parentdir(_path)`
Returns the parent directory of the given path.

**Returns**  

| Type     | Description                  |
|----------|------------------------------|
| `string` | Parent path                  |

#### `pathparts(_path)`
Splits the path into its components.

**Returns**  

| Type  | Description                      |
|-------|----------------------------------|
| `list`| List of path parts (strings)     |

### Directory operations

#### `chdir(_path)`
Changes the current working directory.

**Parameters**

| Type     | Name     | Description                  |
|----------|----------|------------------------------|
| `string` | `_path`  | Target directory             |

**Returns**  

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | Success                      |

#### `cwd()`
Returns the current working directory.

**Returns**  

| Type     | Description                  |
|----------|------------------------------|
| `string` | Current directory path       |

#### `isdir(_path)`
Checks if the path points to an existing directory.

**Returns**  

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | `true` if it is a directory  |

#### `mkdir(_path)`
Creates a single directory (fails if parent directories don't exist).

**Returns**  

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | Success                      |

#### `mkdirp(_path)`
Creates a directory and all necessary parent directories (`mkdir -p` style).

**Returns**  

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | Success                      |

#### `listdir(_path, _recursive)`
Lists directory contents.

**Parameters**

| Type      | Name          | Description                         | Default |
|-----------|---------------|-------------------------------------|---------|
| `string`  | `_path`       | Directory to list                   | —       |
| `boolean` | `_recursive`  | Include subdirectories recursively  | `false` |

**Returns**  

| Type  | Description                      |
|-------|----------------------------------|
| `list`| List of file/directory names     |

#### `rmdir(_path)`
Removes an **empty** directory.

**Returns**  

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | Success                      |

#### `rmdirf(_path)`
Recursively deletes a directory and everything inside it.

**Returns**  

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | Success                      |

### File operations

#### `create(_path)`
Creates an empty file (equivalent to `touch` but may behave differently on some platforms).

**Returns**  

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | Success                      |

#### `touch(_path)`
Creates an empty file or updates the access/modification time of an existing file.

**Returns**  

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | Success                      |

#### `exists(_path)`
Checks whether a file **or** directory exists at the path.

**Returns**  

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | `true` if path exists        |

#### `isfile(_path)`
Checks if the path points to a regular file.

**Returns**  

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | `true` if it is a file       |

#### `filesize(_path)`
Returns the size of a file in bytes.

**Returns**  

| Type      | Description                  |
|-----------|------------------------------|
| `integer` | File size in bytes           |

#### `fileinfo(_path)`
Returns detailed metadata about a file.

**Returns**  

| Type      | Description                                                                 |
|-----------|-----------------------------------------------------------------------------|
| `hashmap` | `{ creation_time, name, extension, full_name, last_access_time, last_write_time, attributes }` |

#### `copy(source, dest, overwrite)`
Copies a single file.

**Parameters**

| Type      | Name        | Description                  | Default |
|-----------|-------------|------------------------------|---------|
| `string`  | `source`    | Source file                  | —       |
| `string`  | `dest`      | Destination                  | —       |
| `boolean` | `overwrite` | Overwrite if exists          | `true`  |

**Returns**  

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | Success                      |

#### `copyr(source, dest)`
Recursively copies a directory (and all contents).

**Returns**  

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | Success                      |

#### `move(source, dest, overwrite)`
Moves or renames a file/directory.

**Parameters**

| Type      | Name        | Description                  | Default |
|-----------|-------------|------------------------------|---------|
| `string`  | `source`    | Source path                  | —       |
| `string`  | `dest`      | Destination path             | —       |
| `boolean` | `overwrite` | Overwrite if exists          | `false` |

**Returns**  

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | Success                      |

#### `remove(_path)`
Deletes a file (not directories).

**Returns**  

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | Success                      |

### Reading

#### `read(_path)`
Reads entire file content as UTF-8 string.

**Returns**  

| Type     | Description          |
|----------|----------------------|
| `string` | File content         |

#### `readbytes(_path)`
Reads entire file as raw bytes.

**Returns**  

| Type   | Description          |
|--------|----------------------|
| `bytes`| File content         |

#### `readlines(_path)`
Reads file and splits it into lines.

**Returns**  

| Type  | Description                  |
|-------|------------------------------|
| `list`| List of strings (lines)      |

#### `readslice(_path, _offset, _size)`
Reads a portion of the file as bytes.

**Parameters**

| Type      | Name       | Description                  |
|-----------|------------|------------------------------|
| `string`  | `_path`    | File path                    |
| `integer` | `_offset`  | Starting position            |
| `integer` | `_size`    | Number of bytes to read      |

**Returns**  

| Type   | Description          |
|--------|----------------------|
| `bytes`| Requested slice      |

### Writing

#### `write(_path, _text)`
Overwrites file with new text content.

**Returns**  

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | Success                      |

#### `writeln(_path, _text)`
**Appends** one line (adds newline).

**Returns**  

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | Success                      |

#### `append(_path, _text)`
Appends text without adding a newline.

**Returns**  

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | Success                      |

#### `writebytes(_path, _data)`
Overwrites file with raw bytes.

**Returns**  

| Type      | Description                  |
|-----------|------------------------------|
| `integer` | Number of bytes written      |

#### `writeslice(_path, _offset, _data)`
Writes bytes at a specific position (seeks + writes).

**Returns**  

| Type      | Description                  |
|-----------|------------------------------|
| `integer` | Number of bytes written      |

### Pattern matching

#### `glob(_path, _include_patterns, _exclude_patterns)`
Finds files matching glob patterns.

**Parameters**

| Type  | Name                 | Description                              | Default |
|-------|----------------------|------------------------------------------|---------|
| `string` | `_path`           | Base directory                           | —       |
| `list`   | `_include_patterns`| Glob patterns to match (e.g. `["*.txt"]`) | —       |
| `list`   | `_exclude_patterns`| Patterns to exclude                      | `[]`    |

**Returns**  

| Type  | Description                      |
|-------|----------------------------------|
| `list`| Matching absolute paths          |

### Temporary files / directories

#### `tmpdir()`
Returns path to system temporary directory.

**Returns**  

| Type     | Description                  |
|----------|------------------------------|
| `string` | Temp directory path          |

#### `tmpfile()`
Creates a new empty temporary file and returns its path.

**Returns**  

| Type     | Description                  |
|----------|------------------------------|
| `string` | Path to new temp file        |
