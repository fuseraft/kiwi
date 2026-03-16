# `sys`

The `sys` package provides simple, cross-platform ways to **launch external processes** from Kiwi code.  
It offers three main functions with different levels of control and output capture:

- `open`: fire-and-forget (non-blocking, no output capture)
- `exec`: blocking, returns only the exit code
- `execout`: blocking, captures stdout, stderr, and exit code

These functions are the foundation for running shell commands, external tools, scripts, or desktop applications from Kiwi.

---

## Package Functions

### `open(_program, _args)`

Launches an external program **without waiting** for it to finish and **without capturing** any output.

Most useful for:

- Opening files with their default application (`sys::open("document.pdf")`)
- Starting GUI tools or background daemons
- Launching editors, browsers, media players, etc.

**Parameters**

| Type     | Name       | Description                              | Default |
|----------|------------|------------------------------------------|---------|
| `string` | `_program` | Executable name or full path             | —       |
| `list`   | `_args`    | List of command-line arguments (strings) | `[]`    |

**Returns**  

| Type      | Description                              |
|-----------|------------------------------------------|
| `boolean` | `true` if the process was successfully started, and `false` if for any reason it fails to start |

**Throws**  
- `TypeError` — if `_args` is not a list of strings

**Example**
```kiwi
# Open Kiwi in VS Code
const KIWI_PATH = env::bin().to_path().parent().find('kiwi').path
sys::open("code", [KIWI_PATH, "--new-window"])

# Open our favorite repository in Firefox (a superior browser).
sys::open("firefox", ["https://github.com/fuseraft/kiwi"])

# Windows: open notepad.exe
sys::open("start", ["notepad.exe"])
```

### `exec(_program, _args)`

Runs an external program **synchronously** (blocks until it exits) and returns only the **exit code**.

Ideal when you only care whether the command succeeded (exit code 0) or failed.

**Parameters**

| Type     | Name       | Description                              | Default |
|----------|------------|------------------------------------------|---------|
| `string` | `_program` | Executable name or full path             | —       |
| `list`   | `_args`    | List of command-line arguments (strings) | `[]`    |

**Returns**  

| Type      | Description                  |
|-----------|------------------------------|
| `integer` | Process exit code (0 = success) |

**Throws**  
- `TypeError`: if `_args` is not a list of strings
- `SystemError`: if the process cannot be started

**Example**
```kiwi
exit_code = sys::exec("git", ["status", "--porcelain"])
if exit_code != 0
  println "Git command failed with code ${exit_code}"
end
```

### `execout(_program, _args)`

Runs an external program **synchronously** and captures **both stdout and stderr** as strings, along with the exit code.

This is the most commonly used function when you need to:

- Read command output
- Parse results from tools (git, ffmpeg, curl, etc.)
- Handle both normal output and errors

**Parameters**

| Type     | Name       | Description                              | Default |
|----------|------------|------------------------------------------|---------|
| `string` | `_program` | Executable name or full path             | —       |
| `list`   | `_args`    | List of command-line arguments (strings) | `[]`    |

**Returns**  

| Type      | Description                                                                 |
|-----------|-----------------------------------------------------------------------------|
| `hashmap` | `{ "stdout": string, "stderr": string, "exit_code": integer }`             |

**Throws**  
- `TypeError`: if `_args` is not a list of strings
- `SystemError`: if the process cannot be started

**Example**
```kiwi
result = sys::execout("curl", [
  "-X", "POST",
  "-H", "Content-Type: application/json",
  "-d", '{"name": "Kiwi", "version": "1.4.0"}',
  "https://httpbin.org/post"
])

if result.exit_code == 0
  println "Files:\n${result.stdout}"
else
  println "Error: ${result.stderr} (code ${result.exit_code})"
end

# One-liner example: get current git branch
branch = sys::execout("git", ["rev-parse", "--abbrev-ref", "HEAD"]).stdout.trim()
println "We are in ${branch}"
```

---

## Process Management

### `pid()`

Returns the PID of the current Kiwi process.

**Returns**

| Type      | Description         |
|-----------|---------------------|
| `integer` | Current process ID  |

**Example**
```kiwi
println "My PID: ${sys::pid()}"
```

---

### `pids(_name)`

Returns a list of PIDs for processes matching `_name`. Pass an empty string (or omit the argument) to get all running PIDs.

**Parameters**

| Type     | Name    | Description                                          | Default |
|----------|---------|------------------------------------------------------|---------|
| `string` | `_name` | Process name to search for. `""` returns all PIDs.  | `""`    |

**Returns**

| Type   | Description                        |
|--------|------------------------------------|
| `list` | List of integer PIDs               |

**Example**
```kiwi
# All PIDs on the system
all = sys::pids()
println "Running processes: ${all.size()}"

# Find processes by name
pids = sys::pids("python3")
println "Python3 PIDs: ${pids}"

# Kill all matching processes
for pid in pids
  sys::kill(pid)
end
```

