# path

> The Path struct, an object-oriented interface to filesystem paths.

The `path` package defines `Path`, an object wrapping a filesystem path, in the spirit of Python's `pathlib`. Create one with `Path.new(text)` or `text.to_path()`, and join paths with `/`.

```kiwi
report = Path.new("/home/user/docs/report.txt")

println report.name()                           # report.txt
println report.ext()                            # .txt
println report.parent().to_string()             # /home/user/docs
println (report.parent() / "notes.txt").to_string()   # /home/user/docs/notes.txt
println report.parts()                          # ["home", "user", "docs", "report.txt"]
println report.contains("docs")                 # true

println "/tmp".to_path().joinpath(["a", "b.txt"]).to_string()   # /tmp/a/b.txt
```

`/` and `joinpath` accept strings or other `Path` values. Methods that return a path return a new `Path`; call `to_string()` to get the text.

## Filesystem operations

```kiwi
scratch = Path.temp_dir() / "path-demo.txt"
scratch.touch()
println scratch.exists()      # true
println scratch.is_file()     # true

fio::write(scratch.to_string(), "hello\nworld")
println scratch.readlines()   # ["hello", "world"]

scratch.remove()
println scratch.exists()      # false
```

| Method | Does |
| :-- | :-- |
| `exists()`, `is_file()`, `is_dir()` | Test the path. |
| `read()`, `readlines()`, `readbytes()` | Read the file. |
| `touch()`, `mkdir()` | Create a file, or a directory with its parents. |
| `copy(dest)` | Copy the file. |
| `remove()`, `rmdir()` | Delete a file or directory. |
| `abspath()` | The absolute path as a string. |
| `parents()` | Every ancestor directory, nearest first. |
| `find(part)` | The nearest ancestor (or self) whose name is `part`. |
| `root()` | The root of the path. |
| `Path.cwd()`, `Path.temp_dir()`, `Path.temp_file()` | The working directory, temp directory, or a new temp file. |

For writing files, use [`fio`](fio.md) with `path.to_string()`.

## `Path` struct

Inherits from `Hashable`.

A struct representing a filesystem path.

### `Path.new`

```kiwi
Path.new(path: string)
```

Creates a new Path instance.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path` | `string` |  | The path as a string. |

### `Path.cwd`

```kiwi
static Path.cwd(): Path
```

Returns the absolute path to the current working directory.

**Returns** `Path` — Returns a path representing the absolute path of the current working directory.

### `Path.path_string`

```kiwi
static Path.path_string(new_path): string
```

Coerces a path into a string.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `new_path` | `any` |  | A new path (or path part) either as a string or a Path. |

**Returns** `string` — Returns a string representing a path.

**Throws** `TypeError` — Expects new_path to be a string or a Path.

### `Path.temp_dir`

```kiwi
static Path.temp_dir(): Path
```

Returns a path to the temp directory.

**Returns** `Path` — Returns a path to the temp directory.

### `Path.temp_file`

```kiwi
static Path.temp_file(): Path
```

Creates a 0 byte file and returns its path.

**Returns** `Path` — Returns a path to the file.

### `.to_string`

```kiwi
to_string(): string
```

Overrides the `to_string()` method to return the path as a string.

**Returns** `string` — Returns a string representing the path.

### `operator /`

```kiwi
fn /(new_path): Path
```

Operator overload to combine (join) paths.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `new_path` | `any` |  | A new path (or path part) either as a string or a Path. |

**Returns** `Path` — Returns a path containing this path and the new path.

**Throws** `TypeError` — Expects new_path to be a string or a Path.

### `.abspath`

```kiwi
abspath(): string
```

Returns the absolute path as a string.

**Returns** `string` — A string containing the absolute path.

### `.combine`

```kiwi
combine(new_path): Path
```

Combines (joins) this path with another.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `new_path` | `any` |  | A new path (or path part) either as a string or a Path. |

**Returns** `Path` — Returns a path containing this path and the new path.

**Throws** `TypeError` — Expects new_path to be a string or a Path.

### `.contains`

```kiwi
contains(path_part: string): boolean
```

Checks if the parts contains a given part.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path_part` | `string` |  | The path part as a string. |

