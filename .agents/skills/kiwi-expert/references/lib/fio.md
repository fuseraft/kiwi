# fio

> Read, write, copy and list files and directories.

The `fio` package (file I/O) works with the filesystem using plain string paths. For an object-oriented interface, see [`path`](path.md). All `fio` functions are unavailable in [safe mode](../tools/cli.md#safe-mode).

## Reading and writing

```kiwi
dir = fio::combine([fio::tmpdir(), "fio-demo"])
fio::mkdirp(dir)
notes = fio::combine([dir, "notes.txt"])

fio::write(notes, "first line\n")        # create or overwrite
fio::append(notes, "second line\n")      # add to the end
fio::writeln(notes, ["third", "fourth"]) # append lines

println fio::read(notes)                 # the whole file as a string
println fio::readlines(notes)            # ["first line", "second line", "third", "fourth"]

fio::rmdirf(dir)
```

| Function | Does |
| :-- | :-- |
| `read(path)` | Returns the file as a string. |
| `readlines(path)` | Returns the lines as a list. |
| `readbytes(path)` | Returns the file as bytes. |
| `readslice(path, offset, size)` | Returns `size` bytes starting at `offset`. |
| `write(path, text)` | Replaces the file's contents. |
| `append(path, text)` | Adds text to the end. |
| `writeln(path, text_or_list)` | Appends one line, or each element of a list as a line. |
| `writebytes(path, bytes)` | Replaces the file's contents with bytes. |
| `writeslice(path, offset, bytes)` | Overwrites bytes at an offset. |

## Files and directories

```kiwi
dir = fio::combine([fio::tmpdir(), "fio-tree"])
fio::mkdirp(fio::combine([dir, "logs"]))
fio::write(fio::combine([dir, "a.txt"]), "A")
fio::write(fio::combine([dir, "logs", "app.log"]), "started")

println fio::isdir(dir)                                        # true
println fio::listdir(dir).map(do (p) => fio::filename(p)).sort()  # ["a.txt", "logs"]
println fio::listdir(dir, true).size()                         # 3 (recursive)

fio::copy(fio::combine([dir, "a.txt"]), fio::combine([dir, "b.txt"]))
fio::move(fio::combine([dir, "b.txt"]), fio::combine([dir, "c.txt"]))
println fio::exists(fio::combine([dir, "c.txt"]))              # true

fio::rmdirf(dir)                                               # remove recursively
```

| Function | Does |
| :-- | :-- |
| `exists`, `isfile`, `isdir` | Test a path. |
| `filesize(path)` | Size in bytes. |
| `fileinfo(path)` | A hashmap with `name`, `full_name`, `extension`, `attributes` and creation, access and write times. |
| `create(path)`, `touch(path)` | Create an empty file. |
| `copy(src, dest, overwrite)`, `copyr(src, dest)` | Copy a file, or a directory recursively. |
| `move(src, dest, overwrite)` | Move or rename. |
| `remove(path)` | Delete a file. |
| `mkdir(path)`, `mkdirp(path)` | Create a directory, or a directory and its parents. |
| `rmdir(path)`, `rmdirf(path)` | Remove an empty directory, or a directory and everything in it. |
| `listdir(path, recursive)` | List entries as full paths. |
| `glob(dir, include, exclude)` | List files matching glob patterns. |
| `cwd()`, `chdir(path)` | Get or change the working directory. |
| `tmpdir()`, `tmpfile()` | The temp directory, or a new empty temp file. |

### Globs

`glob` searches a directory for files that match any of the include patterns and none of the exclude patterns. `**` matches across directories.

```kiwi
sources = fio::glob("./", ["./**/*.kiwi"], ["./tests/**"])
```

## Paths

| Function | Returns |
| :-- | :-- |
| `combine(parts)` | The parts joined with the platform separator. |
| `abspath(path)` | The absolute path. |
| `filename(path)` | The last component: `"notes.txt"`. |
| `ext(path)` | The extension with its dot: `".txt"`, or `""`. |
| `parentdir(path)` | The containing directory. |
| `pathparts(path)` | The components as a list. |

```kiwi
println fio::combine(["logs", "2024", "app.log"])   # logs/2024/app.log (on Linux and macOS)
println fio::filename("/var/log/app.log")           # app.log
println fio::ext("archive.tar.gz")                  # .gz
println fio::pathparts("/a/b/c.txt")                # ["a", "b", "c.txt"]
```

## Functions

### `fio::abspath`

```kiwi
fio::abspath(_path): string
```

Get the absolute path of a file.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The path to a file or a filename. |

**Returns** `string` — The absolute path of a file.

**Throws** `FileSystemError` — When unable to determine absolute path.

### `fio::append`

```kiwi
fio::append(_path: string, _text: string = "")
```

Appends text to a file.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The path to a file or a filename. |
| `_text` | `string` | `""` | The text to append. |

**Returns** `boolean` — Returns `true` when the file is written to.

**Throws** `FileSystemError` — When file writing fails.

### `fio::chdir`

```kiwi
fio::chdir(_path: string): boolean
```

Changes the current directory.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The path to a directory. |

**Returns** `boolean` — Returns `true` when the directory is changed.

**Throws** `FileSystemError` — When changing to the directory fails.

### `fio::combine`

```kiwi
fio::combine(_path_parts: list = []): string
```

Combine two paths.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path_parts` | `list` | `[]` | A list of path parts. |

**Returns** `string` — A string from the list of parts.

### `fio::copy`

```kiwi
fio::copy(source: string, dest: string, overwrite: boolean = true)
```

Copy a file.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `source` | `string` |  | The source path. |
| `dest` | `string` |  | The destination path. |
| `overwrite` | `boolean` | `true` | A flag to indicate whether to overwrite the file (defaults to true). |

**Returns** `boolean` — Returns `true` when copied successfully.

**Throws** `FileSystemError` — When copying fails.

### `fio::copyr`

```kiwi
fio::copyr(source: string, dest: string)
```

Copy a directory recursively.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `source` | `string` |  | The source path. |
| `dest` | `string` |  | The destination path. |

**Returns** `boolean` — Returns `true` when the copy operation succeeds.

**Throws** `FileSystemError` — When copying fails.

### `fio::create`

```kiwi
fio::create(_path: string)
```

Creates an empty file.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The path to the file. |

**Returns** `boolean` — Returns `true` when the file creation succeeds.

**Throws** `FileSystemError` — When file creation fails.

### `fio::cwd`

```kiwi
fio::cwd(): string
```

Get the current working directory.

**Returns** `string` — The current working directory as a string.

### `fio::exists`

```kiwi
fio::exists(_path: string): boolean
```

Check if a path exists.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The path to a file or directory. |

**Returns** `boolean` — Returns `true` when the path exists on the filesystem.

### `fio::ext`

```kiwi
fio::ext(_path: string)
```

Gets the file extension of the path.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The path to a file or a filename. |

**Returns** `string` — Returns a string containing the file extension of the path.

### `fio::filename`

```kiwi
fio::filename(_path: string): string
```

Get the filename from a file path.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The path to a file or a filename. |

**Returns** `string` — The filename as a string.

### `fio::filesize`

```kiwi
fio::filesize(_path: string): integer
```

Get the size of a file in bytes.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The path to a file. |

**Returns** `integer` — Returns the number of bytes in the file as an integer.

**Throws** `FileReadError` — When unable to determine file size.

### `fio::glob`

```kiwi
fio::glob(_path: string, _include_patterns: list, _exclude_patterns: list = []): list
```

Get a list of files from glob patterns.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The path to search. |
| `_include_patterns` | `list` |  | A list of glob patterns to match against paths. |
| `_exclude_patterns` | `list` | `[]` | A list of glob patterns for filtering matches. Defaults to no exclusions. |

**Returns** `list` — Returns a list of matched file paths.

**Throws** `FileSystemError` — When unable to glob a path.

### `fio::fileinfo`

```kiwi
fio::fileinfo(_path: string): hashmap
```

Get a fileinfo hashmap for a file.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The file path. |

**Returns** `hashmap` —  Returns a hashmap with the following keys: - `creation_time`: The creation time as a date. - `name`: The name as a string. - `extension`: The file extension as a string. - `full_name`: The full name as a string. - `last_access_time`: The last access timestamp as a date. - `last_write_time`: The last write timestamp as a date. - `attributes`: A hashmap of file attributes mapped to booleans representing whether they are set.

**Throws** `FileSystemError` — When unable to glob a path.

### `fio::isdir`

```kiwi
fio::isdir(_path: string): boolean
```

Check if a directory exists.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The path to a directory. |

**Returns** `boolean` — Returns `true` when the directory exists.

### `fio::isfile`

```kiwi
fio::isfile(_path: string): boolean
```

Check if a file exists.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The path to a file. |

**Returns** `boolean` — Returns `true` when the file exists.

### `fio::listdir`

```kiwi
fio::listdir(_path: string, _recursive: boolean = false): list
```

Retrieve a list of directory entries for a given path.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The directory path. |
| `_recursive` | `boolean` | `false` | A flag to control recursive retrieval. Defaults to false. |

**Returns** `list` — Returns a list of strings containing the directory entries.

### `fio::mkdir`

```kiwi
fio::mkdir(_path: string): boolean
```

Creates a directory.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The directory path. |

**Returns** `boolean` — Returns `true` when directory creation succeeds.

**Throws** `FileSystemError` — When directory creation fails.

### `fio::mkdirp`

```kiwi
fio::mkdirp(_path: string): boolean
```

Creates a directory and all subdirectories specified in the path.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The directory path. |

**Returns** `boolean` — Returns `true` when directory creation succeeds.

**Throws** `FileSystemError` — When directory creation fails.

### `fio::move`

```kiwi
fio::move(source: string, dest: string, overwrite: boolean = false)
```

Move or rename a file.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `source` | `string` |  | The source path. |
| `dest` | `string` |  | The destination path. |
| `overwrite` | `boolean` | `false` | A flag to indicate whether to overwrite the file (defaults to false). |

**Returns** `boolean` — Returns `true` when the file move succeeds.

**Throws** `FileSystemError` — When the file move fails.

### `fio::parentdir`

```kiwi
fio::parentdir(_path: string): string
```

Get the parent directory of an absolute path.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The path to a file. |

**Returns** `string` — Returns a string containing the parent path.

### `fio::read`

```kiwi
fio::read(_path: string): string
```

Get the content of a file as a string.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The path to a file. |

**Returns** `string` — Returns the contents of the file as a string.

**Throws** `FileReadError` — When unable to read the file.

### `fio::readlines`

```kiwi
fio::readlines(_path: string): list
```

Get the content of a file as a list of strings.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The path to a file. |

**Returns** `list` — Returns the contents of the file as a list of strings.

**Throws** `FileReadError` — When unable to read the file.

### `fio::readslice`

```kiwi
fio::readslice(_path: string, _offset: integer, _size: integer): bytes
```

Get a slice of a file as bytes.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The path to a file. |
| `_offset` | `integer` |  | The position in the file to read from. |
| `_size` | `integer` |  | The number of bytes to read from the file. |

**Returns** `list` — Returns bytes containing a slice of the file.

**Throws** `IndexError` — When the index is invalid.

**Throws** `FileReadError` — When the file cannot be read.

### `fio::readbytes`

```kiwi
fio::readbytes(_path: string): bytes
```

Gets all content from the path as bytes.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The path to a file. |

**Returns** `bytes` — Returns bytes containing the content of the file this path points to.

**Throws** `FileReadError` — When the file cannot be read.

### `fio::remove`

```kiwi
fio::remove(_path: string): boolean
```

Delete a file.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The file path. |

**Returns** `boolean` — Returns `true` when the file is removed.

**Throws** `FileSystemError` — When the file cannot be removed.

### `fio::rmdir`

```kiwi
fio::rmdir(_path: string): boolean
```

Delete a directory.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The directory path. |

**Returns** `boolean` — Returns `true` when the directory is removed.

**Throws** `FileSystemError` — When the directory cannot be removed.

### `fio::rmdirf`

```kiwi
fio::rmdirf(_path)
```

Delete a directory and all of its contents recursively.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The directory path. |

**Returns** `boolean` — Returns `true` when the directory is removed.

**Throws** `FileSystemError` — When the directory cannot be removed.

### `fio::touch`

```kiwi
fio::touch(_path)
```

Create an empty file or updates the timestamp on a file.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The path to a file. |

**Returns** `boolean` — Returns `true` when the file is touched.

**Throws** `FileSystemError` — When file writing fails.

### `fio::tmpdir`

```kiwi
fio::tmpdir(): string
```

Gets the path to the temporary directory.

**Returns** `string` — Returns the temp directory path as a string.

### `fio::tmpfile`

```kiwi
fio::tmpfile(): string
```

Creates and returns an empty temporary file and returns its path.

**Returns** `string` — Returns the path to a newly created empty file.

### `fio::write`

```kiwi
fio::write(_path: string, _text = ""): boolean
```

Write text to a file. This overwrites the file if it exists.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The path to a file or a filename. |
| `_text` | `string` | `""` | The text to write. |

**Returns** `boolean` — Returns `true` when the file write succeeds.

**Throws** `FileSystemError` — When the file write fails.

### `fio::writeln`

```kiwi
fio::writeln(_path: string, _text: string|list): boolean
```

Write a line of text to a file. This always appends to a file.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The path to a file or a filename. |
| `_text` | `string\|list` |  | The text to append. |

**Returns** `boolean` — Returns `true` when the file write succeeds.

**Throws** `FileSystemError` — When the file write fails.

### `fio::writebytes`

```kiwi
fio::writebytes(_path: string, _data: bytes): integer
```

Write a list of bytes to a file. This overwrites the file if it exists.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The path to a file or a filename. |
| `_data` | `bytes` |  |  |

**Returns** `integer` — Returns the number of bytes written.

**Throws** `FileSystemError` — When the file write fails.

### `fio::writeslice`

```kiwi
fio::writeslice(_path: string, _offset: integer, _data: bytes): integer
```

Write a list of bytes to a file at an offset.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The path to a file or a filename. |
| `_offset` | `integer` |  | The zero based position to begin writing. |
| `_data` | `bytes` |  | The list of bytes to write. |

**Returns** `integer` — Returns the number of bytes written.

**Throws** `FileSystemError` — When the file write fails.

### `fio::pathparts`

```kiwi
fio::pathparts(_path: string): list
```

Get path parts from a path.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_path` | `string` |  | The path. |

**Returns** `list` — Returns a list of strings containing all path parts.
