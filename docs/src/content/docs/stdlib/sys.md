---
title: sys
description: Run external programs and manage processes.
---

The `sys` package starts external programs and inspects processes. Arguments are passed as a list, not through a shell, so they don't need quoting.

```kiwi
result = sys::execout("echo", ["hello", "world"])
println result["stdout"].trim()   # hello world
println result["exit_code"]       # 0

code = sys::exec("sh", ["-c", "exit 3"])
println code                      # 3
```

| Function | Does |
| :-- | :-- |
| `exec(program, args)` | Runs a program, waits, and returns its exit code. Its output goes to the terminal. |
| `execout(program, args)` | Runs a program, waits, and returns `stdout`, `stderr` and `exit_code`. |
| `spawn(program, args)` | Starts a program in the background and returns its process id. |
| `wait(pid)` | Waits for a process to exit and returns its exit code. |
| `running(pid)`, `kill(pid)` | Checks or stops a process. |
| `open(program, args)` | Starts a program without waiting for it; returns whether it started. |
| `pid()` | The current process id. |
| `pids(name)`, `processes(name)` | Running process ids, or details, optionally filtered by name. |

```kiwi
pid = sys::spawn("sleep", ["0.2"])
println sys::running(pid)   # true
sys::wait(pid)
println sys::running(pid)   # false
```

To run a command line that uses shell features such as pipes or globbing, run the shell explicitly: `sys::execout("sh", ["-c", "ls *.kiwi | wc -l"])`.

`sys` is unavailable in [safe mode](/kiwi/tools/cli/#safe-mode).

<!-- api: sys -->
