using System.Buffers;
using System.Runtime.InteropServices;
using kiwi.Parsing;
using kiwi.Parsing.AST;
using kiwi.Parsing.Keyword;
using kiwi.Runtime;
using kiwi.Runtime.Builtin.Handler;
using kiwi.Runtime.Builtin.Operation;
using kiwi.Runtime.Builtin.Dispatcher;
using kiwi.Runtime.Builtin.Util;
using kiwi.Settings;
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

    // -- Name resolution caches (LoadGlobal fast-paths) ------------------------
    // _nameCache: cached Values for STABLE bindings (functions, constants, structs,
    //   packages, mapped-lambdas).  Invalidated on DefFunc / StoreGlobal / StoreConst.
    // _pathCache: resolution KIND for MUTABLE bindings (lambdas, package variables,
    //   user globals).  Avoids the sequential Has* dict lookups on repeated access.
    //   Invalidated alongside _nameCache via InvalidateName().
    private readonly Dictionary<string, Value>      _nameCache = new(StringComparer.Ordinal);
    private readonly Dictionary<string, GlobalKind> _pathCache = new(StringComparer.Ordinal);
    private enum GlobalKind : byte { Lambda = 1, PackageVar, UserGlobal }

    // -- Package prefix cache (avoids PackageStack.Reverse() + Join on every StoreGlobal/DefFunc) --
    private string _pkgPrefix = string.Empty;
    private readonly Stack<string> _pkgPrefixStack = new();

    // -- Closure ID counter (replaces Guid.NewGuid) ----------------------------
    // Must be global (not per-VM) so that task VMs don't collide with each other
    // or with the main VM in the shared _context.Lambdas dictionary.
    private static int _globalClosureIdCounter;

    // -- VM debug hook ---------------------------------------------------------
    // Fires when the execution crosses into a new source line.
    // Callback receives (fileId, line). Null when not debugging.
    public Action<int, int>? DebugHook { get; set; }
    private int _dbgLastFileId = -1;
    private int _dbgLastLine   = -1;

    // -- Shared runtime state --------------------------------------------------
    private readonly Scope                _globals;
    private readonly KContext             _context;
    private readonly System.Text.StringBuilder _interpSB = new();  // reused for Interpolate opcode

    // -- Interpreter-side state (formerly on Interpreter class) ----------------
    private GeneratorRef? _activeGenerator;

    public Dictionary<string, string> CliArgs { get; set; } = [];
    public string ExecutionPath { get; set; } = string.Empty;
    public string EntryPath     { get; set; } = string.Empty;
    /// <summary>Working directory at the time kiwi was invoked.</summary>
    public string ProjectRoot   { get; set; } = Directory.GetCurrentDirectory();
    public Stack<StackFrame> CallStack { get; set; } = [];
    internal Stack<string> PackageStack { get; set; } = [];
    internal Stack<string> StructStack  { get; set; } = [];
    public long CurrentTaskId { get; set; } = 0;
    public void SetContext(KContext context) { /* context is readonly in KiwiVM – no-op kept for API compat */ }
    /// <summary>Exposes the shared context for handlers that need it.</summary>
    public KContext Context => _context;

    // -- Thread-local slot -----------------------------------------------------
    [ThreadStatic]
    public static KiwiVM? Current;

    // -- Construction ----------------------------------------------------------

    /// <summary>Root VM: creates its own context and globals.</summary>
    public KiwiVM()
    {
        _context = new KContext();
        _globals = new Scope();
        _globals.Declare("global", Value.CreateHashmap());
        Current = this;
    }

    /// <summary>Sub-VM sharing all state from parent.</summary>
    public KiwiVM(KiwiVM parent)
    {
        _context  = parent._context;
        _globals  = parent._globals;
        CliArgs   = parent.CliArgs;
        ExecutionPath = parent.ExecutionPath;
        EntryPath     = parent.EntryPath;
        ProjectRoot   = parent.ProjectRoot;
        CallStack     = parent.CallStack;
        Current = this;
    }

    /// <summary>Sub-VM with explicit globals (for task threads / generators).</summary>
    public KiwiVM(KiwiVM parent, Scope globals)
    {
        _context  = parent._context;
        _globals  = globals;
        CliArgs   = parent.CliArgs;
        ExecutionPath = parent.ExecutionPath;
        EntryPath     = parent.EntryPath;
        ProjectRoot   = parent.ProjectRoot;
        CallStack     = parent.CallStack;
        Current = this;
    }

    // -- Cache helpers ---------------------------------------------------------

    /// <summary>Removes a name from both resolution caches atomically.</summary>
    private void InvalidateName(string name)
    {
        _nameCache.Remove(name);
        _pathCache.Remove(name);
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

    public Value InvokeVMCallable(Callable callable, IReadOnlyList<Value> args, Token token, InstanceRef? instance = null)
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
        // Propagate the defining-struct name so super.method() can resolve the base correctly.
        if (callable is KFunction kfOwner && !string.IsNullOrEmpty(kfOwner.OwnerStruct))
            _frames[_frameCount - 1].OwnerStruct = kfOwner.OwnerStruct;
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

    // -- Debugger inspection ---------------------------------------------------

    /// <summary>Number of active VM frames (1 = only main, grows with each call).</summary>
    public int FrameCount => _frameCount;

    /// <summary>Returns the VM frame at the given index for backtrace display.</summary>
    public VMFrame GetFrame(int i) => _frames[i];

    /// <summary>
    /// Enumerates the named local variables visible in the current (innermost) frame,
    /// yielding each variable's name and current value.
    /// </summary>
    public IEnumerable<(string Name, Value Val)> GetCurrentLocals()
    {
        if (_frameCount == 0) yield break;
        var frame = _frames[_frameCount - 1];
        foreach (var (name, slot) in frame.Chunk.LocalNames)
            yield return (name, _stack[frame.StackBase + slot]);
    }

    /// <summary>
    /// Returns global-scope variables visible from the current frame, filtered to names
    /// actually referenced by the chunk. Used by the debugger for global-scope frames
    /// where <see cref="GetCurrentLocals"/> yields nothing.
    /// </summary>
    public IEnumerable<(string Name, Value Val)> GetCurrentGlobals()
    {
        if (_frameCount == 0) yield break;
        var frame = _frames[_frameCount - 1];
        var seen  = new HashSet<string>(StringComparer.Ordinal);
        foreach (var name in frame.Chunk.Names)
        {
            if (!seen.Add(name)) continue;
            if (_globals.TryGet(name, out var val) && !val.IsNull())
                yield return (name, val);
        }
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

                // Fire the debug hook whenever execution moves to a new source line.
                if (DebugHook != null)
                {
                    int dbgIp     = frame.IP - 1;
                    int dbgLine   = frame.Chunk.GetLine(dbgIp);
                    int dbgFileId = frame.Chunk.GetFileId(dbgIp);
                    if (dbgLine > 0 && (dbgLine != _dbgLastLine || dbgFileId != _dbgLastFileId))
                    {
                        _dbgLastLine   = dbgLine;
                        _dbgLastFileId = dbgFileId;
                        DebugHook(dbgFileId, dbgLine);
                    }
                }

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

                        // Fast path 1: cached Value for stable bindings (functions, constants, structs, packages, mapped-lambdas)
                        if (_nameCache.TryGetValue(name, out var cachedVal))
                        {
                            Push(cachedVal);
                            break;
                        }

                        // Fast path 2: cached KIND for mutable bindings — skips sequential Has* lookups
                        if (_pathCache.TryGetValue(name, out var pkind))
                        {
                            bool pathHandled = true;
                            switch (pkind)
                            {
                                case GlobalKind.Lambda:
                                    _globals.TryGet(name, out var sl);
                                    Push(sl.IsNull() ? Value.CreateLambda(_context.Lambdas[name].Ref) : sl);
                                    break;
                                case GlobalKind.PackageVar:
                                    Push(_context.PackageVariables[name]);
                                    break;
                                case GlobalKind.UserGlobal:
                                    if (_globals.TryGet(name, out var gvp) && !gvp.IsNull())
                                        Push(gvp);
                                    else
                                    {
                                        _pathCache.Remove(name); // stale — fall through to slow path
                                        pathHandled = false;
                                    }
                                    break;
                                default:
                                    pathHandled = false;
                                    break;
                            }
                            if (pathHandled) break;
                        }

                        // Slow path: full resolution — populates _nameCache or _pathCache for next time
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
                                _nameCache[name] = v; // stable: invalidated on DefFunc
                                Push(v);
                            }
                        }
                        else if (_context.HasLambda(name))
                        {
                            _pathCache[name] = GlobalKind.Lambda; // mutable: cache kind only
                            _globals.TryGet(name, out var storedLambda);
                            Push(storedLambda.IsNull()
                                ? Value.CreateLambda(_context.Lambdas[name].Ref)
                                : storedLambda);
                        }
                        else if (_context.HasMappedLambda(name))
                        {
                            // Mapped lambdas are stable once registered (StoreDecoratedFunc invalidates).
                            // Cache the Value so subsequent accesses skip this path entirely.
                            var mapped = _context.LambdaTable[name];
                            var klambda = _context.Lambdas[mapped];
                            var lref = klambda.Ref.Identifier.Length > 0
                                ? klambda.Ref
                                : new LambdaRef { Identifier = mapped, VMChunk = klambda.VMChunk, VMUpvalues = klambda.VMUpvalues };
                            var v = Value.CreateLambda(lref);
                            _nameCache[name] = v; // stable: invalidated on StoreDecoratedFunc
                            Push(v);
                        }
                        else if (_context.HasConstant(name))
                        {
                            var v = _context.Constants[name];
                            _nameCache[name] = v; // constants never change
                            Push(v);
                        }
                        else if (_context.HasPackageVariable(name))
                        {
                            _pathCache[name] = GlobalKind.PackageVar; // mutable: cache kind only
                            Push(_context.PackageVariables[name]);
                        }
                        else if (_context.HasStruct(name))
                        {
                            var v = Value.CreateStruct(new StructRef { Identifier = name });
                            _nameCache[name] = v; // struct definitions are stable
                            Push(v);
                        }
                        else if (_globals.TryGet(name, out var gv) && !gv.IsNull())
                        {
                            _pathCache[name] = GlobalKind.UserGlobal; // mutable: cache kind only
                            Push(gv);
                        }
                        else if (_context.HasPackage(name))
                        {
                            var v = Value.CreatePackage(name);
                            _nameCache[name] = v; // packages never change
                            Push(v);
                        }
                        else
                        {
                            // Package-internal call: if the current chunk has a package prefix
                            // (e.g. "foo::"), try looking up "foo::name" before giving up.
                            // This lets package-internal functions call each other without
                            // the caller having to write the full qualified name.
                            var pkgPfx = frame.Chunk.PackagePrefix;
                            if (pkgPfx.Length > 0)
                            {
                                var qualName = pkgPfx + name;
                                if (_context.HasFunction(qualName))
                                {
                                    var fn = _context.Functions[qualName];
                                    Push(Value.CreateLambda(new LambdaRef { Identifier = qualName, VMChunk = fn.VMChunk, VMUpvalues = fn.VMUpvalues }));
                                    break;
                                }
                                if (_context.HasLambda(qualName))
                                {
                                    Push(Value.CreateLambda(_context.Lambdas[qualName].Ref));
                                    break;
                                }
                                if (_context.HasConstant(qualName))
                                {
                                    Push(_context.Constants[qualName]);
                                    break;
                                }
                                if (_context.HasPackageVariable(qualName))
                                {
                                    Push(_context.PackageVariables[qualName]);
                                    break;
                                }
                            }
                            // When not found globally and we're inside a struct method,
                            // treat it as an implicit @.name() call on self.
                            if (frame.Self != null)
                                Push(Value.CreateLambda(new LambdaRef { Identifier = name, BoundSelf = frame.Self }));
                            else
                                throw new VariableUndefinedError(frame.GetToken(), name);
                        }
                        break;
                    }
                    case Opcode.StoreGlobal:
                    {
                        var name = frame.Chunk.Names[A];
                        if (_context.HasConstant(name))
                            throw new Tracing.Error.InvalidOperationError(frame.GetToken(), $"Cannot reassign constant '{name}'.");
                        var v    = Pop();
                        if (v.IsLambda())
                        {
                            var lr = v.GetLambda();
                            if (!string.IsNullOrEmpty(lr.Identifier) && _context.HasLambda(lr.Identifier))
                                _context.Lambdas[name] = _context.Lambdas[lr.Identifier];
                        }
                        _globals.Assign(name, v);
                        InvalidateName(name); // invalidate both resolution caches
                        // Mirror package-level variable into PackageVariables for pkg::name access.
                        if (_pkgPrefix.Length > 0)
                        {
                            _context.PackageVariables[_pkgPrefix + name] = v;
                        }
                        break;
                    }

                    case Opcode.StoreConst:
                    {
                        var name = frame.Chunk.Names[A];
                        var v    = Pop();
                        _context.Constants[name] = v;
                        _globals.Assign(name, v);
                        InvalidateName(name);
                        if (_pkgPrefix.Length > 0)
                        {
                            var qualName = _pkgPrefix + name;
                            _context.Constants[qualName] = v;
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
                    case Opcode.Swap:
                    {
                        var top    = Pop();
                        var second = Pop();
                        Push(top);
                        Push(second);
                        break;
                    }

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
                    case Opcode.IntDiv:
                    {
                        var r = Pop(); var l = Pop();
                        Push(TryStructOpOverload(frame.GetToken(), l, r, TokenName.Ops_IntDivide) ?? MathOp.IntDiv(frame.GetToken(), ref l, ref r));
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
                        Push(TryStructOpOverload(frame.GetToken(), l, r, TokenName.Ops_Equal) ?? Value.CreateBoolean(ComparisonOp.Equal(ref l, ref r)));
                        break;
                    }
                    case Opcode.NEq:
                    {
                        var r = Pop(); var l = Pop();
                        Push(TryStructOpOverload(frame.GetToken(), l, r, TokenName.Ops_NotEqual) ?? ComparisonOp.NotEqual(ref l, ref r));
                        break;
                    }
                    case Opcode.Lt:
                    {
                        var r = Pop(); var l = Pop();
                        Push(TryStructOpOverload(frame.GetToken(), l, r, TokenName.Ops_LessThan) ?? ComparisonOp.LessThan(ref l, ref r));
                        break;
                    }
                    case Opcode.LtE:
                    {
                        var r = Pop(); var l = Pop();
                        Push(TryStructOpOverload(frame.GetToken(), l, r, TokenName.Ops_LessThanOrEqual) ?? ComparisonOp.LessThanOrEqual(ref l, ref r));
                        break;
                    }
                    case Opcode.Gt:
                    {
                        var r = Pop(); var l = Pop();
                        Push(TryStructOpOverload(frame.GetToken(), l, r, TokenName.Ops_GreaterThan) ?? ComparisonOp.GreaterThan(ref l, ref r));
                        break;
                    }
                    case Opcode.GtE:
                    {
                        var r = Pop(); var l = Pop();
                        Push(TryStructOpOverload(frame.GetToken(), l, r, TokenName.Ops_GreaterThanOrEqual) ?? ComparisonOp.GreaterThanOrEqual(ref l, ref r));
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
                        var ls = l.IsString() ? l.GetString() : Serialize(l);
                        var rs = r.IsString() ? r.GetString() : Serialize(r);
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
                        var args = new Value[argc];
                        for (int i = 0; i < argc; i++) args[i] = _stack[_sp - argc + i];
                        _sp = objSlot; // collapse obj+args

                        // Dispatch
                        var callResult = DispatchMethod(obj, methodName, args, frame.GetToken());
                        Push(callResult);
                        break;
                    }

                    // -- CallSuperMethod -----------------------------------
                    case Opcode.CallSuperMethod:
                    {
                        int argc       = A;
                        var methodName = frame.Chunk.Names[B];

                        // Collect args from the stack
                        int argsBase = _sp - argc;
                        var args = new Value[argc];
                        for (int i = 0; i < argc; i++) args[i] = _stack[argsBase + i];
                        _sp = argsBase;

                        // Resolve the base struct from the current method's owner struct
                        var ownerStruct = frame.OwnerStruct;
                        if (string.IsNullOrEmpty(ownerStruct))
                            throw new RuntimeError(frame.GetToken(), "super called outside of a struct method.", []);

                        if (!_context.HasStruct(ownerStruct))
                            throw new RuntimeError(frame.GetToken(), $"Struct '{ownerStruct}' not found for super call.", []);

                        var owner = _context.Structs[ownerStruct];
                        if (string.IsNullOrEmpty(owner.BaseStruct))
                            throw new RuntimeError(frame.GetToken(), $"Struct '{ownerStruct}' has no base struct; cannot call super.", []);

                        // Walk the base struct chain to find the method
                        KFunction? superFn = null;
                        var search = _context.HasStruct(owner.BaseStruct) ? _context.Structs[owner.BaseStruct] : null;
                        while (search != null)
                        {
                            if (search.Methods.TryGetValue(methodName, out superFn)) break;
                            superFn = null;
                            search = !string.IsNullOrEmpty(search.BaseStruct) && _context.HasStruct(search.BaseStruct)
                                ? _context.Structs[search.BaseStruct] : null;
                        }

                        if (superFn == null)
                            throw new RuntimeError(frame.GetToken(), $"Method '{methodName}' not found in base of '{ownerStruct}'.", []);

                        var superResult = InvokeCallable(superFn, args, frame.GetToken(), methodName, frame.Self);
                        Push(superResult);
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
                                Value reorderedResult;
                                try { reorderedResult = DispatchMethod(obj, methodName, reorderedArgs, frame.GetToken()); }
                                finally { ReturnArgs(reorderedArgs); }
                                _sp = objSlot;
                                Push(reorderedResult);
                                break;
                            }
                        }

                        // Non-struct or method not found: collect args in source order
                        var plainArgs = new Value[argc];
                        for (int i = 0; i < argc; i++) plainArgs[i] = _stack[_sp - argc + i];
                        _sp = objSlot;
                        Push(DispatchMethod(obj, methodName, plainArgs, frame.GetToken()));
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
                        var funcName = _pkgPrefix.Length > 0 ? _pkgPrefix + rawName : rawName;

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
                        if (_pkgPrefix.Length > 0) sub.PackagePrefix = _pkgPrefix;

                        _context.Functions[funcName] = kfunc;
                        InvalidateName(funcName); // invalidate both resolution caches

                        // If defined inside another function frame (but not a global-scope
                        // include frame), track for cleanup on return.
                        if (_frameCount > 1 && !_frames[_frameCount - 1].IsGlobalScope)
                            _frames[_frameCount - 1].TrackLocalFunction(funcName);

                        break;
                    }

                    // -- DefFuncAndPush ------------------------------------
                    case Opcode.DefFuncAndPush:
                    {
                        // === Same as DefFunc: register the KFunction ===
                        var sub      = frame.Chunk.SubChunks[A];
                        var rawName  = frame.Chunk.Names[B];
                        var funcName = _pkgPrefix.Length > 0 ? _pkgPrefix + rawName : rawName;
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
                        if (_pkgPrefix.Length > 0) sub.PackagePrefix = _pkgPrefix;

                        _context.Functions[funcName] = kfunc;
                        InvalidateName(funcName);

                        if (_frameCount > 1)
                            _frames[_frameCount - 1].TrackLocalFunction(funcName);

                        // === Additional: wrap as internal KLambda and push ===
                        // Mirror what the treewalker does in Visit(DecoratedFunctionNode):
                        // create a KLambda backed by the same VM chunk so decorators can
                        // receive and call the function as a value, even after StoreDecoratedFunc
                        // removes the KFunction entry from Context.Functions.
                        var klambda = new KLambda(BuildLambdaNode(sub))
                        {
                            VMChunk       = sub,
                            VMUpvalues    = upvalues,
                            CapturedScope = _globals,
                        };
                        for (int i = 0; i < sub.Arity; i++)
                            klambda.Parameters.Add(new KeyValuePair<string, Value>(sub.ParamNames[i], Value.Default));
                        foreach (var pn in sub.DefaultParamNames)
                            klambda.DefaultParameters.Add(pn);
                        if (!string.IsNullOrEmpty(sub.VariadicParamName))
                            klambda.VariadicParamName = sub.VariadicParamName;

                        var internalId = $"__deco_{Interlocked.Increment(ref _globalClosureIdCounter)}__";
                        var lref       = new LambdaRef { Identifier = internalId, VMChunk = sub, VMUpvalues = upvalues };
                        klambda.Ref    = lref;
                        _context.Lambdas[internalId] = klambda;
                        Push(Value.CreateLambda(lref));
                        break;
                    }

                    // -- StoreDecoratedFunc --------------------------------
                    case Opcode.StoreDecoratedFunc:
                    {
                        var rawName  = frame.Chunk.Names[A];
                        var funcName = _pkgPrefix.Length > 0 ? _pkgPrefix + rawName : rawName;

                        var value = Pop();
                        _context.Functions.Remove(funcName);
                        InvalidateName(funcName);

                        if (value.IsLambda())
                        {
                            var lr       = value.GetLambda();
                            var lambdaId = string.IsNullOrEmpty(lr.Identifier) ? funcName : lr.Identifier;
                            // Map funcName → the lambda identifier (like Context.AddMappedLambda).
                            _context.LambdaTable[funcName] = lambdaId;
                            // If the lambda has a VMChunk but no entry in Lambdas yet, register it.
                            if (!_context.Lambdas.ContainsKey(lambdaId) && lr.VMChunk != null)
                            {
                                var kl = new KLambda(BuildLambdaNode(lr.VMChunk))
                                {
                                    VMChunk    = lr.VMChunk,
                                    VMUpvalues = lr.VMUpvalues,
                                    Ref        = lr,
                                };
                                _context.Lambdas[lambdaId] = kl;
                            }
                        }
                        else
                        {
                            _globals.Assign(funcName, value);
                            InvalidateName(funcName);
                            if (_pkgPrefix.Length > 0)
                            {
                                _context.PackageVariables[_pkgPrefix + rawName] = value;
                            }
                        }
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

                        // Propagate package prefix from the enclosing frame so that
                        // unqualified sibling-function calls inside the lambda body resolve.
                        if (frame.Chunk.PackagePrefix.Length > 0 && sub.PackagePrefix.Length == 0)
                            sub.PackagePrefix = frame.Chunk.PackagePrefix;

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
                        Push(GetIndex(obj, key, frame.GetToken()));
                        break;
                    }
                    case Opcode.IndexSet:
                    {
                        var val = Pop();
                        var key = Pop();
                        var obj = Pop();
                        SetIndex(obj, key, val, frame.GetToken());
                        Push(val);
                        break;
                    }
                    case Opcode.IndexOpAssign:
                    {
                        var rhs = Pop();
                        var key = Pop();
                        var obj = Pop();
                        var tok = frame.GetToken();
                        var old = GetIndex(obj, key, tok);
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
                        SetIndex(obj, key, newVal, tok);
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
                        Push(GetSlice(obj, startV, stopV, stepV, frame.GetToken()));
                        break;
                    }

                    case Opcode.SliceSet:
                    {
                        var rhs    = Pop();
                        Value? stepV  = (A & 4) != 0 ? Pop() : (Value?)null;
                        Value? stopV  = (A & 2) != 0 ? Pop() : (Value?)null;
                        Value? startV = (A & 1) != 0 ? Pop() : (Value?)null;
                        var obj = Pop();
                        SetSlice(obj, startV, stopV, stepV, rhs, frame.GetToken());
                        Push(rhs);
                        break;
                    }

                    // -- Member access -------------------------------------
                    case Opcode.GetMember:
                    {
                        var memberName = frame.Chunk.Names[A];
                        var obj        = Pop();
                        Push(GetMember(obj, memberName, frame.GetToken()));
                        break;
                    }
                    case Opcode.SetMember:
                    {
                        var memberName = frame.Chunk.Names[A];
                        var obj        = Pop();
                        var val        = Pop();
                        SetMember(obj, memberName, val, frame.GetToken());
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
                        Push(CreateObject(structName, args, frame.GetToken()));
                        break;
                    }

                    // -- Interpolation -------------------------------------
                    case Opcode.Interpolate:
                    {
                        // Pop parts in reverse, serialize each into a shared StringBuilder.
                        // Avoids allocating a string[] + the extra string from string.Concat.
                        var sb = _interpSB;
                        sb.Clear();
                        int interpBase = _sp - A;
                        for (int i = interpBase; i < _sp; i++)
                            sb.Append(Serialize(_stack[i]));
                        _sp = interpBase;
                        Push(Value.CreateString(sb.ToString()));
                        break;
                    }

                    // -- Print ---------------------------------------------
                    case Opcode.Print:
                    {
                        var v         = Pop();
                        var text      = Serialize(v);
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
                        var text = Serialize(v);
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
                                var  sv = Value.CreateString(c);
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
                        YieldFromVM(v);
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
                            errMsg  = Serialize(v);
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

                    // -- Builtin call --------------------------------------
                    case Opcode.CallBuiltin:
                    {
                        var node = (Parsing.AST.FunctionCallNode)frame.Chunk.NodePool[A];
                        var args = new List<Value>(B);
                        for (int i = 0; i < B; i++) args.Add(Pop());
                        args.Reverse();
                        Push(ExecuteBuiltin(node.Token, node.Op, args));
                        break;
                    }

                    // -- Export / Import / Require -------------------------
                    case Opcode.Export:
                    {
                        var pkgName = Pop();
                        ImportPackage(frame.GetToken(), pkgName);
                        break;
                    }

                    case Opcode.ImportPkg:
                    {
                        var pkgName = Pop();
                        ImportPackage(frame.GetToken(), pkgName);
                        Push(Value.CreatePackage(pkgName.GetString()));
                        break;
                    }

                    case Opcode.Require:
                    {
                        var pkgName = Pop();
                        if (!pkgName.IsString())
                            throw new Tracing.Error.InvalidOperationError(frame.GetToken(), "require expects a string package name.");
                        var name = pkgName.GetString();
                        if (!_context.HasPackage(name) && !_context.ImportedPackages.Contains(name))
                            throw new Tracing.Error.PackageUndefinedError(frame.GetToken(), name);
                        break;
                    }

                    // -- Eval / Include ------------------------------------
                    case Opcode.Eval:
                    {
                        var code = Pop();
                        if (!code.IsString())
                            throw new KiwiError(frame.GetToken(), "eval() requires a string expression.");
                        Push(EvalCode(frame.GetToken(), code.GetString()));
                        break;
                    }

                    case Opcode.Include:
                    {
                        var path = Pop();
                        if (!path.IsString())
                            throw new InvalidOperationError(frame.GetToken(), "Include path must be a string.");
                        IncludeFile(frame.GetToken(), path.GetString());
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
                        _context.Events.On(eventName.GetString(), callback, priority: A);
                        break;
                    }
                    case Opcode.EventOnce:
                    {
                        var callback  = Pop();
                        var eventName = Pop();
                        _context.Events.Once(eventName.GetString(), callback, priority: A);
                        break;
                    }
                    case Opcode.EventOff:
                    {
                        Value? callback = A == 1 ? Pop() : null;
                        var eventName = Pop();
                        _context.Events.Off(eventName.GetString(), callback);
                        break;
                    }
                    case Opcode.EventEmit:
                    {
                        // A args were pushed after the event-name; collect them bottom-first.
                        var args = new List<Value>(A);
                        for (int ei = 0; ei < A; ei++) args.Add(Value.Default);
                        for (int ei = A - 1; ei >= 0; ei--) args[ei] = Pop();
                        var eventName = Pop();
                        bool pushedFrame = CallStack.Count == 0;
                        if (pushedFrame) PushVMDispatchFrame(_globals);
                        List<Value> results;
                        try   { results = _context.Events.Emit(this, frame.GetToken(), eventName.GetString(), args); }
                        finally { if (pushedFrame) PopVMDispatchFrame(); }
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
                        StructStack.Push(structName);
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
                        if (_pkgPrefix.Length > 0) sub.PackagePrefix = _pkgPrefix;

                        var struc = _pendingStructs.Peek();
                        kfunc.OwnerStruct = struc.Name;
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
                        StructStack.Pop();
                        if (!struc.IsAbstract && !string.IsNullOrEmpty(struc.BaseStruct))
                        {
                            var baseStruc = _context.Structs[struc.BaseStruct];
                            foreach (var am in baseStruc.AbstractMethods)
                                if (!struc.Methods.ContainsKey(am))
                                    throw new AbstractMethodError(frame.GetToken(), struc.Name, am);
                        }
                        _context.Structs[struc.Name] = struc;
                        InvalidateName(struc.Name);
                        // Also register under the fully-qualified name (e.g. "httpserver::Response")
                        // so that pkg::Struct.new() calls work from any context.
                        if (!string.IsNullOrEmpty(_pkgPrefix))
                        {
                            var qualifiedStructName = _pkgPrefix + struc.Name;
                            _context.Structs[qualifiedStructName] = struc;
                            InvalidateName(qualifiedStructName);
                        }
                        break;
                    }

                    // -- Package definition --------------------------------
                    case Opcode.PackageBegin:
                    {
                        var localName     = frame.Chunk.Names[A];
                        var qualifiedName = _pkgPrefix.Length > 0
                            ? _pkgPrefix + localName
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
                        PackageStack.Push(localName);
                        _pkgPrefixStack.Push(_pkgPrefix);
                        _pkgPrefix = qualifiedName + "::";
                        break;
                    }
                    case Opcode.PackageEnd:
                    {
                        PackageStack.Pop();
                        var qualifiedName = _pkgPrefix.Length >= 2 ? _pkgPrefix[..^2] : _pkgPrefix;
                        _pkgPrefix = _pkgPrefixStack.Count > 0 ? _pkgPrefixStack.Pop() : string.Empty;
                        RegisterTypeBuiltins(qualifiedName);
                        _context.ImportedPackages.Add(qualifiedName);
                        InvalidateName(qualifiedName);
                        break;
                    }
                    case Opcode.PackageAbort:
                    {
                        // Package activation failed: pop the stack but don't mark as imported.
                        // The stored PackageNode AST allows ImportPackage to retry later.
                        PackageStack.Pop();
                        _pkgPrefix = _pkgPrefixStack.Count > 0 ? _pkgPrefixStack.Pop() : string.Empty;
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
                        InvalidateName(kenum.Name);
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
            catch (KiwiError e)
            {
                // Walk up the frame stack to find a frame with an active try-handler.
                // This allows exceptions thrown in callee frames to bubble up to a
                // try-catch in a calling frame.
                bool handled = false;
                while (_frameCount > stopAt)
                {
                    var f = _frames[_frameCount - 1];
                    if (f.HasTryHandlers)
                    {
                        if (f.PopTryHandler(out int catchIP, out _, out int savedSP))
                        {
                            // Pop any callee frames above f (they were called inside the try block).
                            while (_frameCount > stopAt && _frames[_frameCount - 1] != f)
                            {
                                var dead = _frames[_frameCount - 1];
                                CloseUpvalues(dead.StackBase);
                                _frameCount--;
                            }
                            _sp = savedSP;
                            _caughtError = e;
                            f.IP = catchIP;
                            frame = f; // update local frame ref so outer loop re-enters correctly
                            handled = true;
                        }
                        break;
                    }
                    // This frame has no handler — pop it and keep searching.
                    CloseUpvalues(f.StackBase);
                    _frameCount--;
                }
                if (!handled) throw;
                // Re-enter outer loop with the handler frame active.
                continue;
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
        for (int i = 0; i < paramCount; i++) reordered[i] = Value.Default; // null sentinel, not zero-init
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
            Value boundResult;
            try { boundResult = DispatchMethod(selfVal, lr.Identifier, args, token); }
            finally { ReturnArgs(args); }
            _sp = funcSlot;
            Push(boundResult);
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
            Value namedCallableResult;
            try
            {
                var callable2 = _context.HasFunction(lr2.Identifier)
                    ? (Callable)_context.Functions[lr2.Identifier]
                    : _context.Lambdas[lr2.Identifier];
                namedCallableResult = InvokeCallable(callable2, argsL, token, lr2.Identifier);
            }
            finally { ReturnArgs(argsL); }
            _sp = funcSlot;
            Push(namedCallableResult);
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
                Value boundSelfResult;
                try { boundSelfResult = DispatchMethod(selfVal, lr.Identifier, args, token); }
                finally { ReturnArgs(args); }
                _sp = funcSlot;
                Push(boundSelfResult);
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
                    Value genResult;
                    try
                    {
                        // Resolve the KFunction by name so CreateGeneratorFromValues can run the body.
                        KFunction? kfGen = !string.IsNullOrEmpty(lr.Identifier) && _context.HasFunction(lr.Identifier)
                            ? _context.Functions[lr.Identifier] : null;
                        genResult = kfGen != null
                            ? CreateGeneratorFromValues(kfGen, args, token)
                            : Value.Default;
                    }
                    finally { ReturnArgs(args); }
                    _sp = funcSlot;
                    Push(genResult);
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
                        Value kfGenResult;
                        try { kfGenResult = CreateGeneratorFromValues(kf, args, token); }
                        finally { ReturnArgs(args); }
                        _sp = funcSlot;
                        Push(kfGenResult);
                        return false;
                    }
                    SetupCalleeLocals(kf.VMChunk, calleeBase, argc);
                    PushFrame(kf.VMChunk.Name, kf.VMChunk, calleeBase, kf.VMUpvalues ?? [], token);
                    return true;
                }
                var args2 = CollectArgs(calleeBase, argc);
                Value kfResult;
                try { kfResult = InvokeCallable(kf, args2, token, lr.Identifier); }
                finally { ReturnArgs(args2); }
                _sp = funcSlot;
                Push(kfResult);
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
                Value klResult;
                try { klResult = InvokeCallable(kl, args, token, kl.VMChunk?.Name ?? "<lambda>"); }
                finally { ReturnArgs(args); }
                _sp = funcSlot;
                Push(klResult);
                return false;
            }
        }

        if (funcVal.IsNull())
            throw new FunctionUndefinedError(token, callName ?? "<unknown>");

        // Interpreter fallback for builtins / tree-walked functions
        {
            var args = CollectArgs(calleeBase, argc);
            Value callResult;
            try
            {
                if (funcVal.IsLambda())
                {
                    var lr = funcVal.GetLambda();
                    if (_context.HasLambda(lr.Identifier))
                        callResult = InvokeCallable(_context.Lambdas[lr.Identifier], args, token, lr.Identifier);
                    else
                        callResult = Value.Default;
                }
                else if (funcVal.IsStruct())
                {
                    callResult = CreateObject(funcVal.GetStruct().Identifier, args, token);
                }
                else
                {
                    throw new Tracing.Error.InvalidOperationError(token, $"Value of type '{Typing.TypeRegistry.GetTypeName(funcVal)}' is not callable.");
                }
            }
            finally { ReturnArgs(args); }
            _sp = funcSlot;
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

        return InvokeCallable(func, [right], token, $"{inst.StructName}#{func.Name}", inst);
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

    private Value[] CollectArgs(int calleeBase, int argc)
    {
        var args = ArrayPool<Value>.Shared.Rent(argc);
        for (int i = 0; i < argc; i++)
            args[i] = _stack[calleeBase + i];
        return args;
    }

    private static void ReturnArgs(Value[] args)
    {
        ArrayPool<Value>.Shared.Return(args, clearArray: true);
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

    // =========================================================================
    // Methods formerly on Interpreter — now live directly on KiwiVM
    // =========================================================================

    /// <summary>Returns the global scope.</summary>
    public Scope GetGlobalScope() => _globals;

    /// <summary>
    /// Compile and run a single AST node through a fresh sub-VM.
    /// Used by the legacy tree-walker fallback paths.
    /// </summary>
    private Value Interpret(ASTNode? node)
    {
        if (node == null) return Value.Default;
        var prog = new ProgramNode([node!]);
        var chunk = Compiler.CompileExpression(prog);
        var savedVM = KiwiVM.Current;
        try   { return new KiwiVM(this).Execute(chunk); }
        finally { KiwiVM.Current = savedVM; }
    }

    public Value FuncToLambda(KFunction func)
    {
        var lambdaId  = func.Name;
        var lambdaDecl = func.Decl.ToLambda();
        var kl = new KLambda(lambdaDecl)
        {
            Parameters        = func.Parameters,
            DefaultParameters = func.DefaultParameters,
            TypeHints         = func.TypeHints,
            ReturnTypeHint    = func.ReturnTypeHint,
            CapturedScope     = func.CapturedScope,
            VariadicParamName = func.VariadicParamName,
            VMChunk           = func.VMChunk,
            VMUpvalues        = func.VMUpvalues,
            Ref               = new LambdaRef { Identifier = lambdaId }
        };
        _context.Lambdas[lambdaId] = kl;
        return Value.CreateLambda(new LambdaRef { Identifier = lambdaId, VMChunk = func.VMChunk, VMUpvalues = func.VMUpvalues });
    }

    public Value InvokeCallable(Callable callable, IReadOnlyList<Value> args, Token token, string displayName, InstanceRef? instance = null)
    {
        var vm = KiwiVM.Current ?? this;
        if (callable is KFunction kf && kf.VMChunk != null)
            return vm.InvokeVMCallable(kf, args, token, instance);
        if (callable is KLambda kl && kl.VMChunk != null)
            return vm.InvokeVMCallable(kl, args, token, instance);
        throw new InvalidOperationError(token, $"Callable `{displayName}` has no compiled VM chunk.");
    }

    public Value InvokeEvent(Token token, LambdaRef lambda, List<Value> args)
    {
        var lambdaName   = lambda.Identifier;
        var targetLambda = lambdaName;

        if (!_context.HasLambda(targetLambda))
        {
            if (!_context.HasMappedLambda(targetLambda))
                throw new CallableError(token, $"Could not find target lambda `{targetLambda}`");
            targetLambda = _context.LambdaTable[targetLambda];
        }

        var func  = _context.Lambdas[targetLambda];
        var subVm = new KiwiVM(this, func.CapturedScope ?? GetGlobalScope());
        return subVm.InvokeVMCallable(func, args, token, null);
    }

    public Value IncludeFile(Token token, string filePath)
    {
        var fullPath = ResolveIncludePath(token, filePath);

        if (_context.Includes.Contains(fullPath))
            return Value.Default;

        _context.Includes.Add(fullPath);

        if (!FileUtil.FileExists(token, fullPath))
            throw new InvalidOperationError(token, $"File not found: {filePath}");

        var oldExecutionPath = ExecutionPath;
        var savedVM = KiwiVM.Current;
        ExecutionPath = fullPath;

        try
        {
            using var lexer = new Lexer(fullPath);
            var ast   = new Parser(true).ParseTokenStream(lexer.GetTokenStream(), true);
            var chunk = Compiler.CompileProgram((ProgramNode)ast);
            new KiwiVM(this).Execute(chunk);
        }
        finally
        {
            KiwiVM.Current = savedVM;
            ExecutionPath  = oldExecutionPath;
        }

        return Value.Default;
    }

    private string ResolveIncludePath(Token token, string filePath)
    {
        var currentDir = FileUtil.GetParentPath(token, ExecutionPath);

        bool isExplicitRelative = filePath.StartsWith("./",  StringComparison.Ordinal)
                               || filePath.StartsWith("../", StringComparison.Ordinal);

        if (isExplicitRelative)
            return FindWithExtensions(Path.GetFullPath(Path.Combine(currentDir, filePath)));

        var fromRoot = FindWithExtensions(Path.GetFullPath(Path.Combine(ProjectRoot, filePath)));
        if (File.Exists(fromRoot))
            return fromRoot;

        return FindWithExtensions(Path.GetFullPath(Path.Combine(currentDir, filePath)));
    }

    private static string FindWithExtensions(string p)
    {
        if (Path.HasExtension(p)) return p;
        foreach (var ext in Kiwi.Settings.Extensions.Recognized)
            if (File.Exists(p + ext)) return p + ext;
        return p + Kiwi.Settings.Extensions.Primary;
    }

    public Value CreateGeneratorFromValues(KFunction func, IReadOnlyList<Value> args, Token token)
    {
        var ownedArgs        = args.ToArray();
        var generatorRef     = new GeneratorRef();
        var capturedGlobal   = _globals;
        var capturedContext  = _context;
        var capturedExecPath = ExecutionPath;
        var capturedEntryPath = EntryPath;
        var capturedFunc     = func;
        var capturedToken    = token;
        var capturedCliArgs  = CliArgs;
        var capturedRoot     = ProjectRoot;

        generatorRef.Start(() =>
        {
            // Build a minimal root VM to host the generator thread.
            // We can't reuse a KiwiVM(parent) constructor here because the
            // calling VM may be on another thread; build from context directly.
            var genVM = new KiwiVM(capturedGlobal, capturedContext,
                capturedExecPath, capturedEntryPath, capturedRoot, capturedCliArgs);
            genVM._activeGenerator = generatorRef;
            genVM.RunGeneratorBody(capturedFunc, ownedArgs, capturedToken, generatorRef);
        });

        return Value.CreateGenerator(generatorRef);
    }

    /// <summary>Private constructor used by CreateGeneratorFromValues to build a thread-local VM.</summary>
    private KiwiVM(Scope globals, KContext context, string execPath, string entryPath, string projectRoot, Dictionary<string, string> cliArgs)
    {
        _context      = context;
        _globals      = globals;
        ExecutionPath = execPath;
        EntryPath     = entryPath;
        ProjectRoot   = projectRoot;
        CliArgs       = cliArgs;
        Current = this;
    }

    internal void RunGeneratorBody(KFunction func, IReadOnlyList<Value> args, Token token, GeneratorRef generatorRef)
    {
        _activeGenerator = generatorRef;
        var subVm = new KiwiVM(this, func.CapturedScope ?? _globals);
        subVm._activeGenerator = generatorRef;
        subVm.InvokeVMCallable(func, args, token, null);
    }

    public Value EvalCode(Token token, string code)
    {
        using Lexer lexer = new(token.Span.File, code);
        var ast    = new Parser(true).ParseTokenStream(lexer.GetTokenStream(), true);
        var chunk  = Compiler.CompileExpression((ProgramNode)ast);
        var savedVM = KiwiVM.Current;
        try   { return new KiwiVM(this).Execute(chunk); }
        finally { KiwiVM.Current = savedVM; }
    }

    public Value ExecuteBuiltin(Token token, TokenName op, List<Value> args)
    {
        if (KiwiBuiltin.IsBuiltin(op))
            return KiwiBuiltinHandler.Execute(token, op, args, ExecutionPath, EntryPath);
        if (ReflectorBuiltin.IsBuiltin(op))
            return ReflectorBuiltinHandler.Execute(this, token, op, args, _context, CallStack);
        if (TaskBuiltin.IsBuiltin(op))
            return TaskBuiltinHandler.Execute(token, op, args, _context);
        if (ChannelBuiltin.IsBuiltin(op))
            return ChannelBuiltinHandler.Execute(token, op, args);
        if (SocketBuiltin.IsBuiltin(op))
            return SocketBuiltinHandler.Execute(token, op, args);
        if (TlsSocketBuiltin.IsBuiltin(op))
            return TlsSocketBuiltinHandler.Execute(token, op, args);
        return BuiltinDispatch.Execute(token, op, args, CliArgs);
    }

    private Value CallPackageMethod(MethodCallNode node, PackageRef pkg)
    {
        var qualifiedName = pkg.Identifier + "::" + node.MethodName;
        if (_context.Functions.TryGetValue(qualifiedName, out KFunction? func))
        {
            var args = node.Arguments
                .Select(a => a is LiteralNode ln ? ln.Value : Value.Default)
                .ToList();

            if (func.IsGenerator)
                return CreateGeneratorFromValues(func, args, node.Token);

            return InvokeCallable(func, args, node.Token, qualifiedName);
        }
        throw new FunctionUndefinedError(node.Token, node.MethodName);
    }

    internal void RegisterTypeBuiltins(string packageName)
    {
        if (TypeRegistry.TryGetPrimitiveType(packageName, out int type))
        {
            var prefix = packageName + "::";
            var funcs  = _context.Functions.Where(x => x.Key.StartsWith(prefix)).ToList();
            foreach (var func in funcs)
                TypeBuiltins.Register(type, func.Key.Replace(prefix, string.Empty), func.Value);
        }
    }

    public void ImportPackage(Token token, Value packageName)
    {
        if (!packageName.IsString())
            throw new InvalidOperationError(token, "Expected the name of a package to import.");

        var packageNameValue = packageName.GetString();

        if (_context.ImportedPackages.Contains(packageNameValue))
            return;

        if (!_context.HasPackage(packageNameValue))
        {
            var packagePath = FileUtil.TryGetExtensionless(token, packageNameValue, ExecutionPath);
            if (FileUtil.IsScript(token, packagePath))
            {
                ImportExternal(token, packagePath);
                return;
            }
            throw new PackageUndefinedError(token, packageNameValue);
        }

        PackageStack.Push(packageNameValue);
        var decl = _context.Packages[packageNameValue].Decl;

        if (decl.Body.Count > 0)
        {
            var prog    = new ProgramNode([decl]);
            var chunk   = Compiler.CompileProgram(prog);
            var savedVM = KiwiVM.Current;
            try { new KiwiVM(this).Execute(chunk); }
            finally { KiwiVM.Current = savedVM; }
        }

        if (PackageStack.Count > 0)
            PackageStack.Pop();

        RegisterTypeBuiltins(packageNameValue);
        _context.ImportedPackages.Add(packageNameValue);
    }

    private void ImportExternal(Token token, string packageName)
    {
        var packagePath = FileUtil.TryGetExtensionless(token, packageName);
        var content     = FileUtil.ReadFile(token, packagePath);
        if (string.IsNullOrEmpty(content)) return;

        var path = FileUtil.GetAbsolutePath(token, packagePath);

        if (_context.Includes.Contains(path)) return;
        _context.Includes.Add(path);

        using Lexer lexer = new(path);
        var tokenStream = lexer.GetTokenStream();
        var ast   = (ProgramNode)new Parser(true).ParseTokenStream(tokenStream, true);
        var chunk = Compiler.CompileProgram(ast);
        var savedVM = KiwiVM.Current;
        try { new KiwiVM(this).Execute(chunk); }
        finally { KiwiVM.Current = savedVM; }
    }

    public void PushVMDispatchFrame(Scope globals, InstanceRef? self = null)
    {
        var frame = new StackFrame("<vm-dispatch>", globals);
        if (self != null) frame.SetObjectContext(self);
        CallStack.Push(frame);
    }

    public void PopVMDispatchFrame() => CallStack.Pop();

    public Value DispatchMethod(Value obj, string methodName, IReadOnlyList<Value> args, Token token)
    {
        if (obj.IsPackage())
        {
            var argNodes = args.Select(a => (ASTNode?)new LiteralNode(a) { Token = token }).ToList();
            var node = new MethodCallNode(null, methodName, TokenName.Ops_Assign, argNodes) { Token = token };
            return CallPackageMethod(node, obj.GetPackage());
        }
        if (obj.IsObject()) return CallObjectMethodDirect(token, methodName, obj.GetObject(), args);
        if (obj.IsStruct()) return CallStructMethodDirect(token, methodName, obj.GetStruct(), args);

        if (ListBuiltin.Map.TryGetValue(methodName, out TokenName listOp))
        {
            bool pushedFrame = CallStack.Count == 0;
            if (pushedFrame) CallStack.Push(new StackFrame("<vm-dispatch>", _globals));
            try   { return ListBuiltinHandler.HandleListBuiltin(this, token, ref obj, listOp, args); }
            finally { if (pushedFrame) CallStack.Pop(); }
        }
        if (CallableBuiltin.Map.TryGetValue(methodName, out _))
            return HandleCallableBuiltinDirect(token, obj, methodName, args);
        if (CoreBuiltin.Map.TryGetValue(methodName, out TokenName coreOp))
            return BuiltinDispatch.Execute(token, coreOp, obj, args);

        var typeId = TypeRegistry.GetType(token, obj);
        if (TypeBuiltins.TryGetBuiltin(typeId, methodName, out KFunction? typeBuiltin) && typeBuiltin != null)
        {
            var tArgs = new List<Value>(args.Count + 1) { obj };
            tArgs.AddRange(args);
            return InvokeCallable(typeBuiltin, tArgs, token, methodName);
        }

        throw new FunctionUndefinedError(token, methodName);
    }

    private static readonly HashSet<TokenName> _objectSpecificBuiltins =
    [
        TokenName.Builtin_Core_Clone,
        TokenName.Builtin_Core_Get,
        TokenName.Builtin_Core_HasKey,
        TokenName.Builtin_Core_IsA,
        TokenName.Builtin_Core_Keys,
        TokenName.Builtin_Core_Set,
        TokenName.Builtin_Core_Values,
    ];

    private Value CallObjectMethodDirect(Token token, string methodName, InstanceRef obj, IReadOnlyList<Value> args)
    {
        if (!_context.HasStruct(obj.StructName))
            throw new StructUndefinedError(token, obj.StructName);

        var struc = _context.Structs[obj.StructName];
        if (!TryResolveStructMethod(struc, obj, methodName, out KFunction? fn))
        {
            if (CoreBuiltin.Map.TryGetValue(methodName, out TokenName coreOp))
            {
                if (_objectSpecificBuiltins.Contains(coreOp))
                    return ObjectBuiltinHandler.Handle(this, token, coreOp, obj, struc.BaseStruct, args);
                return BuiltinDispatch.Execute(token, coreOp, Value.CreateObject(obj), args);
            }
            throw new UnimplementedMethodError(token, obj.StructName, methodName);
        }

        bool isCtor = methodName == "new";
        if (isCtor)
        {
            var inst = new InstanceRef { StructName = obj.StructName, Identifier = obj.StructName };
            return InvokeCallable(fn!, args, token, methodName, inst);
        }
        return InvokeCallable(fn!, args, token, methodName, obj);
    }

    private bool TryResolveStructMethod(KStruct struc, InstanceRef obj, string methodName, out KFunction? fn)
    {
        if (struc.Methods.TryGetValue(methodName, out fn)) return true;
        if (!string.IsNullOrEmpty(struc.BaseStruct) && _context.HasStruct(struc.BaseStruct))
            return TryResolveStructMethod(_context.Structs[struc.BaseStruct], obj, methodName, out fn);
        fn = null;
        return false;
    }

    private Value CallStructMethodDirect(Token token, string methodName, StructRef sref, IReadOnlyList<Value> args)
    {
        var structName = sref.Identifier;
        if (!_context.HasStruct(structName))
            throw new StructUndefinedError(token, structName);
        var struc = _context.Structs[structName];

        if (methodName == "new" && struc.IsAbstract)
            throw new AbstractInstantiationError(token, structName);

        KFunction? fn   = null;
        var search = struc;
        while (search != null)
        {
            if (search.Methods.TryGetValue(methodName, out fn)) break;
            fn     = null;
            search = !string.IsNullOrEmpty(search.BaseStruct) && _context.HasStruct(search.BaseStruct)
                ? _context.Structs[search.BaseStruct]
                : null;
        }

        if (fn == null)
        {
            if (methodName == "new")
            {
                var inst = new InstanceRef { StructName = structName, Identifier = structName };
                return Value.CreateObject(inst);
            }
            throw new UnimplementedMethodError(token, structName, methodName);
        }

        if (methodName == "new")
        {
            var inst = new InstanceRef { StructName = structName, Identifier = structName };
            InvokeCallable(fn, args, token, methodName, inst);
            return Value.CreateObject(inst);
        }
        var structSelf = new InstanceRef { StructName = structName, Identifier = structName };
        return InvokeCallable(fn, args, token, methodName, structSelf);
    }

    private Value HandleCallableBuiltinDirect(Token token, Value obj, string methodName, IReadOnlyList<Value> args)
    {
        if (!CallableBuiltin.Map.TryGetValue(methodName, out var op))
            throw new FunctionUndefinedError(token, methodName);

        Callable? callable  = null;
        string? callableName = null;

        if (obj.IsLambda())
        {
            callableName = obj.GetLambda().Identifier;
            if (_context.HasLambda(callableName))       callable = _context.Lambdas[callableName];
            else if (_context.HasMappedLambda(callableName)) callable = _context.Lambdas[_context.LambdaTable[callableName]];
            else if (_context.HasFunction(callableName)) callable = _context.Functions[callableName];
        }

        if (callableName == null)
            throw new InvalidOperationError(token, $"Expected a callable for function `{CallableBuiltin.MapName(op)}`.");
        if (callable == null)
            throw new InvalidOperationError(token, $"Expected a callable for function `{CallableBuiltin.MapName(op)}` on `{callableName}`.");
        if (callable is not KFunction && callable is not KLambda)
            throw new InvalidOperationError(token, $"Expected a function or lambda for function `{CallableBuiltin.MapName(op)}` on `{callableName}`.");

        return CallableBuiltinHandler.Execute(this, token, op, callable, callableName, args as List<Value> ?? new List<Value>(args));
    }

    public Value GetIndex(Value obj, Value key, Token token)
    {
        if (obj.IsList())
        {
            var idx  = kiwi.Runtime.Builtin.Operation.ConversionOp.GetInteger(token, key);
            var list = obj.GetList();
            if (idx < 0 || idx >= list.Count)
                throw new IndexError(token, "List index out of bounds.");
            return list[(int)idx];
        }
        if (obj.IsHashmap())
        {
            obj.GetHashmap().TryGetValue(key, out var v);
            return v ?? Value.CreateNull();
        }
        if (obj.IsString())
        {
            var s   = obj.GetString();
            var idx = kiwi.Runtime.Builtin.Operation.ConversionOp.GetInteger(token, key);
            if (idx < 0 || idx >= s.Length)
                throw new IndexError(token, "String index out of bounds.");
            return Value.CreateString(s[(int)idx].ToString());
        }
        if (obj.IsBytes())
        {
            var bytes = obj.GetBytes();
            var idx   = kiwi.Runtime.Builtin.Operation.ConversionOp.GetInteger(token, key);
            if (idx < 0 || idx >= bytes.Length)
                throw new IndexError(token, "Bytes index out of bounds.");
            return Value.CreateInteger(bytes[(int)idx]);
        }
        throw new InvalidOperationError(token, "Cannot index this type.");
    }

    public void SetIndex(Value obj, Value key, Value value, Token token)
    {
        if (obj.IsList())
        {
            var idx  = kiwi.Runtime.Builtin.Operation.ConversionOp.GetInteger(token, key);
            var list = obj.GetList();
            if (idx < 0 || idx >= list.Count)
                throw new IndexError(token, "List index out of bounds.");
            list[(int)idx] = value;
            return;
        }
        if (obj.IsHashmap())
        {
            obj.GetHashmap()[key] = value;
            return;
        }
        throw new InvalidOperationError(token, "Cannot index-assign this type.");
    }

    public Value GetSlice(Value obj, Value? startV, Value? stopV, Value? stepV, Token token)
    {
        var startVal = startV ?? Value.CreateInteger(0L);
        var stopVal  = stopV  ?? (obj.IsList()   ? Value.CreateInteger(obj.GetList().Count) :
                                  obj.IsString() ? Value.CreateInteger(obj.GetString().Length) :
                                  obj.IsBytes()  ? Value.CreateInteger(obj.GetBytes().Length) :
                                                   Value.CreateInteger(0L));
        var stepVal  = stepV  ?? Value.CreateInteger(1L);
        var slice    = new SliceIndex(startVal, stopVal, stepVal) { IsSlice = true };

        if (obj.IsString()) return kiwi.Runtime.Builtin.Util.SliceUtil.StringSlice(token, slice, obj.GetString());
        if (obj.IsList())   return kiwi.Runtime.Builtin.Util.SliceUtil.ListSlice(token, slice, obj.GetList());
        if (obj.IsBytes())  return kiwi.Runtime.Builtin.Util.SliceUtil.BytesSlice(token, slice, obj.GetBytes());
        throw new InvalidOperationError(token, "Cannot slice this type.");
    }

    public void SetSlice(Value obj, Value? startV, Value? stopV, Value? stepV, Value rhs, Token token)
    {
        var startVal = startV ?? Value.CreateInteger(0L);
        var stopVal  = stopV  ?? (obj.IsList()  ? Value.CreateInteger(obj.GetList().Count) :
                                  obj.IsBytes() ? Value.CreateInteger(obj.GetBytes().Length) :
                                                  Value.CreateInteger(0L));
        var stepVal  = stepV  ?? Value.CreateInteger(1L);
        var slice    = new SliceIndex(startVal, stopVal, stepVal) { IsSlice = true };

        if (obj.IsList() && rhs.IsList())
        {
            var targetList = obj.GetList();
            var rhsValues  = rhs.GetList();
            kiwi.Runtime.Builtin.Util.SliceUtil.UpdateListSlice(token, false, ref targetList, slice, rhsValues);
        }
    }

    public Value GetMember(Value obj, string memberName, Token token)
    {
        if (obj.IsObject())
        {
            var inst = obj.GetObject();
            if (!inst.InstanceVariables.TryGetValue("@" + memberName, out var v))
                inst.InstanceVariables.TryGetValue(memberName, out v);
            return v ?? Value.Default;
        }
        if (obj.IsPackage())
        {
            var pkgName       = obj.GetPackage().Identifier;
            var qualifiedName = pkgName + "::" + memberName;
            if (_context.Constants.TryGetValue(qualifiedName, out Value? constVal))
                return constVal;
            if (_context.PackageVariables.TryGetValue(qualifiedName, out Value? pkgVar))
                return pkgVar;
            if (_context.HasFunction(qualifiedName))
                return Value.CreateLambda(new LambdaRef { Identifier = qualifiedName });
            return Value.Default;
        }
        if (obj.IsHashmap())
        {
            obj.GetHashmap().TryGetValue(Value.CreateString(memberName), out var v);
            return v ?? Value.Default;
        }
        if (obj.IsStruct())
        {
            var kstruct = _context.Structs[obj.GetStruct().Identifier];
            if (kstruct.StaticVariables.TryGetValue("@@" + memberName, out var sv))
                return sv;
            return Value.Default;
        }
        throw new InvalidOperationError(token, $"Cannot access member '{memberName}' on this type.");
    }

    public void SetMember(Value obj, string memberName, Value value, Token token)
    {
        if (obj.IsObject())
        {
            var iv  = obj.GetObject().InstanceVariables;
            var key = iv.ContainsKey("@" + memberName) ? "@" + memberName : memberName;
            iv[key] = value;
            return;
        }
        if (obj.IsHashmap())
        {
            obj.GetHashmap()[Value.CreateString(memberName)] = value;
            return;
        }
        if (obj.IsStruct())
        {
            var sName = obj.GetStruct().Identifier;
            if (_context.HasStruct(sName))
                _context.Structs[sName].StaticVariables["@@" + memberName] = value;
            return;
        }
        if (obj.IsPackage()) return; // no-op
        throw new InvalidOperationError(token, $"Cannot set member '{memberName}' on this type.");
    }

    public Value CreateObject(string structName, IReadOnlyList<Value> args, Token token)
    {
        if (!_context.HasStruct(structName))
            throw new StructUndefinedError(token, structName);
        var struc = _context.Structs[structName];
        var inst  = new InstanceRef { StructName = structName, Identifier = structName };
        if (struc.Methods.TryGetValue("new", out KFunction? ctor))
            InvokeCallable(ctor, args, token, "new", inst);
        return Value.CreateObject(inst);
    }

    public string Serialize(Value v) => Serializer.Serialize(v);

    public void YieldFromVM(Value v)
    {
        if (_activeGenerator == null)
            throw new RuntimeError(Token.Eof, "'yield' used outside of a generator.", []);
        _activeGenerator.Yield(v);
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
