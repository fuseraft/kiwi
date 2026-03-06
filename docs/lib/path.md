# `path`

The `path` package provides a clean, object-oriented way to work with filesystem paths in Kiwi, inspired by Python's `pathlib`.  
It defines a single main struct `Path` that wraps a path string and offers convenient methods for path manipulation, filesystem queries, and file/directory operations.

---

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
- Directories -> recursive copy (`fio::copyr`)
- Files -> single file copy (`fio::copy`)
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

#### `mkdir()`
Creates a directory from the path.

**Returns**  

| Type      | Description                          |
|-----------|--------------------------------------|
| `boolean` | `true` if creation succeeded          |

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
| `list`| List of `Path` objects (parent -> root)   |

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

#### `touch()`
Creates a file from the path.

**Returns**  

| Type      | Description                          |
|-----------|--------------------------------------|
| `boolean` | `true` if creation succeeded          |

---

## Examples

### Basic path construction and joining

```kiwi
import "path"

p = Path.new("/home/user/projects")
report = p / "2024" / "report.txt"

println report.to_string()   # /home/user/projects/2024/report.txt
println report.name()        # report.txt
println report.ext()         # .txt
println report.parent()      # /home/user/projects/2024
```

### Working directory and parts

```kiwi
import "path"

cwd = Path.cwd()
println cwd.to_string()   # /home/user/projects

println cwd.parts()       # ["home", "user", "projects"]
println cwd.root()        # /
```

### Filesystem queries

```kiwi
import "path"

p = Path.new("/etc/hosts")
println p.exists()    # true
println p.is_file()   # true
println p.is_dir()    # false

d = Path.new("/etc")
println d.is_dir()    # true
```

### Reading files

```kiwi
import "path"

p = Path.new("/etc/hostname")

println p.read()           # "mymachine\n"
println p.readlines()      # ["mymachine"]
bytes = p.readbytes()
println bytes.size()       # byte count
```

### Ancestor traversal

```kiwi
import "path"

p = Path.new("/home/user/projects/app/src")
println p.parent()         # /home/user/projects/app

all_parents = p.parents()
for ancestor in all_parents do
  println ancestor
end
# /home/user/projects/app
# /home/user/projects
# /home/user
# /home
# /
```

### joinpath with multiple components

```kiwi
import "path"

base = Path.new("/var/log")
full = base.joinpath(["nginx", "access.log"])
println full.to_string()   # /var/log/nginx/access.log
```

### Absolute path resolution

```kiwi
import "path"

p = Path.new("../config")
println p.abspath()   # resolved absolute path
```

### Temporary files and directories

```kiwi
import "path"

tmp = Path.temp_dir()
println tmp.to_string()   # /tmp  (or system temp dir)

f = Path.temp_file()
println f.exists()   # true
f.remove()
```

### Find an ancestor by name

```kiwi
import "path"

p = Path.new("/home/user/projects/myapp/src/main.kiwi")
found = p.find("myapp")
println found.to_string()   # /home/user/projects/myapp
```

### Check if path contains a component

```kiwi
import "path"

p = Path.new("/home/user/projects")
println p.contains("user")      # true
println p.contains("admin")     # false
```
