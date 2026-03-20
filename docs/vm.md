# Kiwi Bytecode VM

The Kiwi VM is a stack-based bytecode virtual machine. It is the default execution engine — `kiwi script.kiwi` compiles and runs via the VM. The tree-walking interpreter is available via `--tree-walker` for compatibility.

---

## Overview

Execution follows this pipeline:

```
Source file
    │
    ▼
Lexer  ──→  TokenStream
    │
    ▼
Parser ──→  AST (ProgramNode)
    │
    ▼
Compiler ──→  Chunk (bytecode)
    │
    ▼
KiwiVM.Execute(chunk)
    │
    ▼
RunLoop() — opcode dispatch
```

The standard library is parsed and compiled together with the user script into a single `Chunk`. There is no separate stdlib load step at runtime.

---

## Chunk

A `Chunk` is the compiled representation of one unit of code — the top-level program, a function, or a lambda. Each function and lambda gets its own nested `Chunk` stored in `SubChunks`.

```
Chunk
├── Code:          List<Instruction>        — bytecode stream
├── Constants:     List<Value>              — literal values (integers, floats, strings)
├── Names:         List<string>             — interned name pool (variables, methods)
├── SubChunks:     List<Chunk>              — nested functions / lambdas
├── Upvalues:      List<UpvalueDescriptor>  — closure capture descriptors
├── NodePool:      List<ASTNode>            — AST nodes for InterpFallback / CallBuiltin
├── ArgNameSets:   List<string[]>           — named-argument metadata
├── LocalNames:    List<(string, int)>      — pre-scanned local → slot mappings
├── Arity:         int                      — explicit parameter count
├── LocalCount:    int                      — total local variable slots
├── ParamNames:    List<string>
├── VariadicParamName: string               — name of *args parameter, if any
├── DefaultParamNames: HashSet<string>      — params with default expressions
└── IsGenerator:   bool                     — true if body contains Yield
```

Each instruction is a 12-byte struct:

```csharp
record struct Instruction(Opcode Op, int A = 0, int B = 0);
```

`A` and `B` are general-purpose operands whose meaning depends on the opcode (constant index, slot number, jump target, argument count, name index, etc.).

---

## Compiler

### Entry Points

| Method | Purpose |
|--------|---------|
| `Compiler.CompileProgram(ProgramNode)` | Compiles the full program; emits `Halt` at end |
| `Compiler.CompileFunction(FunctionNode, enclosing)` | Compiles a named function |
| `Compiler.CompileLambda(LambdaNode, enclosing)` | Compiles an anonymous lambda |

### Variable Resolution

Inside a function, name resolution happens at compile time in this order:

1. **Local slot** — any variable assigned anywhere in the function body (pre-scanned)
2. **Upvalue** — a local from an enclosing function, captured into a closure cell
3. **Global** — everything else; resolved at runtime via `LoadGlobal`

At top level, every name resolves as global.

### Statement Dispatch

`CompileStatement` dispatches each AST node type to a dedicated compile method. Nodes that produce a value on the stack return `true`; statement-only nodes return `false`. If a value-producing node appears in statement position, a `Pop` is emitted to discard the result.

Constructs not yet compiled natively fall through to `Fallback(node)`, which stores the AST node in `NodePool` and emits `InterpFallback`. This is intentional — the tree-walker handles those cases correctly and the result is pushed onto the VM stack.

---

## Execution Engine

### Stack Layout

The VM uses a single flat `Value[]` array (8192 slots) shared across all frames. Each frame owns a contiguous region starting at its `StackBase`:

```
StackBase + 0        param0
StackBase + 1        param1
StackBase + 2        local2    ← pre-scanned at compile time
StackBase + 3        local3
...
StackBase + LocalCount - 1     last local
_sp                            next free slot (grows upward)
```

Temporaries produced during expression evaluation live above `_sp` and are consumed by the opcodes that read them.

### Frame Stack

The VM maintains a `VMFrame[]` array (512 frames). Each frame tracks:

```
VMFrame
├── Chunk          — code being executed
├── IP             — instruction pointer (index into Chunk.Code)
├── StackBase      — absolute stack index where this frame's locals begin
├── Upvalues[]     — captured upvalue cells
├── Self           — current @ (instance context)
├── StructName     — current @@ (static context)
└── TryHandler stack — per-frame list of active try/catch/finally regions
```

