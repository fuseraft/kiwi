# `signal`

The `signal` package provides functionality for handling system signals in Kiwi. It allows you to send signals to processes, raise signals within the current process, and trap signals with custom handlers.

---

## Table of Contents

- [Package Overview](#package-overview)
- [Constants](#constants)
  - [Signal Constants](#signal-constants)
- [Function Details](#function-details)
  - [`send`](#send)
  - [`raise`](#raise)
  - [`trap`](#trap)
- [Example Usage](#example-usage)
  - [Sending a Signal to Another Process](#sending-a-signal-to-another-process)
  - [Raising a Signal in the Current Process](#raising-a-signal-in-the-current-process)
  - [Trapping a Signal with a Custom Handler](#trapping-a-signal-with-a-custom-handler)
- [Important Notes](#important-notes)

---

## Package Overview

The `signal` package includes:

- **Constants** representing common signal numbers used in Unix-like operating systems.
- **Functions** to send signals to processes, raise signals, and trap signals with custom callback handlers.

---

## Constants

### Signal Constants

The following constants represent standard signal numbers:

- **`SIGHUP`**: Hangup detected on controlling terminal or death of controlling process.
  ```kiwi
  const signal::SIGHUP = 1
  ```
- **`SIGINT`**: Interrupt from keyboard (Ctrl+C).
  ```kiwi
  const signal::SIGINT = 2
  ```
- **`SIGQUIT`**: Quit from keyboard.
  ```kiwi
  const signal::SIGQUIT = 3
  ```
- **`SIGILL`**: Illegal instruction.
  ```kiwi
  const signal::SIGILL = 4
  ```
- **`SIGABRT`**: Abort signal from `abort` function.
  ```kiwi
  const signal::SIGABRT = 6
  ```
- **`SIGFPE`**: Floating-point exception.
  ```kiwi
  const signal::SIGFPE = 8
  ```
- **`SIGKILL`**: Kill signal. Cannot be caught or ignored.
  ```kiwi
  const signal::SIGKILL = 9
  ```
- **`SIGSEGV`**: Invalid memory reference.
  ```kiwi
  const signal::SIGSEGV = 11
  ```
- **`SIGPIPE`**: Broken pipe: write to pipe with no readers.
  ```kiwi
  const signal::SIGPIPE = 13
  ```
- **`SIGALRM`**: Timer signal from `alarm` function.
  ```kiwi
  const signal::SIGALRM = 14
  ```
- **`SIGTERM`**: Termination signal.
  ```kiwi
  const signal::SIGTERM = 15
  ```
- **`SIGUSR1`**: User-defined signal 1.
  ```kiwi
  const signal::SIGUSR1 = 10
  ```
- **`SIGUSR2`**: User-defined signal 2.
  ```kiwi
  const signal::SIGUSR2 = 12
  ```
- **`SIGCHLD`**: Child stopped or terminated.
  ```kiwi
  const signal::SIGCHLD = 17
  ```
- **`SIGCONT`**: Continue if stopped.
  ```kiwi
  const signal::SIGCONT = 18
  ```
- **`SIGSTOP`**: Stop process. Cannot be caught or ignored.
  ```kiwi
  const signal::SIGSTOP = 19
  ```
- **`SIGTSTP`**: Stop typed at terminal.
  ```kiwi
  const signal::SIGTSTP = 20
  ```
- **`SIGTTIN`**: Terminal input for background process.
  ```kiwi
  const signal::SIGTTIN = 21
  ```
- **`SIGTTOU`**: Terminal output for background process.
  ```kiwi
  const signal::SIGTTOU = 22
  ```
- **`SIGBUS`**: Bus error (bad memory access).
  ```kiwi
  const signal::SIGBUS = 7
  ```
- **`SIGTRAP`**: Trace/breakpoint trap.
  ```kiwi
  const signal::SIGTRAP = 5
  ```
- **`SIGURG`**: Urgent condition on socket.
  ```kiwi
  const signal::SIGURG = 23
  ```
- **`SIGXCPU`**: CPU time limit exceeded.
  ```kiwi
  const signal::SIGXCPU = 24
  ```
- **`SIGXFSZ`**: File size limit exceeded.
  ```kiwi
  const signal::SIGXFSZ = 25
  ```
- **`SIGVTALRM`**: Virtual alarm clock.
  ```kiwi
  const signal::SIGVTALRM = 26
  ```
- **`SIGPROF`**: Profiling timer expired.
  ```kiwi
  const signal::SIGPROF = 27
  ```
- **`SIGWINCH`**: Window size change.
  ```kiwi
  const signal::SIGWINCH = 28
  ```
- **`SIGSYS`**: Bad system call.
  ```kiwi
  const signal::SIGSYS = 31
  ```

---

## Function Details

### `send`

Sends a signal to a process specified by its PID.

#### **Syntax:**
```kiwi
signal::send(pid: integer, signum: integer)
```

#### **Parameters:**

- **`pid`**: The Process ID of the target process.
  - **Example**: `1234`
- **`signum`**: The signal number to send.
  - **Example**: `signal::SIGTERM`

#### **Returns:**

- **None**

#### **Example:**
```kiwi
# Send SIGTERM to process with PID 1234
signal::send(1234, signal::SIGTERM)
```

---

### `raise`

Raises a signal within the current process.

#### **Syntax:**
```kiwi
signal::raise(signum: integer): integer
```

#### **Parameters:**

- **`signum`**: The signal number to raise.
  - **Example**: `signal::SIGINT`

#### **Returns:**

- **`integer`**: Returns `0` on success, or an error code on failure.

#### **Example:**
```kiwi
# Raise SIGINT in the current process
signal::raise(signal::SIGINT)
```

---

### `trap`

Traps a signal and assigns a custom callback handler to it.

#### **Syntax:**
```kiwi
signal::trap(signum: integer, callback: lambda): integer
```

#### **Parameters:**

- **`signum`**: The signal number to trap.
  - **Example**: `signal::SIGINT`
- **`callback`**: A lambda function that will be called when the signal is received.
  - **Example**: `with do println("Signal received") end`

#### **Returns:**

- **`integer`**: Returns `0` on success, or raises an error if trapping is not possible.

#### **Exceptions:**

- Raises an error if attempting to trap uncatchable signals like `SIGKILL` or `SIGSTOP`.

#### **Example:**
```kiwi
# Trap SIGINT and assign a custom handler
signal::trap(signal::SIGINT, with do
  println("Interrupt signal caught. Exiting gracefully...")
  std::exit(0)
end)
```

---

## Example Usage

### Sending a Signal to Another Process

```kiwi
fn main()
  # PID of the target process
  target_pid = 1234
  
  # Send SIGTERM to gracefully request termination
  signal::send(target_pid, signal::SIGTERM)
  
  println("Sent SIGTERM to process ${target_pid}")
end

main()
```

---

### Raising a Signal in the Current Process

```kiwi
fn main()
  println("Raising SIGUSR1 in the current process")
  
  signal::raise(signal::SIGUSR1)
  
  println("This message may not be printed if the signal terminates the process")
end

main()
```

---

### Trapping a Signal with a Custom Handler

```kiwi
fn main()
  # Trap SIGINT (Ctrl+C) to perform a custom action
  signal::trap(signal::SIGINT, with do
    println("SIGINT caught! Cleaning up resources...")
    # Perform cleanup actions here
    exit(0)
  end)
  
  println("Press Ctrl+C to trigger the signal handler")
  
  # Infinite loop to keep the program running
  while true do
    task::sleep(10)
  end
end

main()
```

---

## Important Notes

1. **Uncatchable Signals**: Certain signals like `SIGKILL` and `SIGSTOP` cannot be caught, blocked, or ignored. Attempting to trap these signals will result in an error.

2. **Permissions**: Sending signals to processes may require appropriate permissions. For instance, sending signals to processes owned by other users typically requires superuser privileges.

3. **Signal Effects**: Be cautious when sending signals, as they can terminate processes or cause unexpected behavior. Ensure that you understand the effect of the signal you are sending.

4. **Platform Dependency**: The `signal` package is designed for Unix-like operating systems. Signal numbers and behavior may vary across different systems.

5. **Error Handling**: Functions like `signal::send` and `signal::trap` may raise exceptions on failure. Use appropriate error handling mechanisms to catch and manage these exceptions.

6. **Signal Handling in Multi-threaded Programs**: Be aware that signal handling in multi-threaded programs can be complex. Signals are delivered to specific threads, and care must be taken to ensure thread safety.
