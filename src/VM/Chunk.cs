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
/// A type hint the VM checks at runtime (see <see cref="Opcode.CheckType"/>).
/// </summary>
/// <param name="Types">The accepted types (TypeRegistry ids); a value matching any of them passes.</param>
/// <param name="Subject">What is being checked, for the error message (e.g. "parameter `x` of `f`").</param>
/// <param name="IsReturn">True for a return-value check, which a generator skips.</param>
/// <param name="IsParameter">True for a parameter check, reported at the call site.</param>
public sealed record TypeCheck(int[] Types, string Subject, bool IsReturn = false, bool IsParameter = false);

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
    public List<int>               Columns   { get; } = [];
    public List<int>               FileIds   { get; } = [];

    /// <summary>
    /// Maps each named local variable to its stack slot index (relative to frame base).
    /// Populated by the compiler for debugger introspection; empty for global-scope chunks.
    /// </summary>
    public List<(string Name, int Slot)> LocalNames { get; } = [];

    // -- Fallback AST nodes ----------------------------------------------------
    public List<ASTNode>           NodePool  { get; } = [];

    // -- Named-argument sets ---------------------------------------------------
    /// <summary>
    /// One entry per named-arg call site.  Each element is a string[] parallel
    /// to the pushed arguments: empty string = positional, non-empty = arg name.
    /// Indexed by B in CallNamed; by (B >> 16) in CallMethodNamed.
    /// </summary>
    public List<string[]> ArgNameSets { get; } = [];

    /// <summary>
    /// Add a named-argument set and return its index.
    /// </summary>
    public int AddArgNameSet(string[] names)
    {
        ArgNameSets.Add(names);
        return ArgNameSets.Count - 1;
    }

    // -- Function metadata -----------------------------------------------------
    /// <summary>
    /// True when this chunk contains at least one Yield opcode (i.e. it is a generator function).
    /// Set by the compiler when CompileYield is called.
    /// </summary>
    public bool IsGenerator { get; set; }

    /// <summary>
    /// The package prefix (e.g. "foo::") of the package this function was defined in.
    /// Empty for top-level functions.  Set by the VM at DefFunc time.
    /// Used by LoadGlobal to resolve unqualified names inside package functions.
    /// </summary>
    public string PackagePrefix { get; set; } = string.Empty;

    // Names[i] qualified with PackagePrefix, built on first use.
    private string[]? _qualifiedNames;
    private string    _qualifiedNamesPrefix = string.Empty;

    /// <summary>
    /// The name at <paramref name="nameIdx"/> qualified with <see cref="PackagePrefix"/>
    /// (e.g. "faker::person"), cached so LoadGlobal doesn't build the string on every access.
    /// </summary>
    public string QualifiedName(int nameIdx)
    {
        var prefix = PackagePrefix;
        var names  = _qualifiedNames;
        if (names == null || names.Length != Names.Count || !ReferenceEquals(_qualifiedNamesPrefix, prefix))
        {
            names = new string[Names.Count];
            _qualifiedNamesPrefix = prefix;
            _qualifiedNames       = names;
        }
        return names[nameIdx] ??= prefix + Names[nameIdx];
    }

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
    /// Parameter names that have default expressions (subset of ParamNames).
    /// Populated by the compiler so the VM can mark them optional in KFunction/KLambda.
    /// </summary>
    public HashSet<string> DefaultParamNames { get; } = [];

    /// <summary>
    /// Number of leading parameters a call must pass: those up to the last one without a default.
    /// </summary>
    public int MinArity { get; set; }

    /// <summary>
    /// True for a lambda. Lambdas accept any number of arguments (missing ones are null,
    /// extra ones ignored) so they can serve as callbacks; `fn` functions and methods don't.
    /// </summary>
    public bool IsLambda { get; set; }

    /// <summary>
    /// Type hints checked by CheckType instructions, indexed by their A operand.
    /// </summary>
    public List<TypeCheck> TypeChecks { get; } = [];

    // -- Emit helpers ----------------------------------------------------------

    /// <summary>
    /// Append one instruction and return its index.
    /// </summary>
    public int Emit(Opcode op, int a = 0, int b = 0, int line = 0, int fileId = 0, int column = 0)
    {
        Code.Add(new Instruction(op, a, b));
        Lines.Add(line);
        Columns.Add(column);
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
    /// Store an AST node in the node pool and return its index.
    /// Used by CallBuiltin (to pass Token/Op) and PackageBegin (to store the AST for retry).
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

    public int GetColumn(int instrIdx) =>
        (uint)instrIdx < (uint)Columns.Count ? Columns[instrIdx] : 0;

    public int GetFileId(int instrIdx) =>
        (uint)instrIdx < (uint)FileIds.Count ? FileIds[instrIdx] : 0;
}
