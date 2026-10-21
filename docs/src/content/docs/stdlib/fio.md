---
title: fio
description: Read, write, copy and list files and directories.
---

The `fio` package (file I/O) works with the filesystem using plain string paths. For an object-oriented interface, see [`path`](/kiwi/stdlib/path/). All `fio` functions are unavailable in [safe mode](/kiwi/tools/cli/#safe-mode).

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

```kiwi nocheck
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

<!-- api: fio -->
