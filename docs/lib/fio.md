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

**Example**

```kiwi
import "fio"

abs = fio::abspath("./data")
println "Absolute path: ${abs}"
# e.g. /home/user/project/data
```

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

**Example**

```kiwi
import "fio"

p = fio::combine(["home", "user", "file.txt"])
println p
# home/user/file.txt
```

#### `ext(_path)`
Extracts the file extension (including the dot).

**Returns**

| Type     | Description                  |
|----------|------------------------------|
| `string` | Extension or `""`            |

**Example**

```kiwi
import "fio"

println fio::ext("report.pdf")   # .pdf
println fio::ext("Makefile")     # (empty string)
```

#### `filename(_path)`
Returns only the final component (filename or last directory name).

**Returns**

| Type     | Description                  |
|----------|------------------------------|
| `string` | Basename                     |

**Example**

```kiwi
import "fio"

println fio::filename("/home/user/data.csv")
# data.csv
```

#### `parentdir(_path)`
Returns the parent directory of the given path.

**Returns**

| Type     | Description                  |
|----------|------------------------------|
| `string` | Parent path                  |

**Example**

```kiwi
import "fio"

println fio::parentdir("/home/user/data.csv")
# /home/user
```

#### `pathparts(_path)`
Splits the path into its components.

**Returns**

| Type  | Description                      |
|-------|----------------------------------|
| `list`| List of path parts (strings)     |

**Example**

```kiwi
import "fio"

parts = fio::pathparts("/home/user/data")
println parts
# ["home", "user", "data"]
```

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

**Example**

```kiwi
import "fio"

ok = fio::chdir("/tmp")
println "Changed to /tmp: ${ok}"
println fio::cwd()
# /tmp
```

#### `cwd()`
Returns the current working directory.

**Returns**

| Type     | Description                  |
|----------|------------------------------|
| `string` | Current directory path       |

**Example**

```kiwi
import "fio"

println fio::cwd()
# /home/user/project
```

#### `isdir(_path)`
Checks if the path points to an existing directory.

**Returns**

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | `true` if it is a directory  |

**Example**

```kiwi
import "fio"

println fio::isdir("/tmp")    # true
println fio::isdir("/tmp/nonexistent")  # false
```

