# Bytecode VM

> How the Kiwi interpreter compiles and runs code, and the VM instruction set.

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

### Constants

| Opcode | Behavior |
| :-- | :-- |
| `Const` | push constants[A] |
| `Null` | push null |
| `True` | push true |
| `False` | push false |

### Local Variables (indexed slots)

| Opcode | Behavior |
| :-- | :-- |
| `LoadLocal` | push stack[base + A] |
| `StoreLocal` | stack[base + A] = pop() |

### Global Variables (dictionary)

| Opcode | Behavior |
| :-- | :-- |
| `LoadGlobal` | push globals[names[A]] |
| `StoreGlobal` | globals[names[A]] = pop() |
| `StoreConst` | globals[names[A]] = pop(); also registers names[A] in context.Constants (immutable). |

### Upvalues (closures)

| Opcode | Behavior |
| :-- | :-- |
| `LoadUpvalue` | push upvalues[A].Get() |
| `StoreUpvalue` | upvalues[A].Set(pop()) |
| `CloseUpvalue` | Close all open upvalues whose absolute slot >= base + A |
| `CloseUpvalueAt` | Close the open upvalue for slot base + A only, if there is one. Emitted at the top of each `for` iteration so closures created in the body keep that iteration's value. |

### Stack Manipulation

| Opcode | Behavior |
| :-- | :-- |
| `Pop` | Discard the top value. |
| `Dup` | Duplicate the top value. |
| `Swap` | Swap the top two stack values. |

### Arithmetic

| Opcode | Behavior |
| :-- | :-- |
| `Add` | pop right, pop left → push left + right |
| `Sub` | pop right, pop left → push left - right |
| `Mul` | pop right, pop left → push left * right |
| `Div` | pop right, pop left → push left / right (float) |
| `IntDiv` | pop right, pop left → push left // right (floor) |
| `Mod` | pop right, pop left → push left % right |
| `Pow` | pop right, pop left → push left ** right |
| `Neg` | Unary negation |

### Bitwise

| Opcode | Behavior |
| :-- | :-- |
| `BAnd` | pop right, pop left → push left & right |
| `BOr` | pop right, pop left → push left \| right |
| `BXor` | pop right, pop left → push left ^ right |
| `BNot` | pop value → push ~value |
| `BLSh` | pop right, pop left → push left << right |
| `BRSh` | pop right, pop left → push left >> right |
| `BURSh` | pop right, pop left → push left >>> right |

### Comparison

| Opcode | Behavior |
| :-- | :-- |
| `Eq` | pop right, pop left → push left == right |
| `NEq` | pop right, pop left → push left != right |
| `Lt` | pop right, pop left → push left < right |
| `LtE` | pop right, pop left → push left <= right |
| `Gt` | pop right, pop left → push left > right |
| `GtE` | pop right, pop left → push left >= right |
| `In` | pop right, pop left → push left in right (collection membership) |

### Logical

| Opcode | Behavior |
| :-- | :-- |
| `Not` | Unary not |
| `JumpAnd` | Short-circuit AND: peek top; if falsy → jump to A (leave value on stack); if truthy → pop and continue (right side will supply the value). |
| `JumpOr` | Short-circuit OR: peek top; if truthy → jump to A (leave value on stack); if falsy → pop and continue. |

### Null Coalescing

| Opcode | Behavior |
| :-- | :-- |
| `NullCoalesce` | pop right, pop left; push left ?? right |

### String Concat

| Opcode | Behavior |
| :-- | :-- |
| `Concat` | String ".." concat: pop right, pop left, push left..right |

### Control Flow

| Opcode | Behavior |
| :-- | :-- |
| `Jump` | IP = A (unconditional jump) |
| `JumpF` | pop; if falsy: IP = A |
| `JumpT` | pop; if truthy: IP = A |

### Function Calls