---

### `processes(_name)`

Returns a list of hashmaps describing running processes. Pass an empty string (or omit the argument) to get all processes.

**Parameters**

| Type     | Name    | Description                                               | Default |
|----------|---------|-----------------------------------------------------------|---------|
| `string` | `_name` | Process name to filter by. `""` returns all processes.   | `""`    |

**Returns**

| Type   | Description                        |
|--------|------------------------------------|
| `list` | List of hashmaps, one per process  |

Each hashmap contains:

| Key        | Type      | Description                                              |
|------------|-----------|----------------------------------------------------------|
| `pid`      | `integer` | Process ID                                               |
| `name`     | `string`  | Process name                                             |
| `path`     | `string`  | Full path to the executable (`""` if inaccessible)       |
| `memory`   | `integer` | Working set memory in bytes (`0` if inaccessible)        |
| `threads`  | `integer` | Number of threads (`0` if inaccessible)                  |
| `started`  | `string`  | ISO 8601 start time (`""` if inaccessible)               |

**Example**
```kiwi
# Inspect a specific process
for p in sys::processes("python3") do
  pid     = p["pid"]
  name    = p["name"]
  mem_mb  = p["memory"] / 1024 / 1024
  threads = p["threads"]
  path    = p["path"]
  println "${name} (${pid})  mem=${mem_mb}MB  threads=${threads}  path=${path}"
end

# Find the most memory-hungry process
all = sys::processes()
top = all.sort(with (a, b) do a["memory"] < b["memory"] end).last()
println "Biggest: ${top["name"]} (${top["pid"]}) — ${top["memory"] / 1024 / 1024}MB"
```

---

### `spawn(_program, _args)`

Starts an external process **without waiting** and returns its PID. Unlike `open`, `spawn` registers the process so it can be managed with `wait`, `kill`, and `running`.

**Parameters**

| Type     | Name       | Description                              | Default |
|----------|------------|------------------------------------------|---------|
| `string` | `_program` | Executable name or full path             | —       |
| `list`   | `_args`    | List of command-line arguments (strings) | `[]`    |

**Returns**

| Type      | Description              |
|-----------|--------------------------|
| `integer` | PID of the spawned process |

**Throws**
- `TypeError` — if `_args` is not a list of strings
- `SystemError` — if the process cannot be started

**Example**
```kiwi
pid = sys::spawn("python3", ["server.py"])
println "Server started with PID: ${pid}"
```

---

### `kill(_pid)`

Kills a process by PID and waits for it to exit. Works for both processes started with `spawn` and arbitrary system processes.

**Parameters**

| Type      | Name   | Description             |
|-----------|--------|-------------------------|
| `integer` | `_pid` | PID of process to kill  |

**Returns**

| Type      | Description                                |
|-----------|--------------------------------------------|
| `boolean` | `true` if killed successfully, `false` otherwise |

**Example**
```kiwi
pid = sys::spawn("sleep", ["60"])
sys::kill(pid)
println "Process stopped."
```

---

### `running(_pid)`

Checks whether a process is currently running.

**Parameters**

| Type      | Name   | Description      |
|-----------|--------|------------------|
| `integer` | `_pid` | PID to check     |

**Returns**

| Type      | Description                                         |
|-----------|-----------------------------------------------------|
| `boolean` | `true` if the process is running, `false` otherwise |

**Example**
```kiwi
pid = sys::spawn("sleep", ["5"])

while sys::running(pid)
  println "Still running..."
  task::sleep(500)
end

println "Done."
```

---

### `wait(_pid)`

Blocks until a process exits and returns its exit code. Works best with processes started via `spawn`.

**Parameters**

| Type      | Name   | Description          |
|-----------|--------|----------------------|
| `integer` | `_pid` | PID to wait for      |

**Returns**

| Type      | Description               |
|-----------|---------------------------|
| `integer` | Exit code of the process  |

**Throws**
- `SystemError` — if the process cannot be found or waited on

**Example**
```kiwi
pid = sys::spawn("make", ["build"])
code = sys::wait(pid)

if code == 0
  println "Build succeeded."
else
  println "Build failed with exit code ${code}."
end
```

---

## Important Notes

- All three functions expect arguments as a **list of strings**. No automatic shell parsing or quoting is performed.
- No shell is involved by default (`cmd.exe`, `bash`, `/bin/sh` etc. are **not** used unless you explicitly pass them as `_program`).
- To run shell commands with pipes/redirection, you must invoke the shell yourself:

```kiwi
# Run a shell command with pipes
sys::execout("bash", ["-c", "git log --oneline | head -n 5"])
```

- On Windows, you may need to use `"cmd.exe"` or full paths to executables.
- `open()` usually starts the process detached (no waiting), but behavior depends on OS and program type.