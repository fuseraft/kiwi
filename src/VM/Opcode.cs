namespace kiwi.VM;

/// <summary>
/// Bytecode instruction set for the Kiwi stack-based virtual machine.
///
/// Stack convention for binary ops:   left is pushed first, right second.
///   i.e.  push A; push B; Add  →  A + B
///
/// Local variable slots are stored inside the VM's value stack at
/// absolute indices [frame.StackBase .. frame.StackBase + chunk.LocalCount - 1].
/// An operand of type "slot" is always relative to StackBase.
/// </summary>
public enum Opcode : byte
{
    // -- Constants ------------------------------------------------------------
    /// <summary>
    /// push constants[A]
    /// </summary>
    Const,
    /// <summary>
    /// push null
    /// </summary>
    Null,
    /// <summary>
    /// push true
    /// </summary>
    True,
    /// <summary>
    /// push false
    /// </summary>
    False,

    // -- Local Variables (indexed slots) --------------------------------------
    /// <summary>
    /// push stack[base + A]
    /// </summary>
    LoadLocal,
    /// <summary>
    /// stack[base + A] = pop()
    /// </summary>
    StoreLocal,

    // -- Global Variables (dictionary) ----------------------------------------
    /// <summary>
    /// push globals[names[A]]
    /// </summary>
    LoadGlobal,
    /// <summary>
    /// globals[names[A]] = pop()
    /// </summary>
    StoreGlobal,

    // -- Upvalues (closures) ---------------------------------------------------
    /// <summary>
    /// push upvalues[A].Get()
    /// </summary>
    LoadUpvalue,
    /// <summary>
    /// upvalues[A].Set(pop())
    /// </summary>
    StoreUpvalue,
    /// <summary>
    /// Close all open upvalues whose absolute slot >= base + A
    /// </summary>
    CloseUpvalue,

    // -- Stack Manipulation ----------------------------------------------------
    Pop,
    Dup,

    // -- Arithmetic ------------------------------------------------------------
    Add, Sub, Mul, Div, Mod, Pow,
    /// <summary>
    /// Unary negation
    /// </summary>
    Neg,

    // -- Bitwise --------------------------------------------------------------
    BAnd, BOr, BXor, BNot, BLSh, BRSh, BURSh,

    // -- Comparison -----------------------------------------------------------
    Eq, NEq, Lt, LtE, Gt, GtE,
    /// <summary>
    /// pop right, pop left → push left in right  (collection membership)
    /// </summary>
    In,

    // -- Logical --------------------------------------------------------------
    /// <summary>
    /// Unary not
    /// </summary>
    Not,
    /// <summary>
    /// Short-circuit AND: peek top; if falsy → jump to A (leave value on stack);
    /// if truthy → pop and continue (right side will supply the value).
    /// </summary>
    JumpAnd,
    /// <summary>
    /// Short-circuit OR: peek top; if truthy → jump to A (leave value on stack);
    /// if falsy → pop and continue.
    /// </summary>
    JumpOr,

    // -- Null Coalescing -------------------------------------------------------
    /// <summary>
    /// pop right, pop left; push left ?? right
    /// </summary>
    NullCoalesce,

    // -- String Concat ---------------------------------------------------------
    /// <summary>
    /// String ".." concat: pop right, pop left, push left..right
    /// </summary>
    Concat,

    // -- Control Flow ---------------------------------------------------------
    /// <summary>
    /// IP = A (unconditional jump)
    /// </summary>
    Jump,
    /// <summary>
    /// pop; if falsy: IP = A
    /// </summary>
    JumpF,
    /// <summary>
    /// pop; if truthy: IP = A
    /// </summary>
    JumpT,

    // -- Function Calls --------------------------------------------------------
    /// <summary>
    /// A = arg count.  Stack layout before: [..., func_value, arg0, …, argN-1].
    /// Calls the function, replaces func+args with the return value.
    /// </summary>
    Call,
    /// <summary>
    /// A = arg count, B = name index (names[B] = method name).
    /// Stack before: [..., obj, arg0, …, argN-1].
    /// Dispatches the method call; replaces obj+args with result.
    /// </summary>
    CallMethod,
    /// <summary>
    /// Return top-of-stack to caller.
    /// </summary>
    Return,
    /// <summary>
    /// Return null to caller.
    /// </summary>
    ReturnNull,

    // -- Function / Closure Definition -----------------------------------------
    /// <summary>
    /// A = sub-chunk index, B = name index (names[B] = function name).
    /// Register a compiled KFunction in the context.  Does not push a value.
    /// </summary>
    DefFunc,
    /// <summary>
    /// A = sub-chunk index, B = number of upvalue descriptors that follow as
    /// inline (isLocal, index) pairs encoded in subsequent Nop-like slots.
    /// Pushes the resulting closure Value.
    /// </summary>
    MakeClosure,