### RunLoop

`RunLoop()` is the main dispatch loop:

```
while frameCount > stopAt:
    frame = top frame
    while IP < code length:
        instr = code[IP++]
        switch instr.Op:
            ... opcode handlers ...
    pop frame
```

When `DoCall` pushes a new frame it returns `true` and the outer loop starts dispatching the new frame's code. When `Return` or `ReturnNull` execute, the frame is popped and execution resumes in the caller.

---

## Opcode Reference

### Constants

| Opcode | A | B | Effect |
|--------|---|---|--------|
| `Const` | constant index | — | Push `constants[A]` |
| `Null` | — | — | Push null |
| `True` | — | — | Push true |
| `False` | — | — | Push false |

### Local Variables

| Opcode | A | B | Effect |
|--------|---|---|--------|
| `LoadLocal` | slot | — | Push `stack[base + A]` |
| `StoreLocal` | slot | — | `stack[base + A] = pop()` |

### Global Variables

| Opcode | A | B | Effect |
|--------|---|---|--------|
| `LoadGlobal` | name index | — | Push `globals[names[A]]`; cached for stable entries |
| `StoreGlobal` | name index | — | `globals[names[A]] = pop()` |

### Upvalues

| Opcode | A | B | Effect |
|--------|---|---|--------|
| `LoadUpvalue` | upvalue index | — | Push `upvalues[A].Get()` |
| `StoreUpvalue` | upvalue index | — | `upvalues[A].Set(pop())` |
| `CloseUpvalue` | slot | — | Close all open upvalues with absolute slot ≥ `base + A` |

### Stack

| Opcode | Effect |
|--------|--------|
| `Pop` | Discard top |
| `Dup` | Duplicate top |

### Arithmetic

| Opcode | Effect |
|--------|--------|
| `Add`, `Sub`, `Mul`, `Div`, `Mod`, `Pow` | Pop right, pop left, push result |
| `Neg` | Pop, push negated |

### Bitwise

| Opcode | Effect |
|--------|--------|
| `BAnd`, `BOr`, `BXor` | Pop right, pop left, push result |
| `BNot` | Pop, push bitwise-not |
| `BLSh`, `BRSh`, `BURSh` | Left/right/unsigned-right shift |

### Comparison and Logic

| Opcode | Effect |
|--------|--------|
| `Eq`, `NEq`, `Lt`, `LtE`, `Gt`, `GtE` | Pop right, pop left, push bool |
| `In` | Pop right (collection), pop left, push `left in right` |
| `Not` | Pop, push logical-not |
| `NullCoalesce` | Pop right, pop left, push `left ?? right` |
| `JumpAnd` | If top is falsy jump to A (leave on stack); else pop and continue |
| `JumpOr` | If top is truthy jump to A (leave on stack); else pop and continue |

### String

| Opcode | A | Effect |
|--------|---|--------|
| `Concat` | — | Pop right, pop left, push `left .. right` |
| `Interpolate` | part count | Pop A parts (bottom-first), concatenate, push |

### Control Flow

| Opcode | A | Effect |
|--------|---|--------|
| `Jump` | target IP | Unconditional jump |
| `JumpF` | target IP | Pop; if falsy jump |
| `JumpT` | target IP | Pop; if truthy jump |

### Function Calls

| Opcode | A | B | Effect |
|--------|---|---|--------|
| `Call` | argc | name index | Stack: `[..., func, arg0…argN-1]`; invoke func with N args |
| `CallSplat` | base argc | — | Like `Call` with splat adjustment from side-stack |
| `CallMethod` | argc | method name index | Pop receiver + args; dispatch `receiver.method(args)` |
| `MethodCallSplat` | base argc | method name index | Like `CallMethod` with splat |
| `CallNamed` | argc | arg-name-set index | Reorder args by name, then call |
| `CallMethodNamed` | argc | packed (method name \| name-set) | Named method call |
| `Return` | — | — | Pop return value; tear down frame; resume caller |
| `ReturnNull` | — | — | Return null; tear down frame; resume caller |

