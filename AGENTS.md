# Kiwi — Agent Guide

Kiwi is a dynamically-typed scripting language written in C# (.NET 9). The interpreter lives in `src/`, the standard library in `lib/`, and the test suite in `tests/`. Understanding this two-language split (C# interpreter + Kiwi scripts) is the foundation for working in this repo.

---

## Repository Layout

```
src/          C# interpreter (.NET 9)
lib/          Standard library (~50 Kiwi modules)
tests/        Test suite (written in Kiwi)
examples/     Example Kiwi scripts
docs/         MkDocs documentation
bench/        Benchmarks and stress tests
build.sh      Linux/macOS build script
build.ps1     Windows build script
```

---

## Build

```bash
./build.sh                 # auto-detects runtime, produces ./bin/kiwi
./build.sh linux-x64       # explicit runtime target
```

Supported runtime IDs: `linux-x64`, `linux-arm64`, `osx-x64`, `osx-arm64`, `win-x64`.

The script attempts an AOT (Ahead-of-Time) build first and falls back to a trimmed non-AOT build if AOT fails. The output binary is always `./bin/kiwi`.

**Always rebuild before running tests after changing C# source.**

---

## Testing

```bash
./bin/kiwi tests/test          # full test suite
./bin/kiwi tests/test_stdlib   # stdlib tests only
```

Exit code `0` means all tests passed; `1` means one or more failed.

Test files live under `tests/lib/suite/` (language features) and `tests/lib/stdlib/` (standard library). The runner at `tests/test.kiwi` auto-discovers them.

**Run the full test suite after any change to C# source, `lib/`, or `tests/`.**

---

## Interpreter Architecture

```
src/
  Program.cs                  Entry point; dispatches to runners
  Runtime/Runner/             Execution modes (REPL, script, debug, eval, syntax check)
  VM/                         Stack-based bytecode VM
    KiwiVM.cs                 VM execution loop
    Compiler.cs               AST → bytecode
    Opcode.cs / Chunk.cs      Instruction set
  Parsing/
    Lexer.cs                  Tokenisation
    Parser.cs / ParserCore.cs Recursive-descent parser
    AST/                      ~70 AST node types
    Keyword/                  Built-in function handlers (31 handlers)
  Runtime/
    Context.cs                Execution context
    Scope.cs / StackFrame.cs  Variable scoping and call stack
  Typing/
    Value.cs                  Runtime value wrapper
    TypeRegistry.cs           Struct/package type definitions
  Tracing/                    Error types and error handler
  Configuration/              CLI argument parsing
```

New language features typically touch: `Lexer.cs` (new token), `Parser.cs` / an AST node, `Compiler.cs` (emit opcodes), `KiwiVM.cs` (execute opcode).

---

## Standard Library Conventions (Kiwi)

- Package declaration: `package mypackage`
- Functions: `snake_case`; use `fn`/`end` blocks
- Type hints are optional: `fn foo(x: integer, y: string): boolean`
- Docstrings use `/#...#/` block comments with `@summary`, `@param`, `@return` tags
- Internal helpers are conventionally prefixed with `_`

Example:

```kiwi
package mymodule

/#
@summary Adds two integers.
@param integer a : First operand.
@param integer b : Second operand.
@return integer : The sum.
#/
fn add(a: integer, b: integer): integer
  return a + b
end
```

---

## C# Conventions

- PascalCase for classes, methods, and public properties
- `_camelCase` for private fields
- Nullable reference types enabled (`#nullable enable`)
- Prefer `switch` expressions and pattern matching
- One class per file, named identically

---

## Common Workflows

### Add a built-in function
1. Add the keyword/token in `Lexer.cs` if needed.
2. Create or update a handler in `src/Parsing/Keyword/`.
3. Add dispatch in the relevant `Dispatcher`.
4. Write tests in `tests/lib/suite/` or `tests/lib/stdlib/`.
5. Rebuild and run the test suite.

### Add a stdlib module
1. Create `lib/mymodule.kiwi` with `package mymodule`.
2. Add tests in `tests/lib/stdlib/mymodule_tests.kiwi`.
3. Run `./bin/kiwi tests/test_stdlib` to verify.

### Modify VM behaviour
1. Edit `Compiler.cs` (emit opcodes) and/or `KiwiVM.cs` (execute opcodes).
2. Add/update opcodes in `Opcode.cs` if introducing a new instruction.
3. Rebuild and run the full test suite.

---

## CI

GitHub Actions runs on push to `main` and on pull requests:
- **dotnet.yml**: `./build.sh` → `./bin/kiwi tests/test`
- **benchmarks.yml**: build → tests → benchmarks
- **docs.yml**: MkDocs deployment on merge to main

Pull requests must pass all CI checks before merging.

---

## Key Files Quick Reference

| File | Role |
|------|------|
| `src/Program.cs` | Entry point |
| `src/VM/KiwiVM.cs` | VM execution loop |
| `src/VM/Compiler.cs` | AST → bytecode |
| `src/Parsing/Lexer.cs` | Tokeniser |
| `src/Parsing/Parser.cs` | Parser |
| `src/Typing/Value.cs` | Runtime value type |
| `lib/__global__.kiwi` | Global built-in functions |
| `lib/std.kiwi` | Core stdlib utilities |
| `tests/test.kiwi` | Test runner entry point |
| `tests/lib/testsuite.kiwi` | Test harness |
