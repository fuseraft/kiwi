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
    /// </summary>
    public string      StructName { get; set; } = string.Empty;

    /// <summary>
    /// Token at the call site — used for error tracing.
    /// </summary>
    public Token?      CallSiteToken { get; set; }

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
