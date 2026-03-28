using System.Runtime.InteropServices;
using kiwi.Parsing;
using kiwi.Parsing.AST;
using kiwi.Runtime;
using kiwi.Runtime.Builtin.Handler;
using kiwi.Runtime.Builtin.Operation;
using kiwi.Runtime.Builtin.Dispatcher;
using kiwi.Tracing;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.VM;

/// <summary>
/// Internal iterator state used by ForIterInit / ForIterNext.
/// </summary>
internal sealed class ForIterState
{
    public List<Value>? List;
    public Dictionary<Value, Value>? Hashmap;
    public byte[]? Bytes;
    public string? StringData; // lazy string iteration: avoids upfront char-Value allocation
    public int     Index;
    public List<Value>? HashmapKeys; // pre-enumerated keys for hashmap iteration
    public GeneratorRef? Generator;  // VM-native generator iteration
}

/// <summary>
/// Stack-based bytecode virtual machine for Kiwi.
///
/// Execution model:
///   - Shared value stack: _stack[0..MaxStack-1]
///   - Frame stack:        _frames[0..MaxFrames-1]
///   - Locals for a frame live at absolute indices [frame.StackBase .. frame.StackBase + Chunk.LocalCount - 1]
///   - Temporaries live above that region (tracked by _sp)
///   - On Call: new frame's StackBase = _sp - argCount (args were pushed into correct slots)
///   - On Return: _sp is restored to StackBase - 1, return value pushed there
/// </summary>
public sealed class KiwiVM
{
    // -- Limits ----------------------------------------------------------------
    private const int MaxStack  = 8192;
    private const int MaxFrames = 512;

    // -- Value stack -----------------------------------------------------------
    private readonly Value[] _stack = new Value[MaxStack];
    private int _sp; // next free slot

    // -- Frame stack -----------------------------------------------------------
    private readonly VMFrame[] _frames = new VMFrame[MaxFrames];
    private int _frameCount;

    // -- Open upvalue linked list ----------------------------------------------
    private Upvalue? _openUpvalues;

    // -- Native object store (for ForIterState etc.) ---------------------------
    private int _nextNativeId;
    private readonly Dictionary<int, object> _nativeObjects = [];

    // -- Caught error (for LoadCatchError inside catch bodies) ------------------
    private KiwiError? _caughtError;

    // -- Splat-argument adjustment side-stack ----------------------------------
    // Each SplatReset pushes 0; each SplatPush adds (list.Count-1) to the top;
    // CallSplat/MethodCallSplat pop and add to the static argc operand.
    // Using a stack (not a single int) so nested splat calls are independent.
    private readonly Stack<int> _splatAdjStack = new();

    // -- Struct / Package definition state -------------------------------------
    private readonly Stack<KStruct> _pendingStructs = new();

    // -- Name resolution cache (LoadGlobal fast-path) --------------------------
    // Caches resolved Values for stable global names (functions, constants,
    // packages, structs).  Invalidated on DefFunc and StoreGlobal so that
    // redefined functions and reassigned globals are never served stale.
    private readonly Dictionary<string, Value> _nameCache = new(StringComparer.Ordinal);

    // -- Closure ID counter (replaces Guid.NewGuid) ----------------------------
    // Must be global (not per-VM) so that task VMs don't collide with each other
    // or with the main VM in the shared _context.Lambdas dictionary.
    private static int _globalClosureIdCounter;


    // -- Shared runtime state (shared with the tree-walking interpreter) -------
    private readonly Interpreter _interp;
    private readonly Scope       _globals;   // = interp._globalScope
    private readonly KContext    _context;   // = interp.Context

    // -- Thread-local slot -----------------------------------------------------
    [ThreadStatic]
    public static KiwiVM? Current;

    // -- Construction ----------------------------------------------------------

    public KiwiVM(Interpreter interp)
    {
        _interp  = interp;
        _globals = interp.GetGlobalScope();
        _context = interp.Context;
        Current  = this;
    }

    /// <summary>
    /// Constructor for task-thread VMs: uses an explicit globals scope (typically the
    /// main interpreter's global scope, obtained from <see cref="Callable.CapturedScope"/>)
    /// rather than the fresh task-interpreter scope.
    /// </summary>
    public KiwiVM(Interpreter interp, Scope globals)
    {
        _interp  = interp;
        _globals = globals;
        _context = interp.Context;
        Current  = this;
    }

    // -- Public entry points ---------------------------------------------------

    /// <summary>
    /// Execute a compiled top-level chunk and return the final value.
    /// </summary>
    public Value Execute(Chunk chunk)
    {
        PushFrame("<main>", chunk, stackBase: 0, upvalues: []);
        _frames[0].IsGlobalScope = true;
        // Initialize locals to null (LocalCount slots)
        for (int i = 0; i < chunk.LocalCount; i++)
            _stack[i] = Value.Default;
        _sp = chunk.LocalCount;

        return RunLoop();
    }

    /// <summary>
    /// Invoke a VM-compiled callable (KFunction or KLambda with a VMChunk) from within
    /// an interpreter-side call (e.g. an interpreted lambda calling a VM-compiled function).
    /// Sets up a new VM frame, runs to completion, and returns the result.
    /// </summary>
    /// <summary>
    /// Execute a top-level program chunk inline (as if it were included into the current program).
    /// Compiles the included file's packages/functions into VM bytecode and discards any return value.
    /// </summary>
    public void ExecuteInclude(Chunk chunk, Token token)
    {
        int savedSp         = _sp;
        int savedFrameCount = _frameCount;
        int calleeBase      = _sp;
        var savedBelow      = calleeBase > 0 ? _stack[calleeBase - 1] : Value.Default;

        // Reserve space for locals
        for (int i = 0; i < chunk.LocalCount; i++)
        {
            if (calleeBase + i < _stack.Length) _stack[calleeBase + i] = Value.Default;
        }
        _sp = calleeBase + chunk.LocalCount;

        int stopAt = _frameCount;
        PushFrame("<include>", chunk, calleeBase, [], token);
        _frames[_frameCount - 1].IsGlobalScope = true;

        try
        {
            RunLoop(stopAt);
        }
        catch
        {
            _sp         = savedSp;
            _frameCount = savedFrameCount;
            throw;
        }

        // Restore the slot potentially clobbered by Return's Push(result)
        if (calleeBase > 0) _stack[calleeBase - 1] = savedBelow;
        _sp = savedSp; // discard return value, restore stack
    }

    public Value InvokeVMCallable(Callable callable, List<Value> args, Token token, InstanceRef? instance = null)
    {
        var sub      = callable is KFunction kf ? kf.VMChunk! : ((KLambda)callable).VMChunk!;
        var upvalues = callable is KFunction kf2 ? (kf2.VMUpvalues ?? []) : (((KLambda)callable).VMUpvalues ?? []);

        int calleeBase      = _sp;
        // When called re-entrantly from C# (e.g. LambdaMap), the Return handler writes
        // the result to _stack[calleeBase - 1] via Push(result). Save and restore that
        // slot so the caller's temp stack is not clobbered.
        var savedBelow      = calleeBase > 0 ? _stack[calleeBase - 1] : Value.Default;
        int savedFrameCount = _frameCount;

        // Push args onto the stack
        foreach (var a in args) Push(a);
        // Zero-initialize remaining local slots beyond args
        SetupCalleeLocals(sub, calleeBase, args.Count);

        int stopAt = _frameCount; // run only until this frame completes
        PushFrame(sub.Name, sub, calleeBase, upvalues, token, instance);
        Value r;
        try
        {
            r = RunLoop(stopAt);
        }
        catch
        {
            // Restore VM state so subsequent InvokeVMCallable calls are not affected.
            _sp         = calleeBase;
            _frameCount = savedFrameCount;
            if (calleeBase > 0) _stack[calleeBase - 1] = savedBelow;
            throw;
        }

        // Restore the slot clobbered by the Return handler's Push(result).
        if (calleeBase > 0) _stack[calleeBase - 1] = savedBelow;
        return r;
    }

    // -- Frame management ------------------------------------------------------

    private void PushFrame(string name, Chunk chunk, int stackBase, Upvalue[] upvalues,
                           Token? callSiteToken = null, InstanceRef? self = null)
    {
        if (_frameCount >= MaxFrames)
            throw new RuntimeError(callSiteToken ?? Token.Eof,
                "Stack overflow.", []);

        var frame = new VMFrame(name, chunk, stackBase, upvalues)
        {
            CallSiteToken = callSiteToken,
            Self          = self,
            StructName    = self?.StructName ?? string.Empty
        };
        _frames[_frameCount++] = frame;
    }

    private VMFrame CurrentFrame => _frames[_frameCount - 1];

    // -- Stack helpers ---------------------------------------------------------

    private void Push(Value v)
    {
        if (_sp >= MaxStack)
            throw new RuntimeError(Token.Eof, "Stack overflow.", []);
        _stack[_sp++] = v;
    }

    private Value Pop()  => _stack[--_sp];
    private Value Peek() => _stack[_sp - 1];

    // -- Upvalue management ----------------------------------------------------

    private Upvalue CaptureUpvalue(int absoluteSlot)
    {
        // Reuse existing open upvalue if one already exists for this slot
        Upvalue? prev = null;
        var cur = _openUpvalues;
        while (cur != null && cur.AbsoluteSlot > absoluteSlot)
        {
            prev = cur;
            cur  = cur.Next;
        }
        if (cur != null && cur.AbsoluteSlot == absoluteSlot)
            return cur;

        var uv = new Upvalue(_stack, absoluteSlot) { Next = cur };
        if (prev == null) _openUpvalues = uv;
        else              prev.Next     = uv;
        return uv;
    }