    /// <summary>
    /// A = upvalue-count.  Upvalue descriptors are stored in chunk.Upvalues[0..A-1].
    /// This is an alternate encoding where upvalue info is in the chunk metadata.
    /// Used internally after MakeClosure to avoid variable-length instruction streams.
    /// (Not emitted directly; kept for tooling.)
    /// </summary>

    // -- Collections ----------------------------------------------------------
    /// <summary>
    /// Pop A values (bottom-first) and push a List.
    /// </summary>
    BuildList,
    /// <summary>
    /// Pop 2*A values (key, value pairs bottom-first) and push a Hashmap.
    /// </summary>
    BuildHashmap,
    /// <summary>
    /// Pop start then stop → push inclusive range list.
    /// </summary>
    BuildRange,

    // -- Indexing -------------------------------------------------------------
    /// <summary>
    /// pop key, pop obj → push obj[key]
    /// </summary>
    IndexGet,
    /// <summary>
    /// pop value, pop key, pop obj → obj[key] = value  (pushes value)
    /// </summary>
    IndexSet,
    /// <summary>
    /// A = inner opcode (as int).  pop rhs, pop key, pop obj →
    ///   old = obj[key]; new = op(old, rhs); obj[key] = new  (pushes new).
    /// Handles compound index assignment: lst[i] += v.
    /// </summary>
    IndexOpAssign,
    /// <summary>
    /// A = count.  Pop a value; if it's a list unpack A elements; else push value then A-1 nulls.
    /// Used for PackAssignment: a, b = func_returning_list().
    /// </summary>
    UnpackList,
    /// <summary>
    /// A = flags: bit0=hasStart, bit1=hasStop, bit2=hasStep.
    /// Pops (in order): obj, start?, stop?, step? → push slice.
    /// </summary>
    SliceGet,

    // -- Member Access ---------------------------------------------------------
    /// <summary>
    /// A = name index; pop obj → push obj.names[A]
    /// </summary>
    GetMember,
    /// <summary>
    /// A = name index; pop value, pop obj → obj.names[A] = value  (pushes value)
    /// </summary>
    SetMember,

    // -- Self / Static Self ----------------------------------------------------
    /// <summary>
    /// Push @ (current object context)
    /// </summary>
    LoadSelf,
    /// <summary>
    /// A = name index → push @.names[A]
    /// </summary>
    LoadSelfAttr,
    /// <summary>
    /// A = name index; pop → @.names[A] = value  (pushes value)
    /// </summary>
    StoreSelfAttr,
    /// <summary>
    /// A = name index → push @@.names[A]  (static variable)
    /// </summary>
    LoadStaticAttr,
    /// <summary>
    /// A = name index; pop → @@.names[A] = value  (pushes value)
    /// </summary>
    StoreStaticAttr,

    // -- Struct Instantiation --------------------------------------------------
    /// <summary>
    /// A = arg count, B = name index; create new struct instance and push it.
    /// </summary>
    NewObject,

    // -- String Interpolation --------------------------------------------------
    /// <summary>
    /// Pop A string parts (bottom-first) and push the concatenated result.
    /// </summary>
    Interpolate,

    // -- Print -----------------------------------------------------------------
    /// <summary>
    /// A: bit0 = newline, bit1 = stderr.
    /// Pop value and print it.
    /// </summary>
    Print,
    /// <summary>
    /// Pop y, pop x, pop text → print text at terminal position (x, y) using ANSI cursor-move.
    /// </summary>
    PrintXy,

    // -- Loop Iteration --------------------------------------------------------
    /// <summary>
    /// Pop the collection and push a ForIterState (as a Pointer value).
    /// </summary>
    ForIterInit,
    /// <summary>
    /// A = jump-if-done target IP, B = number of loop vars to push (1 or 2).
    /// Peek at the ForIterState on top of stack:
    ///   - If done: pop state, jump to A.
    ///   - Else: push next value(s) (B values) without removing state.
    /// </summary>
    ForIterNext,

    // -- Loop Control ----------------------------------------------------------
    /// <summary>
    /// Signal a break: VM pops loop state (A = extra items to pop) and jumps.
    /// </summary>
    BreakJump,
    /// <summary>
    /// Signal a continue/next.
    /// </summary>
    NextJump,

    // -- Generator ------------------------------------------------------------
    /// <summary>
    /// Pop value and yield it to the generator consumer.
    /// </summary>
    Yield,

