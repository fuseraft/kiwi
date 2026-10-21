---
title: path
description: The Path struct, an object-oriented interface to filesystem paths.
---

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

For writing files, use [`fio`](/kiwi/stdlib/fio/) with `path.to_string()`.

<!-- api: path -->