    private void CloseUpvalues(int fromAbsoluteSlot)
    {
        while (_openUpvalues != null && _openUpvalues.AbsoluteSlot >= fromAbsoluteSlot)
        {
            _openUpvalues.Close();
            _openUpvalues = _openUpvalues.Next;
        }
    }

    // -- Main execution loop ---------------------------------------------------

    private Value RunLoop() => RunLoop(0);

    private Value RunLoop(int stopAt)
    {
        Value result = Value.Default;

        while (_frameCount > stopAt)
        {
            var frame    = _frames[_frameCount - 1];
            var codeSpan = CollectionsMarshal.AsSpan(frame.Chunk.Code);

            try
            {
            while (frame.IP < codeSpan.Length)
            {
                var instr = codeSpan[frame.IP++];
                var op    = instr.Op;
                var A     = instr.A;
                var B     = instr.B;

                switch (op)
                {
                    // -- Constants ----------------------------------------
                    case Opcode.Null:  Push(Value.Default); break;
                    case Opcode.True:  Push(Value.True);  break;
                    case Opcode.False: Push(Value.False); break;
                    case Opcode.Const: Push(frame.Chunk.Constants[A]);   break;

                    // -- Locals -------------------------------------------
                    case Opcode.LoadLocal:
                        Push(_stack[frame.StackBase + A]);
                        break;
                    case Opcode.StoreLocal:
                        _stack[frame.StackBase + A] = Pop();
                        break;

                    // -- Globals ------------------------------------------
                    case Opcode.LoadGlobal:
                    {
                        var name = frame.Chunk.Names[A];
                        // Fast path: cached resolution from a prior lookup
                        if (_nameCache.TryGetValue(name, out var cachedVal))
                        {
                            Push(cachedVal);
                            break;
                        }

                        // Slow path: full resolution (result cached for stable entries)
                        if (_context.HasFunction(name))
                        {
                            // A user-assigned global (e.g. an object) shadows a stdlib function
                            // of the same name, matching the tree-walker's scope-first lookup.
                            if (_globals.TryGet(name, out var gv) && !gv.IsNull())
                            {
                                Push(gv);
                            }
                            else
                            {
                                var fn = _context.Functions[name];
                                var v  = Value.CreateLambda(new LambdaRef { Identifier = name, VMChunk = fn.VMChunk, VMUpvalues = fn.VMUpvalues });
                                _nameCache[name] = v; // cached; invalidated on DefFunc
                                Push(v);
                            }
                        }
                        else if (_context.HasLambda(name))
                        {
                            // Prefer the stored globals value (has the correct VMChunk reference).
                            _globals.TryGet(name, out var storedLambda);
                            var v = storedLambda.IsNull()
                                ? Value.CreateLambda(_context.Lambdas[name].Ref)
                                : storedLambda;
                            // Don't cache lambdas: they may be reassigned via StoreGlobal.
                            Push(v);
                        }
                        else if (_context.HasMappedLambda(name))
                        {
                            var mapped = _context.LambdaTable[name];
                            var klambda = _context.Lambdas[mapped];
                            var lref = klambda.Ref.Identifier.Length > 0
                                ? klambda.Ref
                                : new LambdaRef { Identifier = mapped, VMChunk = klambda.VMChunk, VMUpvalues = klambda.VMUpvalues };
                            Push(Value.CreateLambda(lref));
                        }
                        else if (_context.HasConstant(name))
                        {
                            var v = _context.Constants[name];
                            _nameCache[name] = v; // constants never change
                            Push(v);
                        }
                        else if (_context.HasPackageVariable(name))
                        {
                            Push(_context.PackageVariables[name]); // mutable — don't cache
                        }
                        else if (_context.HasStruct(name))
                        {
                            var v = Value.CreateStruct(new StructRef { Identifier = name });
                            _nameCache[name] = v; // struct definitions are stable
                            Push(v);
                        }
                        else if (_globals.TryGet(name, out var gv) && !gv.IsNull())
                        {
                            Push(gv); // user-assigned global shadows package names
                            // Don't cache: mutable
                        }
                        else if (_context.HasPackage(name))
                        {
                            var v = Value.CreatePackage(name);
                            _nameCache[name] = v; // packages never change
                            Push(v);
                        }
                        else if (!string.IsNullOrEmpty(frame.Chunk.PackagePrefix))
                        {
                            // Package-internal sibling call: try pkg::name fallback.
                            var qualName = frame.Chunk.PackagePrefix + "::" + name;
                            if (_context.HasFunction(qualName))
                            {
                                var fn = _context.Functions[qualName];
                                var v  = Value.CreateLambda(new LambdaRef { Identifier = qualName, VMChunk = fn.VMChunk, VMUpvalues = fn.VMUpvalues });
                                Push(v);
                            }
                            else if (_context.HasStruct(qualName))
                            {
                                var v = Value.CreateStruct(new StructRef { Identifier = qualName });
                                Push(v);
                            }
                            else if (frame.Self != null)
                            {
                                Push(Value.CreateLambda(new LambdaRef { Identifier = name, BoundSelf = frame.Self }));
                            }
                            else
                            {
                                throw new VariableUndefinedError(frame.GetToken(), name);
                            }
                        }
                        else
                        {
                            // When not found globally and we're inside a struct method,
                            // treat it as an implicit @.name() call on self.
                            if (frame.Self != null)
                                Push(Value.CreateLambda(new LambdaRef { Identifier = name, BoundSelf = frame.Self }));
                            else
                                throw new VariableUndefinedError(frame.GetToken(), name);
                            // Don't cache mutable globals
                        }
                        break;
                    }
                    case Opcode.StoreGlobal:
                    {
                        var name = frame.Chunk.Names[A];
                        var v    = Pop();
                        if (v.IsLambda())
                        {
                            var lr = v.GetLambda();
                            if (!string.IsNullOrEmpty(lr.Identifier) && _context.HasLambda(lr.Identifier))
                                _context.Lambdas[name] = _context.Lambdas[lr.Identifier];
                        }
                        _globals.Assign(name, v);
                        _nameCache.Remove(name); // invalidate any cached resolution
                        // Mirror package-level variable into PackageVariables for pkg::name access.
                        if (_interp.PackageStack.Count > 0)
                        {
                            var qualName = string.Join("::", _interp.PackageStack.Reverse()) + "::" + name;
                            _context.PackageVariables[qualName] = v;
                        }
                        break;
                    }

                    // -- Upvalues -----------------------------------------
                    case Opcode.LoadUpvalue:
                        Push(frame.Upvalues[A].Get());
                        break;
                    case Opcode.StoreUpvalue:
                        frame.Upvalues[A].Set(Pop());
                        break;
                    case Opcode.CloseUpvalue:
                        CloseUpvalues(frame.StackBase + A);
                        break;

                    // -- Stack ops -----------------------------------------
                    case Opcode.Pop: _sp--; break;
                    case Opcode.Dup: Push(_stack[_sp - 1]); break;

                    // -- Arithmetic ----------------------------------------
                    case Opcode.Add:
                    {
                        var r = Pop(); var l = Pop();
                        Push(TryStructOpOverload(frame.GetToken(), l, r, TokenName.Ops_Add) ?? MathOp.Add(frame.GetToken(), ref l, ref r));
                        break;
                    }
                    case Opcode.Sub:
                    {
                        var r = Pop(); var l = Pop();
                        Push(TryStructOpOverload(frame.GetToken(), l, r, TokenName.Ops_Subtract) ?? MathOp.Sub(frame.GetToken(), ref l, ref r));
                        break;
                    }
                    case Opcode.Mul:
                    {
                        var r = Pop(); var l = Pop();
                        Push(TryStructOpOverload(frame.GetToken(), l, r, TokenName.Ops_Multiply) ?? MathOp.Mul(frame.GetToken(), ref l, ref r));
                        break;
                    }
                    case Opcode.Div:
                    {
                        var r = Pop(); var l = Pop();
                        Push(TryStructOpOverload(frame.GetToken(), l, r, TokenName.Ops_Divide) ?? MathOp.Div(frame.GetToken(), ref l, ref r));
                        break;
                    }
                    case Opcode.Mod:
                    {
                        var r = Pop(); var l = Pop();
                        Push(TryStructOpOverload(frame.GetToken(), l, r, TokenName.Ops_Modulus) ?? MathOp.Mod(frame.GetToken(), ref l, ref r));
                        break;
                    }
                    case Opcode.Pow:
                    {
                        var r = Pop(); var l = Pop();
                        Push(TryStructOpOverload(frame.GetToken(), l, r, TokenName.Ops_Exponent) ?? MathOp.Exp(frame.GetToken(), ref l, ref r));
                        break;
                    }
                    case Opcode.Neg:
                    {
                        var v = Pop();
                        Push(MathOp.Negate(frame.GetToken(), ref v));
                        break;
                    }

                    // -- Bitwise -------------------------------------------
                    case Opcode.BAnd:
                    {
                        var r = Pop(); var l = Pop();
                        Push(BitwiseOp.And(frame.GetToken(), ref l, ref r));
                        break;
                    }
                    case Opcode.BOr:
                    {
                        var r = Pop(); var l = Pop();
                        Push(BitwiseOp.Or(frame.GetToken(), ref l, ref r));
                        break;
                    }
                    case Opcode.BXor:
                    {
                        var r = Pop(); var l = Pop();
                        Push(BitwiseOp.Xor(frame.GetToken(), ref l, ref r));
                        break;
                    }
                    case Opcode.BNot:
                    {
                        var v = Pop();
                        Push(BitwiseOp.Not(frame.GetToken(), ref v));
                        break;
                    }
                    case Opcode.BLSh:
                    {
                        var r = Pop(); var l = Pop();
                        Push(BitwiseOp.Leftshift(frame.GetToken(), ref l, ref r));
                        break;
                    }
                    case Opcode.BRSh:
                    {
                        var r = Pop(); var l = Pop();
                        Push(BitwiseOp.Rightshift(frame.GetToken(), ref l, ref r));
                        break;
                    }
                    case Opcode.BURSh:
                    {
                        var r = Pop(); var l = Pop();
                        Push(BitwiseOp.UnsignedRightshift(frame.GetToken(), ref l, ref r));
                        break;
                    }

                    // -- Comparison ----------------------------------------
                    case Opcode.Eq:
                    {
                        var r = Pop(); var l = Pop();
                        Push(Value.CreateBoolean(ComparisonOp.Equal(ref l, ref r)));
                        break;
                    }
                    case Opcode.NEq:
                    {
                        var r = Pop(); var l = Pop();
                        Push(ComparisonOp.NotEqual(ref l, ref r));
                        break;
                    }
                    case Opcode.Lt:
                    {
                        var r = Pop(); var l = Pop();
                        Push(ComparisonOp.LessThan(ref l, ref r));
                        break;
                    }
                    case Opcode.LtE:
                    {
                        var r = Pop(); var l = Pop();
                        Push(ComparisonOp.LessThanOrEqual(ref l, ref r));
                        break;
                    }
                    case Opcode.Gt:
                    {
                        var r = Pop(); var l = Pop();
                        Push(ComparisonOp.GreaterThan(ref l, ref r));
                        break;
                    }
                    case Opcode.GtE:
                    {
                        var r = Pop(); var l = Pop();
                        Push(ComparisonOp.GreaterThanOrEqual(ref l, ref r));
                        break;
                    }
                    case Opcode.In:
                    {
                        var r = Pop(); var l = Pop();
                        Push(ComparisonOp.In(frame.GetToken(), ref l, ref r));
                        break;
                    }

                    // -- Logical -------------------------------------------
                    case Opcode.Not:
                    {
                        var v = Pop();
                        Push(LogicalOp.Not(ref v));
                        break;
                    }
                    case Opcode.JumpAnd:
                    {
                        // Peek: if falsy, jump to A (leave value); else pop and continue
                        if (!BooleanOp.IsTruthy(Peek()))
                            frame.IP = A;
                        else
                            _sp--;
                        break;
                    }
                    case Opcode.JumpOr:
                    {
                        // Peek: if truthy, jump to A (leave value); else pop and continue
                        if (BooleanOp.IsTruthy(Peek()))
                            frame.IP = A;
                        else
                            _sp--;
                        break;
                    }

                    // -- Null coalesce -------------------------------------
                    case Opcode.NullCoalesce:
                    {
                        var r = Pop(); var l = Pop();
                        Push(ComparisonOp.NullCoalesce(ref l, ref r));
                        break;
                    }

                    // -- Concat (string ..) --------------------------------
                    case Opcode.Concat:
                    {
                        var r = Pop(); var l = Pop();
                        var ls = l.IsString() ? l.GetString() : _interp.Serialize(l);
                        var rs = r.IsString() ? r.GetString() : _interp.Serialize(r);
                        Push(Value.CreateString(ls + rs));
                        break;
                    }

                    // -- Jumps ---------------------------------------------
                    case Opcode.Jump:
                        frame.IP = A;
                        break;
                    case Opcode.JumpF:
                        if (!BooleanOp.IsTruthy(Pop())) frame.IP = A;
                        break;
                    case Opcode.JumpT:
                        if (BooleanOp.IsTruthy(Pop())) frame.IP = A;
                        break;

                    // -- Splat ------------------------------------------------
                    case Opcode.SplatReset:
                        _splatAdjStack.Push(0);
                        break;

                    case Opcode.SplatPush:
                    {
                        var v = Pop();
                        if (v.IsList())
                        {
                            var list = v.GetList();
                            foreach (var item in list) Push(item);
                            if (_splatAdjStack.Count > 0)
                                _splatAdjStack.Push(_splatAdjStack.Pop() + list.Count - 1);
                        }
                        else
                        {
                            Push(v); // single value — no adjustment (count stays as 1)
                        }
                        break;
                    }

                    // -- Call ---------------------------------------------
                    case Opcode.Call:
                    case Opcode.CallSplat:
                    {
                        int argc = op == Opcode.CallSplat
                            ? A + (_splatAdjStack.Count > 0 ? _splatAdjStack.Pop() : 0)
                            : A;
                        // Stack: [..., func, arg0, ..., argN-1]
                        // func is at _sp - argc - 1
                        int funcSlot = _sp - argc - 1;
                        var funcVal  = _stack[funcSlot];
                        // B holds the name index encoded by CompileFuncCall (0 = unknown)
                        string? callName = B > 0 ? frame.Chunk.Names[B] : null;
                        if (DoCall(funcVal, argc, funcSlot, frame.GetToken(), callName))
                            goto nextFrame; // new VM frame pushed; switch to it
                        break;             // interpreter handled in-place; continue current frame
                    }

                    // -- CallMethod ----------------------------------------
                    case Opcode.CallMethod:
                    case Opcode.MethodCallSplat:
                    {
                        int argc       = op == Opcode.MethodCallSplat
                            ? A + (_splatAdjStack.Count > 0 ? _splatAdjStack.Pop() : 0)
                            : A;
                        var methodName = frame.Chunk.Names[B];
                        // Stack: [..., obj, arg0, ..., argN-1]
                        int objSlot = _sp - argc - 1;
                        var obj     = _stack[objSlot];

                        // Collect args
                        var args = new List<Value>(argc);
                        for (int i = _sp - argc; i < _sp; i++) args.Add(_stack[i]);
                        _sp = objSlot; // collapse obj+args

                        // Dispatch
                        var callResult = _interp.DispatchMethod(obj, methodName, args, frame.GetToken());
                        Push(callResult);
                        break;
                    }

                    // -- CallNamed -----------------------------------------
                    case Opcode.CallNamed:
                    {
                        // A = argc, B = argNameSetIdx
                        int argc       = A;
                        var argNames   = frame.Chunk.ArgNameSets[B];
                        int funcSlot   = _sp - argc - 1;
                        var funcVal    = _stack[funcSlot];
                        if (DoCallNamed(funcVal, argc, funcSlot, argNames, frame.GetToken()))
                            goto nextFrame;
                        break;
                    }

                    // -- CallMethodNamed -----------------------------------
                    case Opcode.CallMethodNamed:
                    {
                        // A = argc, B = packed(methodNameIdx:lower16, argNameSetIdx:upper16)
                        int argc          = A;
                        int methodNameIdx = B & 0xFFFF;
                        int nameSetIdx    = (B >> 16) & 0xFFFF;
                        var methodName    = frame.Chunk.Names[methodNameIdx];
                        var argNames      = frame.Chunk.ArgNameSets[nameSetIdx];
                        int objSlot       = _sp - argc - 1;
                        var obj           = _stack[objSlot];
                        int calleeBase    = objSlot + 1;

                        // For struct instances resolve named args against the method signature
                        if (obj.IsObject())
                        {
                            var inst   = obj.GetObject();
                            var sName  = inst.StructName;
                            KFunction? method = null;
                            if (_context.HasStruct(sName))
                            {
                                var struc = _context.Structs[sName];
                                struc.Methods.TryGetValue(methodName, out method);
                                if (method == null && !string.IsNullOrEmpty(struc.BaseStruct)
                                    && _context.HasStruct(struc.BaseStruct))
                                    _context.Structs[struc.BaseStruct].Methods.TryGetValue(methodName, out method);
                            }
                            if (method != null)
                            {
                                int newArgc = ApplyNamedArgs(calleeBase, argc, argNames,
                                    method.Parameters, method.DefaultParameters,
                                    method.VariadicParamName, frame.GetToken(), methodName);
                                var reorderedArgs = CollectArgs(calleeBase, newArgc);
                                _sp = objSlot;
                                Push(_interp.DispatchMethod(obj, methodName, reorderedArgs, frame.GetToken()));
                                break;
                            }
                        }

                        // Non-struct or method not found: collect args in source order
                        var plainArgs = new List<Value>(argc);
                        for (int i = _sp - argc; i < _sp; i++) plainArgs.Add(_stack[i]);
                        _sp = objSlot;
                        Push(_interp.DispatchMethod(obj, methodName, plainArgs, frame.GetToken()));
                        break;
                    }

                    // -- Return --------------------------------------------
                    case Opcode.Return:
                    {
                        result = Pop();
                        CloseUpvalues(frame.StackBase);
                        if (frame.LocalFunctions != null)
                            foreach (var fn in frame.LocalFunctions)
                                _context.Functions.Remove(fn);
                        _sp = frame.StackBase > 0 ? frame.StackBase - 1 : 0;
                        _frameCount--;
                        if (_frameCount > 0) Push(result);
                        goto nextFrame;
                    }
                    case Opcode.ReturnNull:
                    {
                        result = Value.Default;
                        CloseUpvalues(frame.StackBase);
                        if (frame.LocalFunctions != null)
                            foreach (var fn in frame.LocalFunctions)
                                _context.Functions.Remove(fn);
                        _sp = frame.StackBase > 0 ? frame.StackBase - 1 : 0;
                        _frameCount--;
                        if (_frameCount > 0) Push(result);
                        goto nextFrame;
                    }

                    // -- DefFunc -------------------------------------------
                    case Opcode.DefFunc:
                    {
                        var sub      = frame.Chunk.SubChunks[A];
                        var rawName  = frame.Chunk.Names[B];

                        // Qualify the name with the current package prefix (if any).
                        var pkgPrefix = _interp.PackageStack.Count > 0
                            ? string.Join("::", _interp.PackageStack.Reverse())
                            : string.Empty;
                        var funcName = pkgPrefix.Length > 0 ? pkgPrefix + "::" + rawName : rawName;

                        // Store the package prefix on the sub-chunk so LoadGlobal can
                        // resolve unqualified sibling calls when executing this function.
                        sub.PackagePrefix = pkgPrefix;

                        // Build upvalue array for the closure (even for named functions)
                        var upvalues = BuildUpvalues(sub, frame);

                        var kfunc = new KFunction(BuildFunctionNode(sub, funcName))
                        {
                            Name        = funcName,
                            VMChunk     = sub,
                            VMUpvalues  = upvalues,
                            IsGenerator = sub.IsGenerator,
                        };
                        foreach (var (pn, _) in kfunc.Decl.Parameters)
                            kfunc.Parameters.Add(new KeyValuePair<string, Value>(pn, Value.Default));
                        foreach (var pn in sub.DefaultParamNames)
                            kfunc.DefaultParameters.Add(pn);
                        if (!string.IsNullOrEmpty(sub.VariadicParamName))
                            kfunc.VariadicParamName = sub.VariadicParamName;
                        kfunc.CapturedScope = _globals;

                        _context.Functions[funcName] = kfunc;
                        _nameCache.Remove(funcName); // invalidate stale cached lambda ref

                        // If defined inside another function frame (but not a global-scope
                        // include frame), track for cleanup on return.
                        if (_frameCount > 1 && !_frames[_frameCount - 1].IsGlobalScope)
                            _frames[_frameCount - 1].TrackLocalFunction(funcName);

                        break;
                    }

                    // -- MakeClosure ---------------------------------------
                    case Opcode.MakeClosure:
                    {
                        var sub      = frame.Chunk.SubChunks[A];
                        // Inherit the enclosing function's package prefix so that
                        // LoadGlobal inside this closure can resolve sibling functions.
                        if (!string.IsNullOrEmpty(frame.Chunk.PackagePrefix) && string.IsNullOrEmpty(sub.PackagePrefix))
                            sub.PackagePrefix = frame.Chunk.PackagePrefix;
                        var upvalues = BuildUpvalues(sub, frame);

                        var klambda = new KLambda(BuildLambdaNode(sub))
                        {
                            VMChunk    = sub,
                            VMUpvalues = upvalues,
                            CapturedScope = _globals
                        };
                        for (int i = 0; i < sub.Arity; i++)
                            klambda.Parameters.Add(new KeyValuePair<string, Value>(sub.ParamNames[i], Value.Default));
                        foreach (var pn in sub.DefaultParamNames)
                            klambda.DefaultParameters.Add(pn);
                        if (!string.IsNullOrEmpty(sub.VariadicParamName))
                            klambda.VariadicParamName = sub.VariadicParamName;

                        var id   = $"__lam_{Interlocked.Increment(ref _globalClosureIdCounter)}";
                        var lref = new LambdaRef { Identifier = id, VMChunk = sub, VMUpvalues = upvalues };
                        klambda.Ref = lref;
                        _context.Lambdas[id] = klambda;
                        Push(Value.CreateLambda(lref));
                        break;
                    }

                    // -- Collections ---------------------------------------
                    case Opcode.BuildList:
                    {
                        var list = new List<Value>(A);
                        for (int i = _sp - A; i < _sp; i++) list.Add(_stack[i]);
                        _sp -= A;
                        Push(Value.CreateList(list));
                        break;
                    }
                    case Opcode.BuildHashmap:
                    {
                        var map = new Dictionary<Value, Value>(A);
                        // Pairs on stack: k0, v0, k1, v1, ...
                        int start = _sp - A * 2;
                        for (int i = 0; i < A; i++)
                            map[_stack[start + i * 2]] = _stack[start + i * 2 + 1];
                        _sp = start;
                        Push(Value.CreateHashmap(map));
                        break;
                    }
                    case Opcode.BuildRange:
                    {
                        var stop  = Pop();
                        var start = Pop();
                        if (!start.IsInteger() || !stop.IsInteger())
                            throw new RangeError(frame.GetToken(), "Range values must be integers.");
                        long s = start.GetInteger(), e = stop.GetInteger();
                        int  step = e < s ? -1 : 1;
                        var list = new List<Value>();
                        for (long i = s; i != e; i += step) list.Add(Value.CreateInteger(i));
                        list.Add(Value.CreateInteger(e));
                        Push(Value.CreateList(list));
                        break;
                    }

                    // -- Indexing ------------------------------------------
                    case Opcode.IndexGet:
                    {
                        var key = Pop();
                        var obj = Pop();
                        Push(_interp.GetIndex(obj, key, frame.GetToken()));
                        break;
                    }
                    case Opcode.IndexSet:
                    {
                        var val = Pop();
                        var key = Pop();
                        var obj = Pop();
                        _interp.SetIndex(obj, key, val, frame.GetToken());
                        Push(val);
                        break;
                    }
                    case Opcode.IndexOpAssign:
                    {
                        var rhs = Pop();
                        var key = Pop();
                        var obj = Pop();
                        var tok = frame.GetToken();
                        var old = _interp.GetIndex(obj, key, tok);
                        Value newVal = (Opcode)A switch
                        {
                            Opcode.Add  => MathOp.Add(tok, ref old, ref rhs),
                            Opcode.Sub  => MathOp.Sub(tok, ref old, ref rhs),
                            Opcode.Mul  => MathOp.Mul(tok, ref old, ref rhs),
                            Opcode.Div  => MathOp.Div(tok, ref old, ref rhs),
                            Opcode.Mod  => MathOp.Mod(tok, ref old, ref rhs),
                            Opcode.Pow  => MathOp.Exp(tok, ref old, ref rhs),
                            Opcode.BAnd => BitwiseOp.And(tok, ref old, ref rhs),
                            Opcode.BOr  => BitwiseOp.Or(tok, ref old, ref rhs),
                            Opcode.BXor => BitwiseOp.Xor(tok, ref old, ref rhs),
                            Opcode.BLSh => BitwiseOp.Leftshift(tok, ref old, ref rhs),
                            Opcode.BRSh => BitwiseOp.Rightshift(tok, ref old, ref rhs),
                            Opcode.BURSh=> BitwiseOp.UnsignedRightshift(tok, ref old, ref rhs),
                            _           => rhs
                        };
                        _interp.SetIndex(obj, key, newVal, tok);
                        Push(newVal);
                        break;
                    }
                    case Opcode.UnpackList:
                    {
                        int n   = A;
                        var val = Pop();
                        if (val.IsList())
                        {
                            var list = val.GetList();
                            for (int i = 0; i < n; i++)
                                Push(i < list.Count ? list[i] : Value.Default);
                        }
                        else
                        {
                            Push(val);
                            for (int i = 1; i < n; i++) Push(Value.Default);
                        }
                        break;
                    }
                    case Opcode.SliceGet:
                    {
                        Value? startV = null, stopV = null, stepV = null;
                        if ((A & 4) != 0) stepV  = Pop();
                        if ((A & 2) != 0) stopV  = Pop();
                        if ((A & 1) != 0) startV = Pop();
                        var obj = Pop();
                        Push(_interp.GetSlice(obj, startV, stopV, stepV, frame.GetToken()));
                        break;
                    }

                    case Opcode.SliceSet:
                    {
                        var rhs    = Pop();
                        Value? stepV  = (A & 4) != 0 ? Pop() : (Value?)null;
                        Value? stopV  = (A & 2) != 0 ? Pop() : (Value?)null;
                        Value? startV = (A & 1) != 0 ? Pop() : (Value?)null;
                        var obj = Pop();
                        _interp.SetSlice(obj, startV, stopV, stepV, rhs, frame.GetToken());
                        Push(rhs);
                        break;
                    }

                    // -- Member access -------------------------------------
                    case Opcode.GetMember:
                    {
                        var memberName = frame.Chunk.Names[A];
                        var obj        = Pop();
                        Push(_interp.GetMember(obj, memberName, frame.GetToken()));
                        break;
                    }
                    case Opcode.SetMember:
                    {
                        var memberName = frame.Chunk.Names[A];
                        var obj        = Pop();
                        var val        = Pop();
                        _interp.SetMember(obj, memberName, val, frame.GetToken());
                        Push(val);
                        break;
                    }

                    // -- Self ----------------------------------------------
                    case Opcode.LoadSelf:
                        Push(frame.Self != null ? Value.CreateObject(frame.Self) : Value.Default);
                        break;
                    case Opcode.LoadSelfAttr:
                    {
                        var attrName = frame.Chunk.Names[A];
                        if (frame.Self != null)
                        {
                            frame.Self.InstanceVariables.TryGetValue(attrName, out var v);
                            Push(v ?? Value.Default);
                        }
                        else Push(Value.Default);
                        break;
                    }
                    case Opcode.StoreSelfAttr:
                    {
                        var attrName = frame.Chunk.Names[A];
                        var val      = Pop();
                        if (frame.Self != null)
                            frame.Self.InstanceVariables[attrName] = val;
                        break;
                    }
                    case Opcode.LoadStaticAttr:
                    {
                        var attrName   = frame.Chunk.Names[A];
                        var structName = frame.StructName;
                        if (!string.IsNullOrEmpty(structName) && _context.HasStruct(structName))
                        {
                            _context.Structs[structName].StaticVariables.TryGetValue(attrName, out var v);
                            Push(v ?? Value.Default);
                        }
                        else Push(Value.Default);
                        break;
                    }
                    case Opcode.StoreStaticAttr:
                    {
                        var attrName   = frame.Chunk.Names[A];
                        var structName = frame.StructName;
                        var val        = Pop();
                        if (!string.IsNullOrEmpty(structName) && _context.HasStruct(structName))
                            _context.Structs[structName].StaticVariables[attrName] = val;
                        break;
                    }

                    // -- New object ----------------------------------------
                    case Opcode.NewObject:
                    {
                        var structName = frame.Chunk.Names[B];
                        int argc       = A;
                        var args       = new List<Value>(argc);
                        for (int i = _sp - argc; i < _sp; i++) args.Add(_stack[i]);
                        _sp -= argc;
                        Push(_interp.CreateObject(structName, args, frame.GetToken()));
                        break;
                    }

                    // -- Interpolation -------------------------------------
                    case Opcode.Interpolate:
                    {
                        var parts = new string[A];
                        for (int i = A - 1; i >= 0; i--)
                        {
                            var v = Pop();
                            parts[i] = _interp.Serialize(v);
                        }
                        Push(Value.CreateString(string.Concat(parts)));
                        break;
                    }

                    // -- Print ---------------------------------------------
                    case Opcode.Print:
                    {
                        var v         = Pop();
                        var text      = _interp.Serialize(v);
                        bool newline  = (A & 1) != 0;
                        bool stderr   = (A & 2) != 0;
                        var writer    = stderr ? Console.Error : Console.Out;
                        if (newline) writer.WriteLine(text);
                        else         writer.Write(text);
                        break;
                    }

                    case Opcode.PrintXy:
                    {
                        var y    = Pop();
                        var x    = Pop();
                        var v    = Pop();
                        var text = _interp.Serialize(v);
                        int col  = x.IsInteger() ? (int)x.GetInteger() : x.IsFloat() ? (int)x.GetFloat() : 1;
                        int row  = y.IsInteger() ? (int)y.GetInteger() : y.IsFloat() ? (int)y.GetFloat() : 1;
                        Console.SetCursorPosition(col - 1, row - 1);
                        Console.Write(text);
                        break;
                    }

                    // -- For iterator --------------------------------------
                    case Opcode.ForIterInit:
                    {
                        var coll = Pop();
                        var state = new ForIterState();
                        if (coll.IsList())
                        {
                            state.List  = coll.GetList();
                            state.Index = 0;
                        }
                        else if (coll.IsHashmap())
                        {
                            state.Hashmap     = coll.GetHashmap();
                            state.HashmapKeys = [..coll.GetHashmap().Keys];
                            state.Index       = 0;
                        }
                        else if (coll.IsBytes())
                        {
                            state.Bytes = coll.GetBytes();
                            state.Index = 0;
                        }
                        else if (coll.IsString())
                        {
                            // Lazy: store the raw string; chars are produced one at a time in ForIterNext
                            state.StringData = coll.GetString();
                            state.Index      = 0;
                        }
                        else if (coll.IsGenerator())
                        {
                            state.Generator = coll.GetGenerator();
                        }
                        else
                        {
                            // Unsupported collection type - push null state so we stop immediately
                            state.List  = [];
                            state.Index = 0;
                        }
                        var stateId = _nextNativeId++;
                        _nativeObjects[stateId] = state;
                        Push(Value.CreateInteger(stateId));
                        break;
                    }
                    case Opcode.ForIterNext:
                    {
                        // Peek at ForIterState id on top of stack
                        var stateVal = Peek();
                        var stateId  = (int)stateVal.GetInteger();
                        var state    = (ForIterState)_nativeObjects[stateId];
                        int numVars  = B;

                        bool done;
                        if (state.List != null)
                        {
                            done = state.Index >= state.List.Count;
                            if (!done)
                            {
                                var item = state.List[state.Index++];
                                if (numVars == 2)
                                {
                                    Push(Value.CreateInteger(state.Index - 1)); // key = index
                                    Push(item);                                  // val
                                }
                                else Push(item);
                            }
                        }
                        else if (state.Hashmap != null)
                        {
                            done = state.Index >= state.HashmapKeys!.Count;
                            if (!done)
                            {
                                var key = state.HashmapKeys![state.Index++];
                                state.Hashmap.TryGetValue(key, out var val);
                                if (numVars == 2)
                                {
                                    // Push val first so key ends up on top.
                                    // CompileForLoop pops top into valName (first var = key)
                                    // and next into idxName (second var = value).
                                    Push(val ?? Value.Default);
                                    Push(key);
                                }
                                else Push(key);
                            }
                        }
                        else if (state.StringData != null)
                        {
                            done = state.Index >= state.StringData.Length;
                            if (!done)
                            {
                                char c = state.StringData[state.Index++];
                                var  sv = Value.CreateString(c.ToString());
                                if (numVars == 2)
                                {
                                    Push(Value.CreateInteger(state.Index - 1)); // key = index
                                    Push(sv);
                                }
                                else Push(sv);
                            }
                        }
                        else if (state.Bytes != null)
                        {
                            done = state.Index >= state.Bytes.Length;
                            if (!done)
                            {
                                var b = state.Bytes[state.Index++];
                                Push(Value.CreateInteger(b));
                            }
                        }
                        else if (state.Generator != null)
                        {
                            var (hasValue, genVal) = state.Generator.Next();
                            done = !hasValue;
                            if (!done)
                            {
                                if (numVars == 2)
                                {
                                    Push(Value.CreateInteger(state.Index++)); // key = position
                                    Push(genVal);
                                }
                                else Push(genVal);
                            }
                            else
                            {
                                state.Generator.Dispose();
                            }
                        }
                        else
                        {
                            done = true;
                        }

                        if (done)
                        {
                            _nativeObjects.Remove(stateId);
                            _sp--; // pop ForIterState id
                            frame.IP = A; // jump to after loop
                        }
                        break;
                    }

                    // -- Yield ---------------------------------------------
                    case Opcode.Yield:
                    {
                        var v = Pop();
                        _interp.YieldFromVM(v);
                        break;
                    }

                    // -- Throw ---------------------------------------------
                    case Opcode.Throw:
                    {
                        var v   = Pop();
                        var tok = frame.GetToken();
                        // Preserve structured error type from { error: ..., message: ... } hashmaps
                        string errType, errMsg;
                        if (v.IsHashmap())
                        {
                            var map     = v.GetHashmap();
                            var typeKey = Value.CreateString("error");
                            var msgKey  = Value.CreateString("message");
                            errType = map.TryGetValue(typeKey, out var tv) ? tv.GetString() : "Error";
                            errMsg  = map.TryGetValue(msgKey,  out var mv) ? mv.GetString() : string.Empty;
                        }
                        else if (v.IsString())
                        {
                            errType = KiwiError.DefaultErrorType;
                            errMsg  = v.GetString();
                        }
                        else
                        {
                            errType = KiwiError.DefaultErrorType;
                            errMsg  = _interp.Serialize(v);
                        }
                        throw new RuntimeError(tok, errType, errMsg, [frame.FormatTrace()]);
                    }

                    // -- Try handler management ----------------------------
                    case Opcode.PushTryHandler:
                    {
                        int catchIP   = A;
                        int finallyIP = B;
                        frame.PushTryHandler(catchIP, finallyIP, _sp);
                        break;
                    }
                    case Opcode.PopTryHandler:
                    {
                        frame.PopTryHandler(out _, out _, out _);
                        break;
                    }
                    case Opcode.LoadCatchError:
                    {
                        var e = _caughtError;
                        if (e == null) { Push(Value.Default); break; }
                        if (A == 0)
                        {
                            // Single-param catch: push hashmap {error, message}
                            var map = new Dictionary<Value, Value>
                            {
                                [Value.CreateString("error")]   = Value.CreateString(e.Type ?? "Error"),
                                [Value.CreateString("message")] = Value.CreateString(e.Message ?? string.Empty)
                            };
                            Push(Value.CreateHashmap(map));
                        }
                        else if (A == 1)
                        {
                            Push(Value.CreateString(e.Type ?? "Error"));
                        }
                        else
                        {
                            Push(Value.CreateString(e.Message ?? string.Empty));
                        }
                        // Clear after last binding: A==0 (single-param) or A==2 (second of two-param).
                        // A==1 (first of two-param) must NOT clear so the second LoadCatchError
                        // can still read the same error.
                        if (A != 1) _caughtError = null;
                        break;
                    }

                    // -- Interpreter fallback ------------------------------
                    case Opcode.InterpFallback:
                    {
                        var node = frame.Chunk.NodePool[A];
                        // Always push a fresh <vm-fallback> frame via InterpretNodeWithLocals so
                        // that:
                        //   1. CallStack is never empty when builtins call CallStack.Peek().
                        //   2. frame.Self is correctly propagated at ANY nesting depth — the old
                        //      conditional (only push when Count==0) left the wrong self on the
                        //      existing top frame for nested fallback calls.
                        var locals = new Dictionary<string, Value>(frame.Chunk.LocalNames.Count);
                        foreach (var (lname, slot) in frame.Chunk.LocalNames)
                            locals[lname] = _stack[frame.StackBase + slot];
                        var r = _interp.InterpretNodeWithLocals(node, locals, frame.Self);
                        // Sync any updated locals back to the VM stack.
                        foreach (var (lname, slot) in frame.Chunk.LocalNames)
                        {
                            if (locals.TryGetValue(lname, out var updated))
                                _stack[frame.StackBase + slot] = updated;
                        }
                        Push(r);
                        break;
                    }

                    // -- Builtin call --------------------------------------
                    case Opcode.CallBuiltin:
                    {
                        var node = (Parsing.AST.FunctionCallNode)frame.Chunk.NodePool[A];
                        var args = new List<Value>(B);
                        for (int i = 0; i < B; i++) args.Add(Pop());
                        args.Reverse();
                        Push(_interp.ExecuteBuiltin(node.Token, node.Op, args));
                        break;
                    }

                    // -- Export --------------------------------------------
                    case Opcode.Export:
                    {
                        var node = frame.Chunk.NodePool[A];
                        var locals = new Dictionary<string, Value>(frame.Chunk.LocalNames.Count);
                        foreach (var (lname, slot) in frame.Chunk.LocalNames)
                            locals[lname] = _stack[frame.StackBase + slot];
                        _interp.InterpretNodeWithLocals(node, locals, frame.Self);
                        break;
                    }

                    // -- Eval / Include ------------------------------------
                    case Opcode.Eval:
                    {
                        var node = frame.Chunk.NodePool[A];
                        var locals = new Dictionary<string, Value>(frame.Chunk.LocalNames.Count);
                        foreach (var (lname, slot) in frame.Chunk.LocalNames)
                            locals[lname] = _stack[frame.StackBase + slot];
                        Push(_interp.InterpretNodeWithLocals(node, locals, frame.Self));
                        break;
                    }

                    case Opcode.Include:
                    {
                        var node = frame.Chunk.NodePool[A];
                        var locals = new Dictionary<string, Value>(frame.Chunk.LocalNames.Count);
                        foreach (var (lname, slot) in frame.Chunk.LocalNames)
                            locals[lname] = _stack[frame.StackBase + slot];
                        _interp.InterpretNodeWithLocals(node, locals, frame.Self);
                        break;
                    }

                    // -- Type introspection --------------------------------
                    case Opcode.TypeOf:
                    {
                        var v = Pop();
                        Push(Value.CreateString(Typing.TypeRegistry.GetTypeName(v)));
                        break;
                    }

                    // -- Event bus -----------------------------------------
                    case Opcode.EventOn:
                    {
                        var callback  = Pop();
                        var eventName = Pop();
                        _interp.Context.Events.On(eventName.GetString(), callback, priority: A);
                        break;
                    }
                    case Opcode.EventOnce:
                    {
                        var callback  = Pop();
                        var eventName = Pop();
                        _interp.Context.Events.Once(eventName.GetString(), callback, priority: A);
                        break;
                    }
                    case Opcode.EventOff:
                    {
                        Value? callback = A == 1 ? Pop() : null;
                        var eventName = Pop();
                        _interp.Context.Events.Off(eventName.GetString(), callback);
                        break;
                    }
                    case Opcode.EventEmit:
                    {
                        // A args were pushed after the event-name; collect them bottom-first.
                        var args = new List<Value>(A);
                        for (int ei = 0; ei < A; ei++) args.Add(Value.Default);
                        for (int ei = A - 1; ei >= 0; ei--) args[ei] = Pop();
                        var eventName = Pop();
                        bool pushedFrame = _interp.CallStack.Count == 0;
                        if (pushedFrame) _interp.PushVMDispatchFrame(_globals);
                        List<Value> results;
                        try   { results = _interp.Context.Events.Emit(_interp, frame.GetToken(), eventName.GetString(), args); }
                        finally { if (pushedFrame) _interp.PopVMDispatchFrame(); }
                        Push(Value.CreateList(results));
                        break;
                    }

                    // -- Struct definition ---------------------------------
                    case Opcode.StructBegin:
                    {
                        var structName = frame.Chunk.Names[A];
                        bool isAbstract = (B & 1) != 0;
                        int  baseRaw    = B >> 1;
                        var  struc      = new KStruct { Name = structName, IsAbstract = isAbstract };
                        if (baseRaw > 0)
                        {
                            var baseName = frame.Chunk.Names[baseRaw - 1];
                            if (!_context.HasStruct(baseName))
                                throw new StructUndefinedError(frame.GetToken(), baseName);
                            struc.BaseStruct = baseName;
                        }
                        _pendingStructs.Push(struc);
                        _interp.StructStack.Push(structName);
                        break;
                    }
                    case Opcode.DefMethod:
                    {
                        var sub        = frame.Chunk.SubChunks[A];
                        bool isAbstract = (B & unchecked((int)0x80000000)) != 0;
                        bool isStatic   = (B & 0x40000000) != 0;
                        int  nameIdx   = B & 0x3FFFFFFF;
                        var  methodName = frame.Chunk.Names[nameIdx];

                        var upvalues = BuildUpvalues(sub, frame);
                        var kfunc = new KFunction(BuildFunctionNode(sub, methodName))
                        {
                            Name        = methodName,
                            VMChunk     = sub,
                            VMUpvalues  = upvalues,
                            IsAbstract  = isAbstract,
                            IsStatic    = isStatic,
                            CapturedScope = _globals
                        };
                        foreach (var (pn, _) in kfunc.Decl.Parameters)
                            kfunc.Parameters.Add(new KeyValuePair<string, Value>(pn, Value.Default));
                        foreach (var pn in sub.DefaultParamNames)
                            kfunc.DefaultParameters.Add(pn);
                        if (!string.IsNullOrEmpty(sub.VariadicParamName))
                            kfunc.VariadicParamName = sub.VariadicParamName;

                        var struc = _pendingStructs.Peek();
                        struc.Methods[methodName] = kfunc;
                        if (isAbstract)
                            struc.AbstractMethods.Add(methodName);
                        break;
                    }
                    case Opcode.InitStructStatic:
                    {
                        _pendingStructs.Peek().StaticVariables[frame.Chunk.Names[A]] = Pop();
                        break;
                    }
                    case Opcode.StructEnd:
                    {
                        var struc = _pendingStructs.Pop();
                        _interp.StructStack.Pop();
                        if (!struc.IsAbstract && !string.IsNullOrEmpty(struc.BaseStruct))
                        {
                            var baseStruc = _context.Structs[struc.BaseStruct];
                            foreach (var am in baseStruc.AbstractMethods)
                                if (!struc.Methods.ContainsKey(am))
                                    throw new AbstractMethodError(frame.GetToken(), struc.Name, am);
                        }
                        _context.Structs[struc.Name] = struc;
                        _nameCache.Remove(struc.Name);
                        // Also register under the qualified pkg::StructName so
                        // LoadGlobal("pkg::Struct") finds it when inside a package.
                        if (_interp.PackageStack.Count > 0)
                        {
                            var qualName = string.Join("::", _interp.PackageStack.Reverse()) + "::" + struc.Name;
                            _context.Structs[qualName] = struc;
                            _nameCache.Remove(qualName);
                        }
                        break;
                    }

                    // -- Package definition --------------------------------
                    case Opcode.PackageBegin:
                    {
                        var localName     = frame.Chunk.Names[A];
                        var qualifiedName = _interp.PackageStack.Count > 0
                            ? string.Join("::", _interp.PackageStack.Reverse()) + "::" + localName
                            : localName;
                        if (!_context.Packages.ContainsKey(qualifiedName))
                        {
                            // Store the original PackageNode AST so ImportPackage can retry
                            // via the tree-walking interpreter if activation failed.
                            var pkgAst = B < frame.Chunk.NodePool.Count
                                ? (PackageNode)frame.Chunk.NodePool[B]
                                : new PackageNode(null);
                            _context.Packages[qualifiedName] = new KPackage(pkgAst);
                        }
                        _interp.PackageStack.Push(localName);
                        break;
                    }
                    case Opcode.PackageEnd:
                    {
                        var localName     = _interp.PackageStack.Pop();
                        var qualifiedName = _interp.PackageStack.Count > 0
                            ? string.Join("::", _interp.PackageStack.Reverse()) + "::" + localName
                            : localName;
                        _interp.RegisterTypeBuiltins(qualifiedName);
                        _context.ImportedPackages.Add(qualifiedName);
                        _nameCache.Remove(qualifiedName);
                        break;
                    }
                    case Opcode.PackageAbort:
                    {
                        // Package activation failed: pop the stack but don't mark as imported.
                        // The stored PackageNode AST allows ImportPackage to retry later.
                        _interp.PackageStack.Pop();
                        break;
                    }

                    // -- Enum definition -----------------------------------
                    case Opcode.EnumBegin:
                    {
                        var enumName = frame.Chunk.Names[A];
                        _pendingStructs.Push(new KStruct { Name = enumName, IsEnum = true });
                        break;
                    }
                    case Opcode.DefEnumMember:
                    {
                        _pendingStructs.Peek().StaticVariables["@@" + frame.Chunk.Names[A]] = Pop();
                        break;
                    }
                    case Opcode.EnumEnd:
                    {
                        var kenum = _pendingStructs.Pop();
                        _context.Structs[kenum.Name] = kenum;
                        _nameCache.Remove(kenum.Name);
                        break;
                    }

                    // -- Exit ----------------------------------------------
                    case Opcode.Exit:
                    {
                        var exitVal = Pop();
                        int code = exitVal.IsInteger() ? Convert.ToInt32(exitVal.GetInteger()) : 1;
                        Environment.Exit(code);
                        break;
                    }

                    // -- Misc ----------------------------------------------
                    case Opcode.Nop: break;
                    case Opcode.Halt:
                        goto done;

                    default:
                        throw new RuntimeError(frame.GetToken(),
                            $"Unknown opcode: {op}", []);
                }
                continue;

            nextFrame:
                break; // re-enter outer while with new frame
            }
            } // end try
            catch (KiwiError e) when (frame.HasTryHandlers)
            {
                if (frame.PopTryHandler(out int catchIP, out _, out int savedSP))
                {
                    _sp = savedSP;
                    _caughtError = e;
                    frame.IP = catchIP;
                    continue; // re-enter outer loop → inner loop resumes at catchIP
                }
                throw;
            }
            // If we reach end of frame's code without Return/Halt → implicit null return
            if (_frameCount > 0 && _frames[_frameCount - 1] == frame)
            {
                CloseUpvalues(frame.StackBase);
                _sp = frame.StackBase > 0 ? frame.StackBase - 1 : 0;
                _frameCount--;
                if (_frameCount > 0) Push(Value.Default);
            }
        }

    done:
        return result;
    }

