# Runners

Kiwi supports **multiple execution modes** via the `IRunner` interface. Each mode is designed for a specific workflow — from running scripts to debugging syntax to interactive development.

---

## Execution Modes

| Runner | Purpose | Input | Example |
|--------|--------|-------|--------|
| `ScriptRunner` | Run `.kiwi` files | File path | `kiwi script.kiwi` |
| `CodeRunner` | Execute a code string inline | `-e` flag | `kiwi -e 'println "hi"'` |
| `StdInRunner` | Run from piped input | `stdin` | `cat script.kiwi &#124; kiwi` |
| `REPLRunner` | Interactive shell | Keyboard | `kiwi --interactive` |
| `DebugRunner` | Step-debug a script (kdb) | File | `kiwi --debug script.kiwi` |
| `ASTPrinter` | Print AST for debugging | File | `kiwi --ast script.kiwi` |
| `TokenPrinter` | Print tokens for debugging | File | `kiwi --tokens script.kiwi` |

## How They Work

All runners follow the same core pipeline:

1. **Load Standard Library** (once, unless debugging)
2. **Lex -> Parse -> Interpret**
3. **Handle errors gracefully**
4. **Pass CLI args** to `Interpreter.CliArgs`

## `ScriptRunner` – Run a File

**Use Case**: Production scripts, CLI tools, automation.

```bash
kiwi examples/hello.kiwi
```

- Loads the [`standard library`](./lib/README.md) first
- Then loads `hello.kiwi`
- Executes top-level code and `main()` if defined

## `CodeRunner` – Execute Inline Code

**Use Case**: Quick one-liners, shell aliases, scripted invocations without a file.

```bash
kiwi --execute 'println("Hello!")'   # long form
kiwi -e 'println("Hello!")'          # short form
```

- Accepts a **string of Kiwi code** as the argument
- Loads the **standard library** (use `-ns` to skip)
- Supports multi-line code via shell `$'...'` quoting:

```bash
kiwi -e $'x = 6 * 7\nprintln(x)'
# 42
```

- Can be combined with key-value args:

```bash
kiwi -e 'println(__argv__)' -name=world
```

## `StdInRunner` – Run from Pipe

**Use Case**: Shell pipelines, filters, one-liners.

```bash
echo "println 'Hello from pipe'" | kiwi
# Hello from pipe

find . -name "*.kiwi" | xargs -I {} sh -c "echo '--- {} ---'; kiwi {}"
```

- Reads **all input** from `stdin`
- **No file path needed**
- **40 MB limit** (configurable)
- **Does not close stdin** — safe in pipelines

## `REPLRunner` – Interactive Mode

**Use Case**: Learning, debugging, rapid prototyping.

```bash
kiwi --interactive   # long form
kiwi -i              # short form
```

```text
> println 1 + 2
3
> for i in [1, 2, 3] do \
>   println i * 2 \
> end
2
4
6
> .exit
```

### Features
- **Multi-line input** with `\` continuation
- **Immediate execution**
- **`.exit`** to quit
- Full access to the standard library and CLI args

## `ASTPrinter` – Debug the Parser

**Use Case**: Understand how code is parsed.

```bash
kiwi --ast hello.kiwi   # long form
kiwi -a hello.kiwi      # short form
```

**Sample Output**:
```text
Generating AST: hello.kiwi

Program:
  PrintLine:
    Literal: "Hello, World!"
```

## `DebugRunner` – Interactive Debugger (`kdb`)

**Use Case**: Step through code, inspect variables, set breakpoints.

```bash
kiwi --debug script.kiwi   # long form
kiwi -d script.kiwi        # short form
```

Launches the `kdb` interactive debugger. Starts paused at the first statement.

```text
kdb - Kiwi Debugger
Debugging: script.kiwi
Type 'h' for help, 's' to step, 'r' to run.

=> script.kiwi:3
     1: fn greet(name)
     2:   println "Hello, ${name}!"
  => 3:   greet("world")
(kdb)
```

### Commands

| Command | Alias | Description |
|---------|-------|-------------|
| `run` | `r` | Run to next breakpoint (or end) |
| `step` | `s` | Step into next statement |
| `next` | `n` | Step over (skip into function calls) |
| `finish` | `f` | Step out of current function |
| `b <line>` | | Set breakpoint at line in current file |
| `b <file>:<line>` | | Set breakpoint at file:line |
| `d <n>` | | Delete breakpoint #n |
| `info b` | | List all breakpoints |
| `p <expr>` | | Evaluate and print an expression |
| `locals` | `l` | Show local variables in current scope |
| `backtrace` | `bt` | Show call stack |
| `list [line]` | | Show source around current or given line |
| `help` | `h` | Show command help |
| `quit` | `q` | Exit debugger |

Standard library files are automatically skipped — the debugger only pauses in user code.

---

## `TokenPrinter` – Debug the Lexer

**Use Case**: Fix syntax errors, learn tokenization.

```bash
kiwi --tokens hello.kiwi   # long form
kiwi -t hello.kiwi         # short form
```

**Sample Output**:
```text
Tokenizing: hello.kiwi

Token #               Type  Name                 Text                
-------               ----  ----                 ----                
1                  Keyword  KW_PrintLn           println             
2                   String  Default              Hello, World!       
3                      Eof  Default                               
```

## Standard Library

- **Loaded automatically** in `ScriptRunner`, `CodeRunner`, `StdInRunner`, and `REPLRunner`
- **Skipped** in `--ast` and `--tokens`
- Configured in [`kiwi-settings.json`](../src/kiwi-settings.json)
- **Last file wins** (for overrides)

## Error Handling

All runners catch:
- `KiwiError` → Pretty-print error type, message, and file/line to stderr
- `Exception` → Print message and stack trace to stderr

Crash dump logging is **opt-in** via the `-cd`/`--crash-dump` flag (or by setting `"crashdump_path"` in `kiwi-settings.json`). When enabled, errors are also appended to the configured log file.

## Quick Testing

```bash
# Run a script
kiwi test.kiwi

# Execute inline code
kiwi -e 'println("hello")'

# Pipe input
echo "println 100" | kiwi

# Interactive
kiwi -i

# Step debugger
kiwi -d test.kiwi

# Debug parser
kiwi -a test.kiwi

# Debug lexer
kiwi -t test.kiwi
```