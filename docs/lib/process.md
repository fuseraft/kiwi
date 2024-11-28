# `process`

The `process` package provides utilities for interacting with system processes in Kiwi. It allows you to retrieve process information, check for the existence of processes, and manage processes by name or PID (Process ID).

---

## Table of Contents

- [Package Overview](#package-overview)
- [Function Details](#function-details)
  - [`get_proc_name`](#get_proc_name)
  - [`get_pid`](#get_pid)
  - [`get_procs`](#get_procs)
  - [`is_proc`](#is_proc)
  - [`kill`](#kill)
- [Example Usage](#example-usage)
- [Important Notes](#important-notes)

---

## Package Overview

The `process` package includes functions to:

- **Retrieve the name of a process** given its `/proc` path.
- **Get the PID of a process** by its name.
- **List all running processes** with their PIDs and names.
- **Check if a process exists** by name or PID.
- **Kill a process** by name or PID.

---

## Function Details

### `get_proc_name`

Retrieves the process name from the `/proc/[pid]/comm` file.

#### **Syntax:**
```kiwi
process::get_proc_name(proc_path: string): string
```

#### **Parameters:**

- **`proc_path`**: A string representing the path to the process directory in `/proc`.
  - **Example**: `"/proc/1234"`

#### **Returns:**

- **`string`**: The name of the process.
  - If the process name cannot be read, returns an empty string.

#### **Example:**
```kiwi
# Get the name of the process with PID 1234
proc_name = process::get_proc_name("/proc/1234")
println("Process name: ${proc_name}")
```

---

### `get_pid`

Returns the PID of the first process found with a given process name.

#### **Syntax:**
```kiwi
process::get_pid(proc_name: string): integer or null
```

#### **Parameters:**

- **`proc_name`**: The name of the process to search for.
  - **Example**: `"bash"`

#### **Returns:**

- **`integer`**: The PID of the first process that matches the given name.
- **`null`**: If no process with the given name is found.

#### **Example:**
```kiwi
# Get the PID of the 'bash' process
pid = process::get_pid("bash")
if pid != null
  println("PID of 'bash': ${pid}")
else
  println("'bash' process not found.")
end
```

---

### `get_procs`

Returns a list of hashmaps containing `pid` and `name` for all running processes.

#### **Syntax:**
```kiwi
process::get_procs(): list
```

#### **Parameters:**

- None

#### **Returns:**

- **`list`**: A list of hashmaps where each hashmap contains:
  - **`"pid"`**: The PID of the process (integer).
  - **`"name"`**: The name of the process (string).

#### **Example:**
```kiwi
# Get a list of all running processes
processes = process::get_procs()
for proc in processes do
  println("PID: ${proc["pid"]}, Name: ${proc["name"]}")
end
```

---

### `is_proc`

Checks if a process exists by name or PID.

#### **Syntax:**
```kiwi
process::is_proc(proc: any): boolean
```

#### **Parameters:**

- **`proc`**: The name (string) or PID (integer) of the process to check.
  - **Examples**:
    - Process name: `"bash"`
    - PID: `1234`

#### **Returns:**

- **`boolean`**: `true` if the process exists; `false` otherwise.

#### **Example:**
```kiwi
# Check if a process named 'bash' exists
exists = process::is_proc("bash")
println("Does 'bash' exist? ${exists}")
```

---

### `kill`

Kills a process by name or PID.

#### **Syntax:**
```kiwi
process::kill(proc: any)
```

#### **Parameters:**

- **`proc`**: The name (string) or PID (integer) of the process to kill.
  - **Examples**:
    - Process name: `"bash"`
    - PID: `1234`

#### **Returns:**

- None

#### **Example:**
```kiwi
# Kill the process named 'my_app'
process::kill("my_app")
println("'my_app' has been terminated.")
```

---

## Example Usage

### Listing All Processes

```kiwi
fn main()
  processes = process::get_procs()
  println "PID\tNAME"
  for proc in processes do
    println "${proc.pid}\t${proc.name}"
  end
end

main()
```

---

### Checking for a Process and Killing It

```kiwi
fn main()
  proc_name = "my_app"
  if process::is_proc(proc_name)
    pid = process::get_pid(proc_name)
    println("Process '${proc_name}' is running with PID ${pid}. Terminating it...")
    process::kill(proc_name)
    println("Process '${proc_name}' has been terminated.")
  else
    println("Process '${proc_name}' is not running.")
  end
end

main()
```

---

### Getting the Name of a Specific Process by PID

```kiwi
fn main()
  pid = 1234
  proc_path = "/proc/${pid}"
  if fs::exists(proc_path)
    proc_name = process::get_proc_name(proc_path)
    println("Process with PID ${pid} is '${proc_name}'.")
  else
    println("Process with PID ${pid} does not exist.")
  end
end

main()
```

---

## Important Notes

1. **Platform Dependency**: The `process` package relies on the `/proc` filesystem, which is specific to Unix-like operating systems (e.g., Linux). It may not work on other systems like Windows or macOS.

2. **Permissions**: Accessing process information and sending signals may require appropriate permissions. For instance, killing a process that you do not own typically requires superuser privileges.

3. **Error Handling**: Always ensure that the process exists before attempting operations like killing it to avoid exceptions.

4. **Process Names**: Multiple processes may have the same name. The functions like `get_pid` return the first match. If you need to handle multiple processes with the same name, consider using `get_procs` and filtering the results.

5. **Signal Handling**: The `kill` function uses `signal::SIGKILL` to terminate processes forcefully. Use this cautiously, as it does not allow the process to perform any cleanup.