    // -- Named-argument helpers ------------------------------------------------

    /// <summary>
    /// Reorder the <paramref name="argc"/> arguments currently on the stack at
    /// [calleeBase .. calleeBase+argc-1] to match the callee's declared parameter order.
    ///
    /// After return the stack holds the reordered positional args followed by any
    /// variadic overflow (individual values, not pre-packed — SetupCalleeLocals handles
    /// the packing).  Returns the effective new argc.
    ///
    /// If parameter info is unavailable, returns <paramref name="argc"/> unchanged
    /// (args stay in source order, which is fine for interpreter-handled callees).
    /// </summary>
    private int ApplyNamedArgs(
        int calleeBase,
        int argc,
        string[] argNames,
        List<KeyValuePair<string, Value>> paramList,
        HashSet<string> defaultParams,
        string variadicParamName,
        Token token,
        string callName)
    {
        int paramCount = paramList.Count;

        // Snapshot raw args from the stack
        var rawArgs  = new Value[argc];
        var rawNames = new string[argc];
        for (int i = 0; i < argc; i++)
        {
            rawArgs[i]  = _stack[calleeBase + i];
            rawNames[i] = i < argNames.Length ? argNames[i] : string.Empty;
        }

        var reordered = new Value[paramCount];
        var filled    = new bool[paramCount]; // true once a slot has been assigned
        var consumed  = new bool[argc];       // true once a raw arg has been placed

        // Pass 1: place named args into their target slots
        for (int ri = 0; ri < argc; ri++)
        {
            var name = rawNames[ri];
            if (string.IsNullOrEmpty(name)) continue;
            bool found = false;
            for (int pi = 0; pi < paramCount; pi++)
            {
                if (paramList[pi].Key == name)
                {
                    reordered[pi] = rawArgs[ri];
                    filled[pi]    = true;
                    consumed[ri]  = true;
                    found         = true;
                    break;
                }
            }
            if (!found)
                throw new RuntimeError(token,
                    $"'{callName}': no parameter named '{name}'.", []);
        }

        // Pass 2: fill remaining param slots with positional args left-to-right
        int nextPos = 0;
        for (int pi = 0; pi < paramCount; pi++)
        {
            if (filled[pi]) continue;
            while (nextPos < argc && consumed[nextPos]) nextPos++;
            if (nextPos < argc && string.IsNullOrEmpty(rawNames[nextPos]))
            {
                reordered[pi] = rawArgs[nextPos];
                filled[pi]    = true;
                consumed[nextPos] = true;
                nextPos++;
            }
            else if (!defaultParams.Contains(paramList[pi].Key))
            {
                // Missing required argument — runtime error
                throw new ParameterCountMismatchError(token, callName, paramCount, argc);
            }
            // else: default param, leave Value.Default; callee's EmitDefaultInits fills it
        }

        // Pass 3: collect remaining positional args into variadic overflow
        var varargs = new List<Value>();
        if (!string.IsNullOrEmpty(variadicParamName))
        {
            for (int ri = 0; ri < argc; ri++)
            {
                if (!consumed[ri] && string.IsNullOrEmpty(rawNames[ri]))
                    varargs.Add(rawArgs[ri]);
            }
        }

        // Write reordered params back to stack
        for (int pi = 0; pi < paramCount; pi++)
            _stack[calleeBase + pi] = reordered[pi];

        // Write variadic overflow individually (SetupCalleeLocals will pack them)
        for (int vi = 0; vi < varargs.Count; vi++)
            _stack[calleeBase + paramCount + vi] = varargs[vi];

        int newArgc = paramCount + varargs.Count;
        _sp = calleeBase + newArgc;
        return newArgc;
    }

