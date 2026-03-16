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

    // -- Name resolution cache (LoadGlobal fast-path) --------------------------
    // Caches resolved Values for stable global names (functions, constants,
    // packages, structs).  Invalidated on DefFunc and StoreGlobal so that
    // redefined functions and reassigned globals are never served stale.
    private readonly Dictionary<string, Value> _nameCache = new(StringComparer.Ordinal);

    // -- Closure ID counter (replaces Guid.NewGuid) ----------------------------
    private int _closureIdCounter;


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
    public Value InvokeVMCallable(Callable callable, List<Value> args, Token token, InstanceRef? instance = null)
    {
        var sub      = callable is KFunction kf ? kf.VMChunk! : ((KLambda)callable).VMChunk!;
        var upvalues = callable is KFunction kf2 ? (kf2.VMUpvalues ?? []) : (((KLambda)callable).VMUpvalues ?? []);

        int calleeBase = _sp;
        // When called re-entrantly from C# (e.g. LambdaMap), the Return handler writes
        // the result to _stack[calleeBase - 1] via Push(result). Save and restore that
        // slot so the caller's temp stack is not clobbered.
        var savedBelow = calleeBase > 0 ? _stack[calleeBase - 1] : Value.Default;

        // Push args onto the stack
        foreach (var a in args) Push(a);
        // Zero-initialize remaining local slots beyond args
        SetupCalleeLocals(sub, calleeBase, args.Count);

        int stopAt = _frameCount; // run only until this frame completes
        PushFrame(sub.Name, sub, calleeBase, upvalues, token, instance);
        var r = RunLoop(stopAt);

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
                            Push(Value.CreateLambda(_context.Lambdas[mapped].Ref));
                        }
                        else if (_context.HasConstant(name))
                        {
                            var v = _context.Constants[name];
                            _nameCache[name] = v; // constants never change
                            Push(v);
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
                        else
                        {
                            // When not found globally and we're inside a struct method,
                            // treat it as an implicit @.name() call on self.
                            if (frame.Self != null)
                                Push(Value.CreateLambda(new LambdaRef { Identifier = name, BoundSelf = frame.Self }));
                            else
                                Push(gv); // gv is null/default here
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
                        var funcName = frame.Chunk.Names[B];

                        // Build upvalue array for the closure (even for named functions)
                        var upvalues = BuildUpvalues(sub, frame);

                        var kfunc = new KFunction(BuildFunctionNode(sub, funcName))
                        {
                            Name      = funcName,
                            VMChunk   = sub,
                            VMUpvalues = upvalues
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

                        // If defined inside another function frame, track for cleanup on return.
                        if (_frameCount > 1)
                            _frames[_frameCount - 1].TrackLocalFunction(funcName);

                        break;
                    }

                    // -- MakeClosure ---------------------------------------
                    case Opcode.MakeClosure:
                    {
                        var sub      = frame.Chunk.SubChunks[A];
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

                        var id   = $"__lam_{_closureIdCounter++}";
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
                        _caughtError = null; // consumed
                        break;
                    }

                    // -- Interpreter fallback ------------------------------
                    case Opcode.InterpFallback:
                    {
                        var node = frame.Chunk.NodePool[A];
                        Value r;
                        // Ensure the interpreter call-stack is non-empty so that builtins
                        // (e.g. ListBuiltinHandler) can safely call CallStack.Peek().
                        bool pushedFrame = _interp.CallStack.Count == 0;
                        if (pushedFrame) _interp.PushVMDispatchFrame(_globals);
                        try
                        {
                            // If there are named locals in the current frame, expose them to the
                            // interpreter so that sub-expressions (e.g. named-arg values) can
                            // resolve VM-local variables by name.
                            if (frame.Chunk.LocalNames.Count > 0)
                            {
                                var locals = new Dictionary<string, Value>(frame.Chunk.LocalNames.Count);
                                foreach (var (name, slot) in frame.Chunk.LocalNames)
                                    locals[name] = _stack[frame.StackBase + slot];
                                r = _interp.InterpretNodeWithLocals(node, locals);
                                // Sync any updated locals back to the VM stack.
                                foreach (var (name, slot) in frame.Chunk.LocalNames)
                                {
                                    if (locals.TryGetValue(name, out var updated))
                                        _stack[frame.StackBase + slot] = updated;
                                }
                            }
                            else
                            {
                                r = _interp.InterpretNode(node);
                            }
                        }
                        finally
                        {
                            if (pushedFrame) _interp.PopVMDispatchFrame();
                        }
                        Push(r);
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
                    SetupCalleeLocals(kf.VMChunk, calleeBase, argc);
                    PushFrame(kf.VMChunk.Name, kf.VMChunk, calleeBase, kf.VMUpvalues ?? [], token);
                    return true;
                }
                var args = CollectArgs(calleeBase, argc);
                _sp = funcSlot;
                Push(_interp.InvokeCallable(kf, args, token, lr.Identifier));
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
        {
            throw new FunctionUndefinedError(token, callName ?? "<unknown>");
        }

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
        if (frame.CallSiteToken is { } cst)
            return cst;

        // No call-site token (e.g. top-level frame): build one from the chunk's
        // debug info so errors point at the right file and line.
        int instrIdx = Math.Max(0, frame.IP - 1);
        int line     = frame.Chunk.GetLine(instrIdx);
        int fileId   = frame.Chunk.GetFileId(instrIdx);
        return new Token(TokenType.Eof, TokenName.Default, new TokenSpan(fileId, line, 0), string.Empty, Value.Default);
    }
}
