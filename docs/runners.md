# Runners

Kiwi supports **multiple execution modes** via the `IRunner` interface. Each mode is designed for a specific workflow — from running scripts to debugging syntax to interactive development.

## Execution Modes

| Runner | Purpose | Input | Example |
|--------|--------|-------|--------|
| `ScriptRunner` | Run `.kiwi` files | File path | `kiwi script.kiwi` |
| `StdInRunner` | Run from piped input | `stdin` | `cat script.kiwi \| kiwi` |
| `REPLRunner` | Interactive shell | Keyboard | `kiwi --interactive` |
| `ASTPrinter` | Print AST for debugging | File | `kiwi --ast script.kiwi` |
| `TokenPrinter` | Print tokens for debugging | File | `kiwi --tokenize script.kiwi` |

## How They Work

All runners follow the same core pipeline:

1. **Load Standard Library** (once, unless debugging)
2. **Lex → Parse → Interpret**
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
kiwi --interactive
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
kiwi --ast hello.kiwi
```

**Sample Output**:
```text
Generating AST: hello.kiwi

Program:
  PrintLine:
    Literal: "Hello, World!"
```

## `TokenPrinter` – Debug the Lexer

**Use Case**: Fix syntax errors, learn tokenization.

```bash
kiwi --tokenize hello.kiwi
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

- **Loaded automatically** in `ScriptRunner`, `StdInRunner`, and `REPLRunner`
- **Skipped** in `--ast` and `--tokenize`
- Configured in [`kiwi-settings.json`](../src/kiwi-settings.json)
- **Last file wins** (for overrides)

## Error Handling

All runners catch:
- `KiwiError` → Pretty print with file/line
- `Exception` → Full crash log + stack trace

## Quick Testing

```bash
# Run a script
kiwi test.kiwi

# Pipe input
echo "println 100" | kiwi

# Interactive
kiwi --interactive

# Debug parser
kiwi --ast test.kiwi

# Debug lexer
kiwi --tokenize test.kiwi
```