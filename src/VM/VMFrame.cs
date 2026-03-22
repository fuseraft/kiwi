using kiwi.Parsing;
using kiwi.Typing;

namespace kiwi.VM;

/// <summary>
/// Execution state for one active function call inside the KiwiVM.
/// </summary>
public sealed class VMFrame(string name, Chunk chunk, int stackBase, Upvalue[] upvalues)
{
    /// <summary>
    /// The compiled code unit for this function.
    /// </summary>
    public Chunk Chunk { get; } = chunk;

    /// <summary>
    /// Instruction pointer: index of the next instruction to execute.
    /// </summary>
    public int IP { get; set; } = 0;

    /// <summary>
    /// Absolute index in the VM's shared value stack where this frame's
    /// local slot 0 begins.  Parameters and locals live at
    /// <c>_stack[StackBase .. StackBase + Chunk.LocalCount - 1]</c>.
    /// </summary>
    public int StackBase { get; } = stackBase;

    /// <summary>
    /// Upvalue cells captured by this closure (empty array if the function
    /// is not a closure or has no upvalues).
    /// </summary>
    public Upvalue[] Upvalues { get; } = upvalues;

    /// <summary>
    /// Current <c>@</c> object context (null outside struct methods).
    /// </summary>
    public InstanceRef? Self      { get; set; }

    /// <summary>
    /// Struct name when executing a struct method (used for <c>@@</c> access).
    /// Equals the concrete instance's struct name (e.g. "C" for a C instance).
    /// </summary>
    public string      StructName { get; set; } = string.Empty;

    /// <summary>
    /// The struct that declared the currently-executing method.
    /// Used to resolve <c>super.method()</c>: the search starts from
    /// <c>OwnerStruct.BaseStruct</c> rather than the concrete instance type.
    /// Empty for non-method frames and lambdas.
    /// </summary>
    public string      OwnerStruct { get; set; } = string.Empty;

    /// <summary>
    /// Token at the call site - used for error tracing.
    /// </summary>
    public Token?      CallSiteToken { get; set; }

    // -- Try handler stack (nested try/catch within this frame) ----------------
    // Each entry: (catchIP, finallyIP, savedSP)
    // null until first try block is entered (avoids alloc for non-try functions).
    private Stack<(int CatchIP, int FinallyIP, int SavedSP)>? _tryHandlers;
    public bool HasTryHandlers => _tryHandlers?.Count > 0;

    public void PushTryHandler(int catchIP, int finallyIP, int savedSP)
    {
        _tryHandlers ??= new Stack<(int, int, int)>();
        _tryHandlers.Push((catchIP, finallyIP, savedSP));
    }

    public bool PopTryHandler(out int catchIP, out int finallyIP, out int savedSP)
    {
        if (_tryHandlers?.Count > 0)
        {
            var (c, f, s) = _tryHandlers.Pop();
            catchIP = c; finallyIP = f; savedSP = s;
            return true;
        }
        catchIP = finallyIP = savedSP = 0;
        return false;
    }

    /// <summary>
    /// Names of functions defined inside this frame that must be removed from
    /// the global function table when the frame returns (prevents leaking).
    /// Null until first inner function is defined (avoids alloc for most frames).
    /// </summary>
    private HashSet<string>? _localFunctions;

    public void TrackLocalFunction(string name)
    {
        _localFunctions ??= [];
        _localFunctions.Add(name);
    }

    public HashSet<string>? LocalFunctions => _localFunctions;

    /// <summary>
    /// Human-readable name of this function/lambda for stack traces.
    /// </summary>
    public string Name { get; } = name;

    public string FormatTrace()
    {
        if (CallSiteToken == null)
            return $"at {Name}";

        var tok = CallSiteToken!.Value;
        var path = Tracing.FileRegistry.Instance.GetFilePath(tok.Span.File);
        return $"at {Name} in {path}:{tok.Span.Line}:{tok.Span.Pos}";
    }
}