### Function and Closure Definition

| Opcode | A | B | Effect |
|--------|---|---|--------|
| `DefFunc` | sub-chunk index | name index | Create `KFunction` with VMChunk; register in context; no push |
| `MakeClosure` | sub-chunk index | upvalue count | Capture upvalues; create closure; push lambda value |

### Collections

| Opcode | A | Effect |
|--------|---|--------|
| `BuildList` | count | Pop A values (bottom-first); push list |
| `BuildHashmap` | pair count | Pop 2*A values (key, value pairs); push hashmap |
| `BuildRange` | — | Pop stop, pop start; push inclusive range list |

### Indexing and Slicing

| Opcode | A | Effect |
|--------|---|--------|
| `IndexGet` | — | Pop key, pop obj; push `obj[key]` |
| `IndexSet` | — | Pop value, pop key, pop obj; `obj[key] = value`; push value |
| `IndexOpAssign` | inner opcode | Compound index assignment (`lst[i] += v`) |
| `UnpackList` | count | Pop list; unpack A elements (or push value + nulls if not a list) |
| `SliceGet` | flags | Pop obj, optional start/stop/step; push slice |

### Member Access

| Opcode | A | Effect |
|--------|---|--------|
| `GetMember` | name index | Pop obj; push `obj.name` |
| `SetMember` | name index | Pop value, pop obj; `obj.name = value`; push value |

### Self and Static

| Opcode | A | Effect |
|--------|---|--------|
| `LoadSelf` | — | Push current `@` (instance context) |
| `LoadSelfAttr` | name index | Push `@.name` |
| `StoreSelfAttr` | name index | Pop value; `@.name = value`; push value |
| `LoadStaticAttr` | name index | Push `@@.name` (struct static variable) |
| `StoreStaticAttr` | name index | Pop value; `@@.name = value`; push value |

### Object Instantiation

| Opcode | A | B | Effect |
|--------|---|---|--------|
| `NewObject` | argc | struct name index | Create instance; call constructor with A args; push |

### Printing

| Opcode | A | Effect |
|--------|---|--------|
| `Print` | flags | Pop value; print (bit0 = newline, bit1 = stderr) |
| `PrintXy` | — | Pop y, pop x, pop text; print at terminal position (x, y) |

### Iteration

| Opcode | A | B | Effect |
|--------|---|---|--------|
| `ForIterInit` | — | — | Pop collection; push `ForIterState` pointer |
| `ForIterNext` | done-target IP | var count | Advance iterator; if exhausted pop state and jump to A; else push 1 or 2 values |

### Loop Control

| Opcode | A | Effect |
|--------|---|--------|
| `BreakJump` | extra items to pop | Exit loop |
| `NextJump` | — | Continue to next iteration |

### Generators

| Opcode | Effect |
|--------|--------|
| `Yield` | Pop value; suspend and yield to consumer |

### Splat (Spread Operator)

| Opcode | Effect |
|--------|--------|
| `SplatReset` | Push 0 onto splat-adjustment side-stack |
| `SplatPush` | Pop value; if list, expand onto stack and adjust side-stack count; else push unchanged |

### Error Handling

| Opcode | A | B | Effect |
|--------|---|---|--------|
| `Throw` | — | — | Pop value; raise as `KiwiError` |
| `PushTryHandler` | catch IP | finally IP | Register try/catch/finally handler on current frame |
| `PopTryHandler` | — | — | Remove top try handler (normal completion) |
| `LoadCatchError` | 0/1/2 | — | 0 = push error hashmap; 1 = push error type; 2 = push message |

### Type Introspection

| Opcode | Effect |
|--------|--------|
| `TypeOf` | Pop value; push type name string (e.g. `"integer"`, `"list"`) |

### Events

| Opcode | A | Effect |
|--------|---|--------|
| `EventOn` | priority | Pop callback, pop event name; register handler |
| `EventOnce` | priority | Pop callback, pop event name; register one-time handler |
| `EventOff` | 0 or 1 | Pop event name (and optionally callback); deregister |
| `EventEmit` | argc | Pop A args, pop event name; emit; push result list |

### Struct Definition