    /// <summary>
    /// Get the parameter list and variadic info for a callable value, if available.
    /// Returns false when the callable type is not inspectable (e.g. C# builtin).
    /// </summary>
    private bool TryGetCallableParams(
        Value funcVal,
        out Chunk? chunk,
        out Upvalue[] upvalues,
        out List<KeyValuePair<string, Value>> parms,
        out HashSet<string> defaults,
        out string variadicName)
    {
        chunk       = null;
        upvalues    = [];
        parms       = [];
        defaults    = [];
        variadicName = string.Empty;

        if (!funcVal.IsLambda()) return false;
        var lr = funcVal.GetLambda();

        // Directly-attached VMChunk (closure with upvalues)
        if (lr.VMChunk != null)
        {
            chunk    = lr.VMChunk;
            upvalues = lr.VMUpvalues ?? [];
            // Build param list from chunk metadata (Arity = non-variadic count)
            for (int i = 0; i < chunk.Arity; i++)
                parms.Add(new KeyValuePair<string, Value>(chunk.ParamNames[i], Value.Default));
            defaults     = chunk.DefaultParamNames;
            variadicName = chunk.VariadicParamName;
            return true;
        }

        if (_context.HasFunction(lr.Identifier))
        {
            var kf   = _context.Functions[lr.Identifier];
            chunk    = kf.VMChunk;
            upvalues = kf.VMUpvalues ?? [];
            parms    = kf.Parameters;
            defaults = kf.DefaultParameters;
            variadicName = kf.VariadicParamName;
            return true;
        }

        if (_context.HasLambda(lr.Identifier))
        {
            var kl   = _context.Lambdas[lr.Identifier];
            chunk    = kl.VMChunk;
            upvalues = kl.VMUpvalues ?? [];
            parms    = kl.Parameters;
            defaults = kl.DefaultParameters;
            variadicName = kl.VariadicParamName;
            return true;
        }

        return false;
    }