| Opcode | Behavior |
| :-- | :-- |
| `Call` | A = arg count. Stack layout before: [..., func_value, arg0, …, argN-1]. Calls the function, replaces func+args with the return value. |
| `CallMethod` | A = arg count, B = name index (names[B] = method name). Stack before: [..., obj, arg0, …, argN-1]. Dispatches the method call; replaces obj+args with result. |
| `CallSuperMethod` | A = arg count, B = name index (names[B] = method name). Stack before: [..., arg0, …, argN-1] (no obj — uses frame.Self and frame.OwnerStruct). Dispatches a `super.method()` call starting from the base of the defining struct. |
| `Return` | Return top-of-stack to caller. |
| `ReturnNull` | Return null to caller. |

### Function / Closure Definition

| Opcode | Behavior |
| :-- | :-- |
| `DefFunc` | A = sub-chunk index, B = name index (names[B] = function name). Register a compiled KFunction in the context. Does not push a value. |
| `DefFuncAndPush` | A = sub-chunk index, B = name index (names[B] = raw function name). Like DefFunc but also wraps the function as an internal KLambda and pushes a lambda-ref onto the stack. Used by decorator compilation so the function can be passed as a value to the decorator callable without a separate LoadGlobal. |
| `StoreDecoratedFunc` | A = name index (raw function name). Pop the top-of-stack (the final decorated value); compute the package-qualified function name; remove the original KFunction entry from Context.Functions; then register the decorated result (lambda mapping or global assignment). Used as the last step of decorator application. |
| `MakeClosure` | A = sub-chunk index, B = number of upvalue descriptors that follow as inline (isLocal, index) pairs encoded in subsequent Nop-like slots. Pushes the resulting closure Value. |

### Collections

| Opcode | Behavior |
| :-- | :-- |
| `BuildList` | Pop A values (bottom-first) and push a List. |
| `BuildHashmap` | Pop 2*A values (key, value pairs bottom-first) and push a Hashmap. |
| `BuildRange` | Pop start then stop → push inclusive range list. |

### Indexing

| Opcode | Behavior |
| :-- | :-- |
| `IndexGet` | pop key, pop obj → push obj[key] |
| `IndexSet` | pop value, pop key, pop obj → obj[key] = value (pushes value) |
| `IndexOpAssign` | A = inner opcode (as int). pop rhs, pop key, pop obj → old = obj[key]; new = op(old, rhs); obj[key] = new (pushes new). Handles compound index assignment: lst[i] += v. |
| `UnpackList` | A = count. Pop a value; if it's a list unpack A elements; else push value then A-1 nulls. Used for PackAssignment: a, b = func_returning_list(). |
| `SliceGet` | A = flags: bit0=hasStart, bit1=hasStop, bit2=hasStep. Pops (in order): obj, start?, stop?, step? → push slice. |
| `SliceSet` | A = flags: bit0=hasStart, bit1=hasStop, bit2=hasStep. Pops (in order from stack top): rhs, step?, stop?, start?, obj. Applies rhs to the slice range of obj in-place; pushes rhs. |

### Member Access

| Opcode | Behavior |
| :-- | :-- |
| `GetMember` | A = name index; pop obj → push obj.names[A] |
| `SetMember` | A = name index; pop value, pop obj → obj.names[A] = value (pushes value) |

### Self / Static Self

| Opcode | Behavior |
| :-- | :-- |
| `LoadSelf` | Push @ (current object context) |
| `LoadSelfAttr` | A = name index → push @.names[A] |
| `StoreSelfAttr` | A = name index; pop → @.names[A] = value (pushes value) |
| `LoadStaticAttr` | A = name index → push @@.names[A] (static variable) |
| `StoreStaticAttr` | A = name index; pop → @@.names[A] = value (pushes value) |

### Struct Instantiation

| Opcode | Behavior |
| :-- | :-- |
| `NewObject` | A = arg count, B = name index; create new struct instance and push it. |

### String Interpolation

| Opcode | Behavior |
| :-- | :-- |
| `Interpolate` | Pop A string parts (bottom-first) and push the concatenated result. |

### Print