| Opcode | A | B | Effect |
|--------|---|---|--------|
| `StructBegin` | name index | packed (abstract flag \| base name index) | Create `KStruct`; push onto pending-struct stack |
| `DefMethod` | sub-chunk index | packed (name index \| flags) | Register method into pending struct |
| `InitStructStatic` | name index | — | Pop value; store in pending struct's static variables |
| `StructEnd` | — | — | Validate abstract compliance; register struct |

### Package Definition

| Opcode | A | B | Effect |
|--------|---|---|--------|
| `PackageBegin` | name index | node-pool index | Build qualified name; push onto package stack; register `KPackage` |
| `PackageEnd` | — | — | Pop package stack; register type builtins; mark imported |
| `PackageAbort` | — | — | Catch path; pop package stack without marking imported |

### Enum Definition

| Opcode | A | Effect |
|--------|---|--------|
| `EnumBegin` | name index | Create enum struct |
| `DefEnumMember` | name index | Pop value; store as `@@name` in pending enum |
| `EnumEnd` | — | Register enum |

### Interpreter Boundary

| Opcode | A | B | Effect |
|--------|---|---|--------|
| `InterpFallback` | node-pool index | — | Execute `NodePool[A]` via tree-walker; sync locals; push result |
| `CallBuiltin` | node-pool index | argc | Pop B args; call `ExecuteBuiltin(token, op, args)`; push result |
| `Export` | node-pool index | — | Interpret `ExportNode` via `InterpretNodeWithLocals` |

### System

| Opcode | Effect |
|--------|--------|
| `Exit` | Pop exit code; call `Environment.Exit` |
| `Nop` | No operation |
| `Halt` | End of program |

---

## Closures and Upvalues

Every function or lambda that captures a variable from an enclosing scope becomes a closure. The captured variables are called **upvalues**.

### Upvalue Lifecycle

An upvalue starts **open**: it holds a direct pointer into the VM stack at the variable's absolute slot. All readers and writers share the same slot, so mutations are immediately visible.

When the enclosing frame returns, any open upvalues for its locals are **closed**: the current value is copied from the stack into the upvalue's own heap cell. Subsequent reads and writes go to the heap cell instead. This allows closures to outlive the frame that created them.

```
Open:    upvalue._stack[slot]  ← reads/writes go here (shared with frame)
Closed:  upvalue._closed       ← reads/writes go here (heap copy)
```

### Open Upvalue List

The VM maintains `_openUpvalues`: a singly-linked list of all currently-open upvalues ordered by descending absolute slot index. When a new upvalue is needed, the list is walked to find or insert it. This ensures that two closures capturing the same variable get the same upvalue cell.

### Upvalue Descriptors

The compiler emits a `UpvalueDescriptor` for each captured variable:

```
UpvalueDescriptor
├── IsLocal: bool    — true = capture a local from the immediately enclosing frame
└── Index:   int     — slot index (if IsLocal) or parent upvalue index (if not)
```

When `DefFunc` or `MakeClosure` executes, `BuildUpvalues` walks the descriptors and either calls `CaptureUpvalue(absoluteSlot)` (for locals) or reuses an upvalue from the current frame's own upvalue array (for variables captured from further up the chain).

---

## Function Calls: DoCall

All calls — user functions, stdlib functions, lambdas, methods — flow through `DoCall(funcVal, argc, funcSlot, token)`.

```
DoCall
├── funcVal is a VM-compiled lambda or function?
│     ├── IsGenerator → create GeneratorRef, push, return false
│     └── Normal → SetupCalleeLocals(); PushFrame(); return true  ← new VM frame
│
├── funcVal is a tree-walked KFunction (no VMChunk)?
│     └── _interp.InvokeCallable(); push result; return false
│
└── funcVal is a bound method or other callable?
      └── _interp.DispatchMethod(); push result; return false
```

`return true` means a new frame was pushed and the outer loop should start dispatching it. `return false` means the call was handled in-place and the result is already on the stack.

### Stack Setup

