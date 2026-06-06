---
name: kiwi-lang
description: Use when working on the Kiwi language — interpreter (src/), stdlib (lib/), .kiwi scripts, builds, tests, or language feature questions.
---

# Kiwi Language Skill

Provides specialized assistance for working with the Kiwi programming language and its .NET-based interpreter.

## When to Use

- User is developing or modifying the Kiwi interpreter (C# source in `src/`).
- User is creating or editing standard library modules in `lib/` or tests in `tests/`.
- User needs to build (`./build.sh`), test (`./bin/kiwi tests/test`), or debug Kiwi scripts.
- User asks about Kiwi syntax, semantics, package conventions, or adding built-in functions/keywords.
- User is following the AGENTS.md guide for contributions.

**Do not use** for general C# questions unrelated to Kiwi, or non-Kiwi scripting tasks.

## Workflow

### Step 1: Orient to the Project Structure
Note the split between C# interpreter (`src/`), Kiwi scripts (`lib/`, `tests/`), and docs (`docs/`). Load `references/docs-map.md` for a map of the documentation. Load `references/syntax-cheatsheet.md`, `references/types-and-builtins.md`, or `references/stdlib-catalog.md` as needed for the task.

### Step 2: Build and Test After Changes
After any C# change:
- Run `./build.sh` (or specific runtime target).
- Always run the full test suite: `./bin/kiwi tests/test` (exit code 0 = success).
- For stdlib-only: `./bin/kiwi tests/test_stdlib`.

### Step 3: Modify Language Features
- New token/keyword: Edit `src/Parsing/Lexer.cs`.
- Parser/AST: Update `src/Parsing/Parser.cs` or add node in `src/Parsing/AST/`.
- Compiler: Emit opcodes in `src/VM/Compiler.cs`.
- VM execution: Handle in `src/VM/KiwiVM.cs` (and possibly `src/VM/Opcode.cs`).
- Keyword handlers: Add to `src/Parsing/Keyword/`.

Rebuild and test after changes.

### Step 4: Add Stdlib Modules or Tests
- Create `lib/mymodule.kiwi` with `package mymodule`.
- Use `snake_case` functions, optional type hints, `/# ... #/` docstrings.
- Add corresponding tests under `tests/lib/stdlib/`.
- Follow internal helper prefix `_`.

### Step 5: Verify and Report
Run full test suite. Report pass/fail status and any errors. If adding features, ensure all existing tests still pass.

## References

- `references/syntax-cheatsheet.md` — Kiwi syntax quick reference.
- `references/types-and-builtins.md` — built-in types, operators, and functions.
- `references/stdlib-catalog.md` — standard library modules and their APIs.
- `references/docs-map.md` — map of the docs/ directory for language documentation.