#### `mkdir(_path)`
Creates a single directory (fails if parent directories don't exist).

**Returns**

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | Success                      |

**Example**

```kiwi
import "fio"

ok = fio::mkdir("output")
println "Created output/: ${ok}"
```

#### `mkdirp(_path)`
Creates a directory and all necessary parent directories (`mkdir -p` style).

**Returns**

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | Success                      |

**Example**

```kiwi
import "fio"

# Creates a/, a/b/, and a/b/c/ as needed
ok = fio::mkdirp("a/b/c")
println "Deep mkdir: ${ok}"
```

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

**Example**

```kiwi
import "fio"

# Shallow listing
entries = fio::listdir(".", false)
for entry in entries do
  println entry
end

# Recursive listing — every file under the current directory
all_files = fio::listdir(".", true)
println "Total entries: ${all_files.size()}"
```

#### `rmdir(_path)`
Removes an **empty** directory.

**Returns**

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | Success                      |

**Example**

```kiwi
import "fio"

fio::mkdir("empty_folder")
ok = fio::rmdir("empty_folder")
println "Removed: ${ok}"
```

#### `rmdirf(_path)`
Recursively deletes a directory and everything inside it.

**Returns**

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | Success                      |

**Example**

```kiwi
import "fio"

fio::mkdirp("trash/a/b")
fio::write("trash/a/b/file.txt", "bye")
ok = fio::rmdirf("trash")
println "Deleted trash/: ${ok}"
```

### File operations

#### `create(_path)`
Creates an empty file (equivalent to `touch` but may behave differently on some platforms).

**Returns**

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | Success                      |

**Example**

```kiwi
import "fio"

ok = fio::create("new.txt")
println "Created: ${ok}"
println "Size: ${fio::filesize("new.txt")}"  # 0
```

#### `touch(_path)`
Creates an empty file or updates the access/modification time of an existing file.

**Returns**

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | Success                      |

**Example**

```kiwi
import "fio"

fio::touch("log.txt")        # creates if absent, updates mtime if present
println fio::exists("log.txt")  # true
```

#### `exists(_path)`
Checks whether a file **or** directory exists at the path.

**Returns**

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | `true` if path exists        |

**Example**

```kiwi
import "fio"

if fio::exists("config.json")
  println "Config found"
else
  println "No config — using defaults"
end
```

#### `isfile(_path)`
Checks if the path points to a regular file.

**Returns**

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | `true` if it is a file       |

**Example**

```kiwi
import "fio"

println fio::isfile("report.pdf")   # true (if it exists as a file)
println fio::isfile("/tmp")         # false (directory)
```

#### `filesize(_path)`
Returns the size of a file in bytes.

**Returns**

| Type      | Description                  |
|-----------|------------------------------|
| `integer` | File size in bytes           |

**Example**

```kiwi
import "fio"

bytes = fio::filesize("data.csv")
println "Size: ${bytes} bytes"
```

#### `fileinfo(_path)`
Returns detailed metadata about a file.

**Returns**

| Type      | Description                                                                 |
|-----------|-----------------------------------------------------------------------------|
| `hashmap` | `{ creation_time, name, extension, full_name, last_access_time, last_write_time, attributes }` |

**Example**

```kiwi
import "fio"

info = fio::fileinfo("report.pdf")
println "Name:      ${info["name"]}"
println "Extension: ${info["extension"]}"
println "Modified:  ${info["last_write_time"]}"
println "Created:   ${info["creation_time"]}"
```

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

**Example**

```kiwi
import "fio"

ok = fio::copy("a.txt", "b.txt")
println "Copied: ${ok}"

# Prevent overwrite
ok = fio::copy("a.txt", "b.txt", false)
println "Overwrite prevented: ${!ok}"
```

#### `copyr(source, dest)`
Recursively copies a directory (and all contents).

**Returns**

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | Success                      |

**Example**

```kiwi
import "fio"

ok = fio::copyr("src_dir", "backup_dir")
println "Directory copied: ${ok}"
```

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

**Example**

```kiwi
import "fio"

# Rename a file
fio::move("old.txt", "new.txt")

# Move into a directory (overwrite allowed)
fio::move("new.txt", "archive/new.txt", true)
```

#### `remove(_path)`
Deletes a file (not directories).

**Returns**

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | Success                      |

**Example**

```kiwi
import "fio"

fio::write("temp.txt", "delete me")
ok = fio::remove("temp.txt")
println "Deleted: ${ok}"
println "Still exists: ${fio::exists("temp.txt")}"  # false
```

### Reading

#### `read(_path)`
Reads entire file content as UTF-8 string.

**Returns**

| Type     | Description          |
|----------|----------------------|
| `string` | File content         |

**Example**

```kiwi
import "fio"

content = fio::read("notes.txt")
println content
```

#### `readbytes(_path)`
Reads entire file as raw bytes.

**Returns**

| Type   | Description          |
|--------|----------------------|
| `bytes`| File content         |

**Example**

```kiwi
import "fio"

raw = fio::readbytes("image.png")
println "Read ${raw.size()} bytes"
```

#### `readlines(_path)`
Reads file and splits it into lines.

**Returns**

| Type  | Description                  |
|-------|------------------------------|
| `list`| List of strings (lines)      |

**Example**

```kiwi
import "fio"

lines = fio::readlines("data.csv")
for line in lines do
  cols = line.split(",")
  println cols[0]
end
```

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

**Example**

```kiwi
import "fio"

# Read the first 4 bytes (e.g. to check a file magic number)
header = fio::readslice("archive.zip", 0, 4)
println header.to_list()
```

### Writing

#### `write(_path, _text)`
Overwrites file with new text content.

**Returns**

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | Success                      |

**Example**

```kiwi
import "fio"

fio::write("out.txt", "Hello, world!")
println fio::read("out.txt")
# Hello, world!
```

#### `writeln(_path, _text)`
**Appends** one line (adds newline).

**Returns**

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | Success                      |

**Example**

```kiwi
import "fio"

fio::write("log.txt", "")   # start fresh
fio::writeln("log.txt", "Line 1")
fio::writeln("log.txt", "Line 2")
fio::writeln("log.txt", "Line 3")
println fio::read("log.txt")
# Line 1
# Line 2
# Line 3
```

#### `append(_path, _text)`
Appends text without adding a newline.

**Returns**

| Type      | Description                  |
|-----------|------------------------------|
| `boolean` | Success                      |

**Example**

```kiwi
import "fio"

fio::write("out.txt", "Hello")
fio::append("out.txt", ", world!")
println fio::read("out.txt")
# Hello, world!
```

#### `writebytes(_path, _data)`
Overwrites file with raw bytes.

**Returns**

| Type      | Description                  |
|-----------|------------------------------|
| `integer` | Number of bytes written      |

**Example**

```kiwi
import "fio"

raw = fio::readbytes("original.bin")
n = fio::writebytes("copy.bin", raw)
println "Wrote ${n} bytes"
```

#### `writeslice(_path, _offset, _data)`
Writes bytes at a specific position (seeks + writes).

**Returns**

| Type      | Description                  |
|-----------|------------------------------|
| `integer` | Number of bytes written      |

**Example**

```kiwi
import "fio"

# Patch bytes starting at offset 16
patch = "PATCHED".to_bytes()
n = fio::writeslice("binary.dat", 16, patch)
println "Patched ${n} bytes at offset 16"
```

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

**Example**

```kiwi
import "fio"

# All .kiwi files under the current directory
kiwi_files = fio::glob(".", ["**/*.kiwi"])
for f in kiwi_files do
  println f
end

# All .txt files, excluding anything in a temp/ folder
txt_files = fio::glob(".", ["**/*.txt"], ["**/temp*"])
println "Found ${txt_files.size()} non-temp text files"
```

### Temporary files / directories

#### `tmpdir()`
Returns path to system temporary directory.

**Returns**

| Type     | Description                  |
|----------|------------------------------|
| `string` | Temp directory path          |

**Example**

```kiwi
import "fio"

println fio::tmpdir()
# /tmp  (on Linux/macOS)
```

#### `tmpfile()`
Creates a new empty temporary file and returns its path.

**Returns**

| Type     | Description                  |
|----------|------------------------------|
| `string` | Path to new temp file        |

**Example**

```kiwi
import "fio"

tmp = fio::tmpfile()
println "Temp file: ${tmp}"

fio::write(tmp, "scratch data")
println fio::read(tmp)

fio::remove(tmp)
```