    /// <summary>
    /// Like <see cref="DoCall"/> but reorders args according to named-argument
    /// descriptors before entering the callee.
    /// </summary>
    private bool DoCallNamed(
        Value funcVal, int argc, int funcSlot,
        string[] argNames, Token token, string? callName = null)
    {
        if (funcVal.IsLambda() && funcVal.GetLambda().BoundSelf != null)
        {
            // Bound-self dispatch: route as plain method call (named args not meaningful)
            var lr      = funcVal.GetLambda();
            var selfVal = Value.CreateObject(lr.BoundSelf!);
            var args    = CollectArgs(funcSlot + 1, argc);
            _sp = funcSlot;
            Push(_interp.DispatchMethod(selfVal, lr.Identifier, args, token));
            return false;
        }

        if (TryGetCallableParams(funcVal, out var sub, out var upvalues,
                out var parms, out var defaults, out var variadicName))
        {
            int calleeBase = funcSlot + 1;
            string name    = callName ?? sub?.Name ?? "<lambda>";
            int newArgc = ApplyNamedArgs(calleeBase, argc, argNames,
                parms, defaults, variadicName, token, name);

            if (sub != null)
            {
                SetupCalleeLocals(sub, calleeBase, newArgc);
                PushFrame(sub.Name, sub, calleeBase, upvalues, token);
                return true;
            }

            // Interpreter-side callable (no VMChunk): pass reordered args
            var lr2  = funcVal.GetLambda();
            var argsL = CollectArgs(calleeBase, newArgc);
            _sp = funcSlot;
            var callable2 = _context.HasFunction(lr2.Identifier)
                ? (Callable)_context.Functions[lr2.Identifier]
                : _context.Lambdas[lr2.Identifier];
            Push(_interp.InvokeCallable(callable2, argsL, token, lr2.Identifier));
            return false;
        }

        // Can't determine param order — fall back to plain DoCall (source order)
        return DoCall(funcVal, argc, funcSlot, token, callName);
    }

