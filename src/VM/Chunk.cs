using kiwi.Parsing.AST;
using kiwi.Typing;

namespace kiwi.VM;

/// <summary>
/// A single bytecode instruction.
/// </summary>
/// <param name="Op">The opcode.</param>
/// <param name="A">Primary operand (constant index, slot, jump target, count, …).</param>
/// <param name="B">Secondary operand.</param>
public readonly record struct Instruction(Opcode Op, int A = 0, int B = 0);

/// <summary>
/// Describes how a closure captures a variable from an enclosing scope.
/// </summary>
public struct UpvalueDescriptor
{
    /// <summary>
    /// If true, captures a local slot from the immediately enclosing function frame.
    /// If false, captures an upvalue (by index) from the enclosing closure's upvalue array.
    /// </summary>
    public bool IsLocal;

    /// <summary>
    /// When IsLocal=true:  the local slot index (relative to enclosing frame base).
    /// When IsLocal=false: the upvalue index in the enclosing closure.
    /// </summary>
    public int Index;
}

/// <summary>
/// A compiled code unit - one Chunk per function, lambda, or top-level program.
/// </summary>
public class Chunk
{
    public string Name { get; set; } = "<chunk>";

    // -- Bytecode --------------------------------------------------------------
    public List<Instruction>       Code      { get; } = [];

    // -- Constant pool ---------------------------------------------------------
    public List<Value>             Constants { get; } = [];

    // -- Name pool -------------------------------------------------------------
    public List<string>            Names     { get; } = [];
    private readonly Dictionary<string, int> _nameIndex = new(StringComparer.Ordinal);

    // -- Nested function / lambda chunks --------------------------------------
    public List<Chunk>             SubChunks { get; } = [];

    // -- Upvalue descriptors (for closure sub-chunks) --------------------------
    public List<UpvalueDescriptor> Upvalues  { get; } = [];

    // -- Debug info ------------------------------------------------------------
    // One entry per instruction, parallel arrays.
    public List<int>               Lines     { get; } = [];
    public List<int>               FileIds   { get; } = [];

    // -- Fallback AST nodes ----------------------------------------------------
    public List<ASTNode>           NodePool  { get; } = [];

    // -- Function metadata -----------------------------------------------------
    /// <summary>
    /// Number of explicit parameters (not counting variadic).
    /// </summary>
    public int Arity { get; set; }

    /// <summary>
    /// Total number of local-variable stack slots reserved for this frame
    /// (includes parameters and all pre-scanned locals).
    /// </summary>
    public int LocalCount { get; set; }

    /// <summary>
    /// Parameter names in declaration order.
    /// </summary>
    public List<string> ParamNames { get; } = [];

    /// <summary>
    /// Name of the variadic parameter, empty if none.
    /// </summary>
    public string VariadicParamName { get; set; } = string.Empty;

    /// <summary>
    /// Maps every pre-scanned local variable to its stack slot index (relative to frame base).
    /// Populated by the compiler so the VM can build an interpreter scope for InterpFallback.
    /// </summary>
    public List<(string Name, int Slot)> LocalNames { get; } = [];

    /// <summary>
    /// Parameter names that have default expressions (subset of ParamNames).
    /// Populated by the compiler so the VM can mark them optional in KFunction/KLambda.
    /// </summary>
    public HashSet<string> DefaultParamNames { get; } = [];

    // -- Emit helpers ----------------------------------------------------------

    /// <summary>
    /// Append one instruction and return its index.
    /// </summary>
    public int Emit(Opcode op, int a = 0, int b = 0, int line = 0, int fileId = 0)
    {
        Code.Add(new Instruction(op, a, b));
        Lines.Add(line);
        FileIds.Add(fileId);
        return Code.Count - 1;
    }

    /// <summary>
    /// Add a constant to the pool and return its index.
    /// </summary>
    public int AddConstant(Value v)
    {
        Constants.Add(v);
        return Constants.Count - 1;
    }

    /// <summary>
    /// Intern a name string and return its index (deduplicated).
    /// </summary>
    public int AddName(string name)
    {
        if (_nameIndex.TryGetValue(name, out var idx)) return idx;
        idx = Names.Count;
        Names.Add(name);
        _nameIndex[name] = idx;
        return idx;
    }

    /// <summary>
    /// Add a sub-chunk and return its index.
    /// </summary>
    public int AddSubChunk(Chunk sub)
    {
        SubChunks.Add(sub);
        return SubChunks.Count - 1;
    }

    /// <summary>
    /// Store an AST node in the fallback pool and return its index.
    /// Used by the InterpFallback opcode.
    /// </summary>
    public int AddNodeFallback(ASTNode node)
    {
        NodePool.Add(node);
        return NodePool.Count - 1;
    }

    /// <summary>
    /// Overwrite the A operand of a previously emitted jump instruction.
    /// </summary>
    public void PatchJump(int instrIdx, int target)
    {
        var instr = Code[instrIdx];
        Code[instrIdx] = instr with { A = target };
    }

    /// <summary>
    /// Source line for the instruction at the given index (0 if unknown).
    /// </summary>
    public int GetLine(int instrIdx) =>
        (uint)instrIdx < (uint)Lines.Count ? Lines[instrIdx] : 0;

    public int GetFileId(int instrIdx) =>
        (uint)instrIdx < (uint)FileIds.Count ? FileIds[instrIdx] : 0;
}
