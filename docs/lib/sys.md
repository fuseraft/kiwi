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