---
title: Bytecode VM
description: How the Kiwi interpreter compiles and runs code, and the VM instruction set.
---

This page is for people working on the interpreter itself. Kiwi source is compiled to bytecode and executed by a stack-based virtual machine. The code lives in `src/VM/`.

## Pipeline

```text
source ──▶ Lexer ──▶ tokens ──▶ Parser ──▶ AST ──▶ Compiler ──▶ Chunk ──▶ KiwiVM
```

| Stage | Source | Output |
| :-- | :-- | :-- |
| Lexing | `src/Parsing/Lexer.cs` | A token stream |
| Parsing | `src/Parsing/Parser.cs`, `ParserCore.cs` | An abstract syntax tree (`src/Parsing/AST/`) |
| Compiling | `src/VM/Compiler.cs` | A `Chunk` of bytecode |
| Execution | `src/VM/KiwiVM.cs` | Program effects |

The standard library is parsed and compiled together with the script into a single top-level chunk, so it is loaded before the first line of user code runs.

## Chunks

A `Chunk` (`src/VM/Chunk.cs`) is the compiled form of one unit of code: the program, a function, or a lambda. Functions and lambdas get their own chunks, stored in their parent's `SubChunks`.

| Field | Contents |
| :-- | :-- |
| `Code` | The instructions. |
| `Constants` | Literal values referenced by index. |
| `Names` | Interned identifier strings referenced by index. |
| `SubChunks` | Nested function and lambda chunks. |
| `Upvalues` | Descriptors for variables captured from enclosing functions. |
| `Lines`, `Columns`, `FileIds` | Source positions for each instruction, used in error messages. |
| `LocalNames`, `LocalCount` | Local variable slots, assigned at compile time. |
| `ParamNames`, `Arity`, `MinArity`, `VariadicParamName`, `DefaultParamNames` | Parameter metadata, used to check a call's argument count (lambdas, marked `IsLambda`, are exempt). |
| `TypeChecks` | The type hints that `CheckType` instructions check: parameters, the return value and `var` initializers. |
| `ArgNameSets` | Argument names for calls that use named arguments. |
| `NodePool` | AST nodes kept for instructions that need them at runtime, such as `CallBuiltin`. |
| `IsGenerator` | Whether the function contains `yield`. |

Each instruction is a small record:

```csharp
record struct Instruction(Opcode Op, int A = 0, int B = 0);
```

`A` and `B` are operands whose meaning depends on the opcode: a constant or name index, a slot number, a jump target, an argument count, or packed flags.

## Name resolution

Inside a function, the compiler resolves each name at compile time:

1. **Local slot**: a parameter or a variable assigned in the function body.
2. **Upvalue**: a local of an enclosing function, captured by a closure.
3. **Global**: everything else, looked up at runtime with `LoadGlobal`.

At the top level every name is global.

## Execution

The VM uses one value stack of 8,192 slots shared by all frames, and a frame stack of 512 frames. Each frame (`VMFrame`) records its chunk, instruction pointer, the stack index where its locals begin (`StackBase`), its captured upvalues, the current instance (`@`) and struct (`@@`), and its active `try` handlers.

```text
StackBase + 0             parameter 0
StackBase + 1             parameter 1
StackBase + 2 ...         other locals
StackBase + LocalCount    temporaries (grow upward)
```

`RunLoop` fetches and dispatches instructions for the top frame. A call to compiled Kiwi code pushes a new frame and keeps dispatching; native builtins run in place and push their result. `Return` pops the frame and resumes the caller.

### Closures

A closure captures variables through upvalues. An upvalue starts *open*, pointing at the variable's stack slot so that the function and the closure share it. When the enclosing frame returns, its open upvalues are *closed*: the value moves into the upvalue itself, so the closure keeps working after the frame is gone. Two closures that capture the same variable share one upvalue. A `for` loop closes the loop variable's upvalue (`CloseUpvalueAt`) at the start of each iteration, so the closures created in one iteration keep that iteration's value.

### Errors

`try` blocks push a handler (`PushTryHandler`) recording the catch and finally targets and the stack height. When an error is thrown, the VM unwinds frames to the nearest handler, restores the stack height and jumps to the catch target, where `LoadCatchError` loads the error in the form the `catch` clause asks for.

A `finally` block is compiled inline on every path out of the `try` statement: after the `try` block, after the `catch` block, before a `return`, `break` or `next` that leaves it, and on an error path that keeps the error in a hidden local, runs the block and then throws the error again (`Rethrow`).

## Disassembly

Set `KIWI_DISASM=1` to print the compiled bytecode before running. Add `-ns` to leave out the standard library:

```text
$ KIWI_DISASM=1 kiwi -ns greet.kiwi
=== <main> (arity=0, locals=0, upvalues=0) ===
     0  [   1]  DefFunc            sub[0]  name="greet"
     1  [   4]  LoadGlobal         "greet"
     2  [   4]  Const                 0
     3  [   4]  Call               argc=1
     4  [   0]  Pop
     5  [   0]  Halt

  === greet (arity=1, locals=1, upvalues=0) ===
    params: name
       0  [   2]  Const                 0
       1  [   2]  LoadLocal          slot=0
       2  [   2]  Const                 1
       3  [   2]  Interpolate        parts=3
       4  [   2]  Print              newline=True  stderr=False
       5  [   0]  ReturnNull
```

The bracketed number is the source line.

## Instruction set

The tables below are generated from the doc comments in `src/VM/Opcode.cs`. `A` and `B` are the instruction operands.

<!-- opcodes -->

## Adding a language feature

A new construct usually touches each stage in turn:

1. **Lexer**: add a token in `Lexer.cs` if the syntax needs one.
2. **Parser**: add an AST node in `src/Parsing/AST/` and parse it in `Parser.cs`.
3. **Compiler**: emit instructions for the node in `Compiler.cs`.
4. **VM**: if no existing opcode fits, add one to `Opcode.cs`, with a `<summary>` comment so it appears on this page, and handle it in `KiwiVM.cs`.
5. **Tests**: add tests under `tests/lib/suite/`, rebuild with `./build.sh`, and run `./bin/kiwi tests/test`.