    // -- Call dispatch ---------------------------------------------------------

    /// <summary>
    /// Dispatch a call. Returns <c>true</c> if a new VM frame was pushed (caller must
    /// <c>goto nextFrame</c>); returns <c>false</c> if the call was handled in-place
    /// by the interpreter (result already on stack, caller must NOT goto nextFrame).
    /// </summary>
    private bool DoCall(Value funcVal, int argc, int funcSlot, Token token, string? callName = null)
    {
        // StackBase of callee = position of first arg
        int calleeBase = funcSlot + 1;

        if (funcVal.IsLambda())
        {
            var lr = funcVal.GetLambda();

            // Bare method call on self inside a struct method: dispatch as @.method()
            if (lr.BoundSelf != null)
            {
                var selfVal = Value.CreateObject(lr.BoundSelf);
                var args    = CollectArgs(calleeBase, argc);
                _sp = funcSlot;
                Push(_interp.DispatchMethod(selfVal, lr.Identifier, args, token));
                return false;
            }


            // VM-compiled lambda?
            if (lr.VMChunk != null)
            {
                var sub      = lr.VMChunk;
                var upvalues = lr.VMUpvalues ?? [];
                // Generator functions must not execute directly — create a GeneratorRef instead.
                if (sub.IsGenerator)
                {
                    var args = CollectArgs(calleeBase, argc);
                    _sp = funcSlot;
                    // Resolve the KFunction by name so CreateGeneratorFromValues can run the body.
                    KFunction? kfGen = !string.IsNullOrEmpty(lr.Identifier) && _context.HasFunction(lr.Identifier)
                        ? _context.Functions[lr.Identifier] : null;
                    Push(kfGen != null
                        ? _interp.CreateGeneratorFromValues(kfGen, args, token)
                        : Value.Default);
                    return false;
                }
                SetupCalleeLocals(sub, calleeBase, argc);
                PushFrame(sub.Name, sub, calleeBase, upvalues, token);
                return true;
            }

            // Check if this is a named KFunction (e.g. package functions like math::round)
            if (_context.HasFunction(lr.Identifier))
            {
                var kf = _context.Functions[lr.Identifier];
                if (kf.VMChunk != null)
                {
                    if (kf.IsGenerator)
                    {
                        var args = CollectArgs(calleeBase, argc);
                        _sp = funcSlot;
                        Push(_interp.CreateGeneratorFromValues(kf, args, token));
                        return false;
                    }
                    SetupCalleeLocals(kf.VMChunk, calleeBase, argc);
                    PushFrame(kf.VMChunk.Name, kf.VMChunk, calleeBase, kf.VMUpvalues ?? [], token);
                    return true;
                }
                var args2 = CollectArgs(calleeBase, argc);
                _sp = funcSlot;
                Push(_interp.InvokeCallable(kf, args2, token, lr.Identifier));
                return false;
            }

            // Check if this is actually a named lambda alias
            if (_context.HasLambda(lr.Identifier))
            {
                var kl = _context.Lambdas[lr.Identifier];
                if (kl.VMChunk != null)
                {
                    SetupCalleeLocals(kl.VMChunk, calleeBase, argc);
                    PushFrame(kl.VMChunk.Name, kl.VMChunk, calleeBase, kl.VMUpvalues ?? [], token);
                    return true;
                }
                // Fallback to interpreter
                var args = CollectArgs(calleeBase, argc);
                _sp = funcSlot;
                Push(_interp.InvokeCallable(kl, args, token, kl.VMChunk?.Name ?? "<lambda>"));
                return false;
            }
        }

        if (funcVal.IsNull())
            throw new FunctionUndefinedError(token, callName ?? "<unknown>");

        // Interpreter fallback for builtins / tree-walked functions
        {
            var args = CollectArgs(calleeBase, argc);
            _sp = funcSlot;
            Value callResult;

            if (funcVal.IsLambda())
            {
                var lr = funcVal.GetLambda();
                if (_context.HasLambda(lr.Identifier))
                    callResult = _interp.InvokeCallable(_context.Lambdas[lr.Identifier], args, token, lr.Identifier);
                else
                    callResult = Value.Default;
            }
            else if (funcVal.IsStruct())
            {
                callResult = _interp.CreateObject(funcVal.GetStruct().Identifier, args, token);
            }
            else
            {
                callResult = Value.Default;
            }
            Push(callResult);
            return false;
        }
    }