Before pushing a new frame, `SetupCalleeLocals` prepares the stack:
- Arguments are already at `calleeBase + 0 .. calleeBase + argc - 1` (put there by the `Call` opcode's arg-compilation)
- Extra locals beyond `argc` are zero-initialized up to `calleeBase + LocalCount`
- If the function has a variadic parameter, trailing args are packed into a list at the variadic slot
- `_sp` is set to `calleeBase + LocalCount`

---

## Interpreter Boundary

The VM and the tree-walking interpreter share runtime state (`Interpreter`, `Scope`, `KContext`). This allows constructs that aren't compiled natively to fall back to the interpreter without a full context switch.

### InterpFallback

Used for AST node types not yet handled by the compiler (complex compound assignments, `eval`, decorated functions, `include`). The node was stored in `NodePool` at compile time.

At runtime:
1. Extract `NodePool[A]`
2. Build a `locals` dict from pre-scanned local slots
3. Call `_interp.InterpretNodeWithLocals(node, locals, frame.Self)` — this pushes a `<vm-fallback>` frame onto the interpreter's call stack so that builtins calling `CallStack.Peek()` don't crash
4. Sync any mutated locals back to the VM stack
5. Push the result

### CallBuiltin

Used for C# native calls that aren't `CoreBuiltin` method calls — math functions, socket ops, task/channel builtins, serializers, etc. Arguments are compiled normally (pushed as bytecode), so only the dispatch crosses the boundary.

At runtime:
1. Extract `FunctionCallNode` from `NodePool[A]` (for its `Token` and `Op`)
2. Pop `B` args from stack; reverse to restore left-to-right order
3. Call `_interp.ExecuteBuiltin(node.Token, node.Op, args)` — dispatches to `KiwiBuiltinHandler`, `SocketBuiltinHandler`, `BuiltinDispatch`, etc.
4. Push the result

### Export

`export "package_name"` re-interprets the `ExportNode` via `InterpretNodeWithLocals`. This is necessary because `ImportPackage` re-evaluates the package body (struct definitions, function definitions), which calls into `CreateFunction` → `CaptureCurrentScope().Peek()` — requiring a live interpreter frame.

### What Uses InterpFallback vs. CallBuiltin

| Construct | Mechanism |
|-----------|-----------|
| Non-CoreBuiltin function call, no named/splat args | `CallBuiltin` |
| `export "pkg"` | `Export` opcode (InterpretNodeWithLocals) |
| Complex `IndexAssignment` (compound ops) | `InterpFallback` |
| `eval` | `InterpFallback` |
| `include` | `InterpFallback` |
| `@decorator fn` | `InterpFallback` |
| Named + splat arg combo | `InterpFallback` |

---

## Error Handling

`try / catch / finally` is implemented via a per-frame try-handler stack. Each `PushTryHandler` opcode records a catch IP, a finally IP, and the stack pointer at the time. When a `KiwiError` propagates:

1. The VM unwinds frames until it finds one with a try handler
2. The stack pointer is restored to the saved value
3. The caught error is stored in `_caughtError`
4. Execution jumps to the catch IP

`LoadCatchError` retrieves the error's type string (A=1), message (A=2), or full hashmap (A=0) from `_caughtError`.

`PopTryHandler` removes the handler on normal completion. The finally block is emitted inline by the compiler so it runs in both the normal and error paths.

---

## Disassembly

Set `KIWI_DISASM=1` to dump the compiled chunk to stdout before execution:

```bash
KIWI_DISASM=1 kiwi script.kiwi
```

Sample output:

```
=== <main> (arity=0, locals=1, upvalues=0) ===
     0  [   1]  DefFunc              sub[0]  name="greet"
     1  [   3]  LoadGlobal           "greet"
     2  [   3]  Const                   0   ("world")
     3  [   3]  Call             argc=1
     4  [   3]  Pop
     5  [   3]  Halt

  === greet (arity=1, locals=1, upvalues=0) ===
  params: name
       0  [   1]  LoadLocal        slot=0
       1  [   1]  Interpolate      parts=2
       2  [   1]  Print            newline=True  stderr=False
       3  [   1]  ReturnNull
```

Sub-chunks are indented under their parent and listed after the parent's instruction stream.

---

## Limits

| Resource | Limit |
|----------|-------|
| Value stack depth | 8192 slots |
| Call stack depth | 512 frames |