**Returns** `boolean` — Returns `true` when the path contains the given path part.

### `.mkdir`

```kiwi
mkdir(): boolean
```

Creates a directory from the path.

**Returns** `boolean` — Returns `true` when the directory is created successfully.

### `.copy`

```kiwi
copy(new_path): boolean
```

Copies data from one path to another. If the path is a directory, all subdirectory contents are copied as well.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `new_path` | `any` |  | A new path either as a string or a Path. |

**Returns** `boolean` — Returns `true` when the data was copied.

**Throws** `TypeError` — Expects new_path to be a string or a Path.

### `.exists`

```kiwi
exists(): boolean
```

Checks if the path exists.

**Returns** `boolean` — Returns `true` when the path exists on the filesystem.

### `.ext`

```kiwi
ext(): string
```

Gets the file extension of the path.

**Returns** `string` — Returns a string containing the file extension of the path.

### `.find`

```kiwi
find(path_part: string): Path
```

Gets a new path from a path part.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path_part` | `string` |  | A part of a path as a string. |

**Returns** `Path` — Returns the path at the given path part.

### `.is_dir`

```kiwi
is_dir(): boolean
```

Checks if the path is a directory.

**Returns** `boolean` — Returns `true` when the path is a directory.

### `.is_file`

```kiwi
is_file(): boolean
```

Checks if the path is a file.

**Returns** `boolean` — Returns `true` when the path is a file.

### `.joinpath`

```kiwi
joinpath(path_parts: list): Path
```

Combines (joins) multiple path parts to this path.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `path_parts` | `list` |  | A list of path parts. |

**Returns** `Path` — Returns a new path by combining this path with a list of path parts.

### `.name`

```kiwi
name(): string
```

Gets the last part of the path as a string.

**Returns** `string` — Returns a string containing the last part of the path.

### `.parent`

```kiwi
parent(): Path
```

Gets the parent path.

**Returns** `Path` — Returns a Path containing the parent of this path.

### `.parents`

```kiwi
parents(): list
```

Gets a list of paths containing all parents of this path.

**Returns** `list` — Returns a list of paths containing all parents of this path.

### `.parts`

```kiwi
parts(): list
```

Gets the list of path parts in this path.

**Returns** `list` — Returns a list of path parts in this path.

### `.remove`

```kiwi
remove(): boolean
```

Removes this path from the file system.

**Returns** `boolean` — Returns `true` when the path was successfully removed from the file system.

**Throws** `FileSystemError` — Throws when the path is a non-empty directory. Use `rmdir()` instead.

### `.rmdir`

```kiwi
rmdir(): boolean
```

Removes the path from the file system, and all of its subpaths.

**Returns** `boolean` — Returns `true` when the path was successfully removed from the file system.

### `.root`

```kiwi
root(): Path
```

Gets the path root.

**Returns** `Path` — Returns a Path representing the root of this path.

### `.readbytes`

```kiwi
readbytes(): bytes
```

Gets all content from the path as bytes.

**Returns** `bytes` — Returns bytes containing the content of the file this path points to.

### `.readlines`

```kiwi
readlines(): list
```

Gets all content from the path as a list of strings representing the lines in the file.

**Returns** `list` — Returns a list of stirngs containing the line content of the file this path points to.

### `.read`

```kiwi
read(): string
```

Gets all content from the path as a string.

**Returns** `bytes` — Returns bytes containing the content of the file this path points to.

### `.read_bytes`

```kiwi
read_bytes(): bytes
```

Gets all content from the path as bytes.

**Returns** `bytes` — Returns bytes containing the content of the file this path points to.

### `.read_lines`

```kiwi
read_lines(): list
```

Gets all content from the path as a list of strings representing the lines in the file.

**Returns** `list` — Returns a list of stirngs containing the line content of the file this path points to.

### `.touch`

```kiwi
touch(): boolean
```

Creates a file from the path.

**Returns** `boolean` — Returns `true` when the file is created successfully.