    // -- Struct operator overload dispatch -------------------------------------

    /// <summary>
    /// If <paramref name="left"/> is a struct instance that defines an operator
    /// method matching <paramref name="op"/>, invoke it and return the result.
    /// Returns null when no overload is found (caller should fall through to
    /// the built-in arithmetic/comparison handler).
    /// </summary>
    private Value? TryStructOpOverload(Token token, Value left, Value right, TokenName op)
    {
        if (!left.IsObject()) return null;

        var inst = left.GetObject();
        if (!_context.HasStruct(inst.StructName)) return null;

        var struc = _context.Structs[inst.StructName];
        var opString = Serializer.GetOperatorString(op);
        if (string.IsNullOrEmpty(opString)) return null;

        KFunction? func = null;
        if (!struc.Methods.TryGetValue(opString, out func) && !string.IsNullOrEmpty(struc.BaseStruct))
        {
            if (_context.HasStruct(struc.BaseStruct))
                _context.Structs[struc.BaseStruct].Methods.TryGetValue(opString, out func);
        }

        if (func == null) return null;

        return _interp.InvokeCallable(func, [right], token, $"{inst.StructName}#{func.Name}", inst);
    }

    // -- Call helpers ----------------------------------------------------------

    private void SetupCalleeLocals(Chunk sub, int calleeBase, int argc)
    {
        int end = calleeBase + sub.LocalCount;
        if (end > MaxStack) throw new RuntimeError(Token.Eof, "Stack overflow.", []);

        if (!string.IsNullOrEmpty(sub.VariadicParamName))
        {
            // Pack extra args (at and beyond the variadic slot) into a list.
            int varSlot = calleeBase + sub.Arity;
            var varList = new List<Value>(Math.Max(0, argc - sub.Arity));
            for (int i = varSlot; i < calleeBase + argc; i++)
                varList.Add(_stack[i]);
            _stack[varSlot] = Value.CreateList(varList);
            // Zero-fill any remaining local slots after the variadic slot.
            for (int i = varSlot + 1; i < end; i++)
                _stack[i] = Value.Default;
        }
        else
        {
            // Zero-initialize slots beyond argc (pre-scanned locals)
            for (int i = calleeBase + argc; i < end; i++)
                _stack[i] = Value.Default;
        }

        _sp = end;
    }

