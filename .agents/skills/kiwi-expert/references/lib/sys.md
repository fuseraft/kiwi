# sys

> Run external programs and manage processes.

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

`sys` is unavailable in [safe mode](../tools/cli.md#safe-mode).

## Functions

### `sys::open`

```kiwi
sys::open(_program: string, _args: list = []): boolean
```

Open an external process without waiting for its completion.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_program` | `string` |  | The program. |
| `_args` | `list` | `[]` | The program arguments. Defaults to an empty list. |

**Returns** `boolean` — Returns `true` process start succeeds.

**Throws** `TypeError` — Expects _args to contain a list of strings.

### `sys::exec`

```kiwi
sys::exec(_program: string, _args: list = []): integer
```

Start an external process and capture its exit code.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_program` | `string` |  | The program. |
| `_args` | `list` | `[]` | The program arguments. Defaults to an empty list. |

**Returns** `integer` — Returns the exit code of the external process.

**Throws** `SystemError` — When unable to start the process.

**Throws** `TypeError` — Expects _args to contain a list of strings.

### `sys::execout`

```kiwi
sys::execout(_program: string, _args: list = []): hashmap
```

Run an external process and capture its output.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_program` | `string` |  | The program. |
| `_args` | `list` | `[]` | The program arguments. Defaults to an empty list. |

**Returns** `hashmap` —  Returns a hashmap with the following keys: - `stdout`: Data from the standard output stream as a string. - `stderr`: Data from the standard error stream as a string. - `exit_code`: The exit code of the process as an integer.

**Throws** `SystemError` — When unable to start the process.

**Throws** `TypeError` — Expects _args to contain a list of strings.

### `sys::pid`

```kiwi
sys::pid(): integer
```

Get the PID of the current Kiwi process.

**Returns** `integer` — The current process ID.

### `sys::pids`

```kiwi
sys::pids(_name: string = ""): list
```

Search for process IDs by name, or list all PIDs.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_name` | `string` | `""` | Process name to search for. Pass an empty string to return all PIDs. |

**Returns** `list` — A list of integer PIDs matching the search.

### `sys::processes`

```kiwi
sys::processes(_name: string = ""): list
```

List running processes with detailed info.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_name` | `string` | `""` | Process name to filter by. Pass an empty string to return all processes. |

**Returns** `list` —  Returns a list of hashmaps, each with the following keys: - `pid`: The process ID as an integer. - `name`: The process name as a string. - `path`: The full path to the executable as a string (empty if inaccessible). - `memory`: Working set memory usage in bytes as an integer (0 if inaccessible). - `threads`: Number of threads as an integer (0 if inaccessible). - `started`: ISO 8601 start time as a string (empty if inaccessible).

### `sys::spawn`

```kiwi
sys::spawn(_program: string, _args: list = []): integer
```

Start an external process without waiting, returning its PID.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_program` | `string` |  | The program. |
| `_args` | `list` | `[]` | The program arguments. Defaults to an empty list. |

**Returns** `integer` — The PID of the spawned process.

**Throws** `SystemError` — When unable to start the process.

**Throws** `TypeError` — Expects _args to contain a list of strings.

### `sys::kill`

```kiwi
sys::kill(_pid: integer): boolean
```

Kill a process by PID.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_pid` | `integer` |  | The PID of the process to kill. |

**Returns** `boolean` — Returns `true` if the process was killed, `false` otherwise.

### `sys::running`

```kiwi
sys::running(_pid: integer): boolean
```

Check whether a process is still running.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_pid` | `integer` |  | The PID to check. |

**Returns** `boolean` — Returns `true` if the process is running, `false` otherwise.

### `sys::wait`

```kiwi
sys::wait(_pid: integer): integer
```

Wait for a process to exit and return its exit code.

| Parameter | Type | Default | Description |
| :-- | :-- | :-- | :-- |
| `_pid` | `integer` |  | The PID of the process to wait for. |

**Returns** `integer` — The exit code of the process.

**Throws** `SystemError` — When unable to wait for the process.