    // -- Splat (spread arguments) ---------------------------------------------
    /// <summary>
    /// Push 0 onto the splat-adjustment side-stack.
    /// Emitted before the function/object of any call that contains splat arguments,
    /// so nested splat calls each get their own adjustment slot.
    /// </summary>
    SplatReset,
    /// <summary>
    /// Pop value. If it is a list, push each element and add (count-1) to the
    /// top of the splat-adjustment side-stack. Otherwise push the value unchanged
    /// (single-element, zero adjustment).
    /// </summary>
    SplatPush,
    /// <summary>
    /// Like Call but argc = A + splatAdjStack.Pop().
    /// Used for function/lambda calls that contain one or more splat arguments.
    /// </summary>
    CallSplat,
    /// <summary>
    /// Like CallMethod but argc = A + splatAdjStack.Pop().
    /// Used for method calls that contain one or more splat arguments.
    /// </summary>
    MethodCallSplat,
    /// <summary>
    /// Like Call but reorders args to match the callee's declared parameter order.
    /// A = argc, B = index into chunk.ArgNameSets (parallel string[] of arg names;
    /// empty string = positional, non-empty = named arg name).
    /// </summary>
    CallNamed,
    /// <summary>
    /// Like CallMethod but reorders args before dispatch.
    /// A = argc, B = packed: lower 16 bits = method name index in chunk.Names,
    /// upper 16 bits = index into chunk.ArgNameSets.
    /// </summary>
    CallMethodNamed,

    // -- Error Handling --------------------------------------------------------
    /// <summary>
    /// Pop value and throw it as a Kiwi runtime error.
    /// </summary>
    Throw,
    /// <summary>
    /// A = catch IP, B = finally IP (0 = none).
    /// Pushes a try-handler frame: on KiwiError, restore _sp to current value,
    /// jump to catch IP, and store the caught error in _caughtError.
    /// </summary>
    PushTryHandler,
    /// <summary>
    /// Pop the top try-handler frame (try body completed normally).
    /// </summary>
    PopTryHandler,
    /// <summary>
    /// A = 0 → push error hashmap {error, message}  (single-param catch)
    /// A = 1 → push error type string               (first of two-param catch)
    /// A = 2 → push error message string             (second of two-param catch)
    /// Reads from the VM's _caughtError field set when a handler fires.
    /// </summary>
    LoadCatchError,

    // -- Type Introspection ----------------------------------------------------
    /// <summary>
    /// Pop value → push typeof string (e.g. "integer", "string", "list", …).
    /// </summary>
    TypeOf,

    // -- Event Bus -------------------------------------------------------------
    /// <summary>
    /// A = priority.  Pop callback, pop event-name → register 'on' handler.
    /// </summary>
    EventOn,
    /// <summary>
    /// A = priority.  Pop callback, pop event-name → register 'once' handler.
    /// </summary>
    EventOnce,
    /// <summary>
    /// A: 0 = no callback (remove all), 1 = pop callback too.
    /// Pop [callback,] event-name → unregister handler(s).
    /// </summary>
    EventOff,
    /// <summary>
    /// A = arg count.  Pop A args (bottom-first), pop event-name → emit event,
    /// push result list.
    /// </summary>
    EventEmit,

    // -- Struct Definition -----------------------------------------------------
    /// <summary>
    /// A = name index.
    /// B = packed: bit 0 = isAbstract, bits 1+ = baseNameIdx+1 (0 = no base).
    /// Create KStruct and push onto the pending-struct stack.
    /// </summary>
    StructBegin,
    /// <summary>
    /// A = sub-chunk index, B = nameIdx | (isAbstract ? 0x80000000 : 0).
    /// Like DefFunc but registers the method into the current pending struct.
    /// </summary>
    DefMethod,
    /// <summary>
    /// A = name index. Pop value → store in pending struct's StaticVariables[names[A]].
    /// </summary>
    InitStructStatic,
    /// <summary>
    /// Validate abstract compliance, register pending struct in Context.Structs.
    /// </summary>
    StructEnd,

    // -- Package Definition ----------------------------------------------------
    /// <summary>
    /// A = name index, B = node-pool index (stores PackageNode AST for retry).
    /// Build qualified name, push onto PackageStack, register KPackage.
    /// </summary>
    PackageBegin,
    /// <summary>
    /// Pop PackageStack; register type builtins; mark package imported.
    /// </summary>
    PackageEnd,
    /// <summary>
    /// Catch path for a failed package activation: pop PackageStack without marking as imported.
    /// Allows ImportPackage to retry after dependencies are resolved.
    /// </summary>
    PackageAbort,

    // -- Interpreter Fallback --------------------------------------------------
    /// <summary>
    /// A = node-pool index.  Execute chunk.NodePool[A] via the tree-walking
    /// interpreter and push the result.  Used for constructs not yet compiled
    /// natively to bytecode.
    /// </summary>
    InterpFallback,

    // -- Exit -----------------------------------------------------------------
    /// <summary>
    /// Pop exit-code value; call Environment.Exit(code).
    /// Condition (if any) is handled at compile time via a JumpF guard.
    /// </summary>
    Exit,

    // -- Misc -----------------------------------------------------------------
    Nop,
    Halt,
}