    private List<Value> CollectArgs(int calleeBase, int argc)
    {
        var args = new List<Value>(argc);
        for (int i = calleeBase; i < calleeBase + argc; i++)
            args.Add(_stack[i]);
        return args;
    }

    // -- Upvalue construction (for DefFunc / MakeClosure) ----------------------

    private Upvalue[] BuildUpvalues(Chunk sub, VMFrame frame)
    {
        var uvs = new Upvalue[sub.Upvalues.Count];
        for (int i = 0; i < sub.Upvalues.Count; i++)
        {
            var desc = sub.Upvalues[i];
            if (desc.IsLocal)
                uvs[i] = CaptureUpvalue(frame.StackBase + desc.Index);
            else
                uvs[i] = frame.Upvalues[desc.Index];
        }
        return uvs;
    }

    // -- Synthetic AST nodes (for DefFunc / MakeClosure) -----------------------

    private static FunctionNode BuildFunctionNode(Chunk sub, string name)
    {
        var fn = new FunctionNode { Name = name };
        // Only include non-variadic params; variadic is conveyed via VariadicParamName.
        for (int i = 0; i < sub.Arity; i++)
            fn.Parameters.Add(new KeyValuePair<string, ASTNode?>(sub.ParamNames[i], null));
        fn.VariadicParamName = sub.VariadicParamName;
        return fn;
    }

    private static LambdaNode BuildLambdaNode(Chunk sub)
    {
        var ln = new LambdaNode();
        // Only include non-variadic params; variadic is conveyed via VariadicParamName.
        for (int i = 0; i < sub.Arity; i++)
            ln.Parameters.Add(new KeyValuePair<string, ASTNode?>(sub.ParamNames[i], null));
        ln.VariadicParamName = sub.VariadicParamName;
        return ln;
    }
}

// -- Extension on VMFrame ------------------------------------------------------

internal static class VMFrameExt
{
    internal static Token GetToken(this VMFrame frame)
    {
        // Prefer the chunk's own debug info: it points at the instruction that
        // actually caused the error, not the call site.
        int instrIdx = Math.Max(0, frame.IP - 1);
        int line     = frame.Chunk.GetLine(instrIdx);
        int fileId   = frame.Chunk.GetFileId(instrIdx);

        if (line > 0)
            return new Token(TokenType.Eof, TokenName.Default, new TokenSpan(fileId, line, 0), string.Empty, Value.Default);

        // No debug info (e.g. synthesised chunk) — fall back to the call-site token.
        if (frame.CallSiteToken is { } cst)
            return cst;

        return new Token(TokenType.Eof, TokenName.Default, new TokenSpan(0, 0, 0), string.Empty, Value.Default);
    }
}