| Opcode | Behavior |
| :-- | :-- |
| `Print` | A: bit0 = newline, bit1 = stderr. Pop value and print it. |
| `PrintXy` | Pop y, pop x, pop text → print text at terminal position (x, y) using ANSI cursor-move. |

### Loop Iteration

| Opcode | Behavior |
| :-- | :-- |
| `ForIterInit` | Pop the collection and push a ForIterState (held directly in the stack slot). |
| `ForIterInitRange` | Pop stop, pop start → push a ForIterState that counts from start to stop inclusive (downward when stop < start). Used for `for x in [a to b]` so the range is never materialized as a list. |
| `ForIterNext` | A = jump-if-done target IP, B = number of loop vars to push (1 or 2). Peek at the ForIterState on top of stack: - If done: pop state, jump to A. - Else: push next value(s) (B values) without removing state. |

### Loop Control

| Opcode | Behavior |
| :-- | :-- |
| `BreakJump` | Signal a break: VM pops loop state (A = extra items to pop) and jumps. |
| `NextJump` | Signal a continue/next. |

### Generator

| Opcode | Behavior |
| :-- | :-- |
| `Yield` | Pop value and yield it to the generator consumer. |

### Splat (spread arguments)

| Opcode | Behavior |
| :-- | :-- |
| `SplatReset` | Push 0 onto the splat-adjustment side-stack. Emitted before the function/object of any call that contains splat arguments, so nested splat calls each get their own adjustment slot. |
| `SplatPush` | Pop value. If it is a list, push each element and add (count-1) to the top of the splat-adjustment side-stack. Otherwise push the value unchanged (single-element, zero adjustment). |
| `CallSplat` | Like Call but argc = A + splatAdjStack.Pop(). Used for function/lambda calls that contain one or more splat arguments. |
| `MethodCallSplat` | Like CallMethod but argc = A + splatAdjStack.Pop(). Used for method calls that contain one or more splat arguments. |
| `CallNamed` | Like Call but reorders args to match the callee's declared parameter order. A = argc, B = index into chunk.ArgNameSets (parallel string[] of arg names; empty string = positional, non-empty = named arg name). |
| `CallMethodNamed` | Like CallMethod but reorders args before dispatch. A = argc, B = packed: lower 16 bits = method name index in chunk.Names, upper 16 bits = index into chunk.ArgNameSets. |

### Error Handling

| Opcode | Behavior |
| :-- | :-- |
| `Throw` | Pop value and throw it as a Kiwi runtime error. |
| `PushTryHandler` | A = catch IP, B = finally IP (0 = none). Pushes a try-handler frame: on KiwiError, restore _sp to current value, jump to catch IP, and store the caught error in _caughtError. |
| `PopTryHandler` | Pop the top try-handler frame (try body completed normally). |
| `LoadCatchError` | A = 0 → push error hashmap {error, message} (single-param catch) A = 1 → push error type string (first of two-param catch) A = 2 → push error message string (second of two-param catch) A = 3 → push the error itself, to rethrow after a `finally` block Reads from the VM's _caughtError field set when a handler fires. |
| `CheckType` | A = type-check index. Throw a TypeError unless the value on top of the stack (left in place) matches the type hint chunk.TypeChecks[A]: a parameter, return value or `var`. |
| `Rethrow` | Pop an error pushed by LoadCatchError (A = 3) and throw it again, so an error leaving a `try` with a `finally` block keeps propagating once the block has run. |

### Type Introspection

| Opcode | Behavior |
| :-- | :-- |
| `TypeOf` | Pop value → push typeof string (e.g. "integer", "string", "list", …). |

### Event Bus

| Opcode | Behavior |
| :-- | :-- |
| `EventOn` | A = priority. Pop callback, pop event-name → register 'on' handler. |
| `EventOnce` | A = priority. Pop callback, pop event-name → register 'once' handler. |
| `EventOff` | A: 0 = no callback (remove all), 1 = pop callback too. Pop [callback,] event-name → unregister handler(s). |
| `EventEmit` | A = arg count. Pop A args (bottom-first), pop event-name → emit event, push result list. |

### Struct Definition

| Opcode | Behavior |
| :-- | :-- |
| `StructBegin` | A = name index. B = packed: bit 0 = isAbstract, bits 1+ = baseNameIdx+1 (0 = no base). Create KStruct and push onto the pending-struct stack. |
| `DefMethod` | A = sub-chunk index, B = nameIdx \| (isAbstract ? 0x80000000 : 0) \| (isStatic ? 0x40000000 : 0) \| (isPrivate ? 0x20000000 : 0) \| (isDecorated ? 0x10000000 : 0). Like DefFunc but registers the method into the current pending struct. A decorated method is also pushed as a value for its decorators. |
| `StoreDecoratedMethod` | A = name index. Pop the decorated value of a method and install it in the pending struct: calling the method calls that value, with `@` still the instance. |
| `InitStructStatic` | A = name index. Pop value → store in pending struct's StaticVariables[names[A]]. |
| `StructEnd` | Validate abstract compliance, register pending struct in Context.Structs. |

### Package Definition

| Opcode | Behavior |
| :-- | :-- |
| `PackageBegin` | A = name index, B = node-pool index (stores PackageNode AST for retry). Build qualified name, push onto PackageStack, register KPackage. |
| `PackageEnd` | Pop PackageStack; register type builtins; mark package imported. |
| `PackageAbort` | Catch path for a failed package activation: pop PackageStack without marking as imported. Allows ImportPackage to retry after dependencies are resolved. |

### Enum Definition

| Opcode | Behavior |
| :-- | :-- |
| `EnumBegin` | A = name index. Create KStruct{IsEnum=true} and push onto the pending-struct stack. |
| `DefEnumMember` | A = name index. Pop value → store in pending enum's StaticVariables["@@" + names[A]]. |
| `EnumEnd` | Pop pending KStruct and register it as an enum in Context.Structs. |

### Builtin call

| Opcode | Behavior |
| :-- | :-- |
| `CallBuiltin` | A = node-pool index (FunctionCallNode, used for Token and Op). B = argc. Pop B args (in order), call _interp.ExecuteBuiltin, push result. Used for C# native builtin calls (KiwiBuiltin, SocketBuiltin, Math, etc.) that are not CoreBuiltin method calls. |

### Export / Import / Require

| Opcode | Behavior |
| :-- | :-- |
| `Export` | Pop top-of-stack (package name string); call _interp.ImportPackage with it. |
| `ImportPkg` | Pop top-of-stack (package name string); call _interp.ImportPackage with it; push Value.CreatePackage(name) as the result of the import expression. |
| `Require` | Pop top-of-stack (package name string); throw PackageUndefinedError if the package is not registered in the context. Used by the `require` keyword. |

### Eval / Include

| Opcode | Behavior |
| :-- | :-- |
| `Eval` | Pop top-of-stack (Kiwi source string); lex/parse/interpret it; push the result. |
| `Include` | Pop top-of-stack (file path string); load and interpret the file (no push). |

### Exit

| Opcode | Behavior |
| :-- | :-- |
| `Exit` | Pop exit-code value; call Environment.Exit(code). Condition (if any) is handled at compile time via a JumpF guard. |

### Misc

| Opcode | Behavior |
| :-- | :-- |
| `Nop` | Do nothing. |
| `Halt` | Stop executing the chunk. |


## Adding a language feature

A new construct usually touches each stage in turn:

1. **Lexer**: add a token in `Lexer.cs` if the syntax needs one.
2. **Parser**: add an AST node in `src/Parsing/AST/` and parse it in `Parser.cs`.
3. **Compiler**: emit instructions for the node in `Compiler.cs`.
4. **VM**: if no existing opcode fits, add one to `Opcode.cs`, with a `<summary>` comment so it appears on this page, and handle it in `KiwiVM.cs`.
5. **Tests**: add tests under `tests/lib/suite/`, rebuild with `./build.sh`, and run `./bin/kiwi tests/test`.
