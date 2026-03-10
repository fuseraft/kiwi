using kiwi.Parsing;
using kiwi.Parsing.AST;
using kiwi.Parsing.Keyword;
using kiwi.Typing;

namespace kiwi.VM;

/// <summary>
/// Compiles a Kiwi AST into bytecode Chunks for the KiwiVM.
///
/// Variable resolution order (inside a function):
///   1. Local slot   — name assigned anywhere in the function body (pre-scanned)
///   2. Upvalue      — local in an enclosing function captured by this closure
///   3. Global       — everything else (runtime Scope lookup)
///
/// At top level (isGlobal=true) every name resolves as global.
/// </summary>
public sealed class Compiler
{
    // -- Local variable --------------------------------------------------------

    private struct LocalVar
    {
        public string Name;
        public int    Slot;
        public bool   Captured;
    }

    // -- Loop context (break / next patching) ---------------------------------

    private sealed class LoopCtx
    {
        public int       LoopTop;       // IP of loop condition or ForIterNext
        public int       ForIterSlot;   // -1 = no extra stack item (while/repeat)
        public List<int> BreakPatches = [];
        public List<int> NextPatches  = [];
    }

    // -- Fields ----------------------------------------------------------------

    private readonly Chunk     _chunk;
    private readonly Compiler? _enclosing;
    private readonly bool      _isGlobal;

    private readonly List<LocalVar>          _locals   = [];
    private readonly List<UpvalueDescriptor> _upvalues = [];
    private readonly List<LoopCtx>           _loops    = []; // last = innermost

    private int _slotCount;

    // -- Construction ----------------------------------------------------------

    private Compiler(string name, Compiler? enclosing, bool isGlobal)
    {
        _chunk     = new Chunk { Name = name };
        _enclosing = enclosing;
        _isGlobal  = isGlobal;
    }

    // -- Public entry points ---------------------------------------------------

    public static Chunk CompileProgram(ProgramNode program)
    {
        var c = new Compiler("<main>", null, isGlobal: true);
        foreach (var s in program.Statements)
            if (s != null) c.CompileStatement(s);
        c._chunk.Emit(Opcode.Halt);
        return c._chunk;
    }

    public static Chunk CompileFunction(FunctionNode fn, Compiler? enclosing)
    {
        var c = new Compiler(fn.Name, enclosing, isGlobal: false);
        c.SetupLocals(fn.Parameters, fn.VariadicParamName, fn.Body);
        c.EmitDefaultInits(fn.Parameters);
        if (!c.CompileFunctionBody(fn.Body))
            c.EmitFinalReturn();
        foreach (var uv in c._upvalues) c._chunk.Upvalues.Add(uv);
        return c._chunk;
    }

    public static Chunk CompileLambda(LambdaNode fn, Compiler? enclosing)
    {
        var c = new Compiler("<lambda>", enclosing, isGlobal: false);
        c.SetupLocals(fn.Parameters, fn.VariadicParamName, fn.Body);
        c.EmitDefaultInits(fn.Parameters);
        if (!c.CompileFunctionBody(fn.Body))
            c.EmitFinalReturn();
        foreach (var uv in c._upvalues) c._chunk.Upvalues.Add(uv);
        return c._chunk;
    }

    // -- Default parameter initialization --------------------------------------

    /// <summary>
    /// For each parameter that has a default expression, emits a null-check at
    /// function entry: if the slot is still null (caller omitted the arg), evaluate
    /// the default expression and store it into the slot.
    ///   LoadLocal slot; Null; Eq; JumpF skip; [default expr]; StoreLocal slot; skip:
    /// </summary>
    private void EmitDefaultInits(List<KeyValuePair<string, ASTNode?>> parameters)
    {
        for (int i = 0; i < parameters.Count; i++)
        {
            var (name, defaultExpr) = parameters[i];
            if (defaultExpr == null) continue;

            _chunk.DefaultParamNames.Add(name);

            int slot = _locals.FindIndex(l => l.Name == name);
            if (slot < 0) continue;

            _chunk.Emit(Opcode.LoadLocal, slot);
            _chunk.Emit(Opcode.Null);
            _chunk.Emit(Opcode.Eq);
            int jumpIdx = _chunk.Emit(Opcode.JumpF, 0); // jump over default if arg was provided
            if (CompileNode(defaultExpr))
                _chunk.Emit(Opcode.StoreLocal, slot);
            else
                _chunk.Emit(Opcode.Null); // no-op fallback (shouldn't happen for valid defaults)
            _chunk.PatchJump(jumpIdx, _chunk.Code.Count);
        }
    }

    // -- Local variable setup --------------------------------------------------

    private void SetupLocals(
        List<KeyValuePair<string, ASTNode?>> parameters,
        string variadicParam,
        List<ASTNode?> body)
    {
        _chunk.Arity             = parameters.Count;
        _chunk.VariadicParamName = variadicParam;

        foreach (var (name, _) in parameters)
        {
            _chunk.ParamNames.Add(name);
            DeclareLocal(name);
        }
        if (!string.IsNullOrEmpty(variadicParam))
        {
            _chunk.ParamNames.Add(variadicParam);
            DeclareLocal(variadicParam);
        }

        foreach (var name in CollectAssigned(body))
        {
            if (!_locals.Any(l => l.Name == name) && ResolveUpvalue(name) < 0)
                DeclareLocal(name);
        }

        _chunk.LocalCount = _slotCount;

        // Publish name→slot mapping so the VM can expose locals to interpreter fallbacks.
        foreach (var lv in _locals)
            _chunk.LocalNames.Add((lv.Name, lv.Slot));
    }

    // -- Name pre-scan ---------------------------------------------------------

    private static HashSet<string> CollectAssigned(IEnumerable<ASTNode?> stmts)
    {
        var out_ = new HashSet<string>(StringComparer.Ordinal);
        foreach (var s in stmts)
            if (s != null) ScanNode(s, out_);
        return out_;
    }

    private static void ScanNode(ASTNode node, HashSet<string> out_)
    {
        switch (node.Type)
        {
            case ASTNodeType.Assignment:
            {
                var a = (AssignmentNode)node;
                // Left is null or an IdentifierNode for simple `x = expr` assignments
                if ((a.Left == null || a.Left is IdentifierNode) && !string.IsNullOrEmpty(a.Name))
                    out_.Add(a.Name);
                if (a.Initializer != null) ScanNode(a.Initializer, out_);
                if (a.Condition   != null) ScanNode(a.Condition,   out_);
                break;
            }
            case ASTNodeType.PackAssignment:
            {
                foreach (var lhs in ((PackAssignmentNode)node).Left)
                    if (lhs is IdentifierNode id) out_.Add(id.Name);
                break;
            }
            case ASTNodeType.Variable:
                foreach (var (n, _) in ((VariableNode)node).Variables) out_.Add(n);
                break;
            case ASTNodeType.ForLoop:
            {
                // Iterator variables are always loop-locals; body assignments follow
                // outer-scope shadowing (resolved at runtime via EmitStore).
                var fl = (ForLoopNode)node;
                if (fl.ValueIterator is IdentifierNode vi) out_.Add(vi.Name);
                if (fl.IndexIterator is IdentifierNode ii) out_.Add(ii.Name);
                break;
            }
            case ASTNodeType.RepeatLoop:
            {
                var rl = (RepeatLoopNode)node;
                if (rl.Alias is IdentifierNode al) out_.Add(al.Name);
                break;
            }
            case ASTNodeType.If:
                // Do not recurse: assignments inside conditional branches should resolve
                // against outer scope at runtime (StoreLocal if already a local, StoreGlobal
                // otherwise), matching the tree-walker's dynamic-scope behaviour.
                break;
            case ASTNodeType.WhileLoop:
                break; // same reasoning as If
            case ASTNodeType.Try:
            {
                var tn = (TryNode)node;
                // Only the catch-clause binding names are explicit locals.
                if (tn.ErrorType    is IdentifierNode et) out_.Add(et.Name);
                if (tn.ErrorMessage is IdentifierNode em) out_.Add(em.Name);
                break;
            }
            case ASTNodeType.Case:
            {
                var cn = (CaseNode)node;
                if (cn.TestValueAlias is IdentifierNode tva) out_.Add(tva.Name);
                break;
            }
            case ASTNodeType.CaseWhen:
                break;
            // Never recurse into nested function scopes
            case ASTNodeType.Function:
            case ASTNodeType.DecoratedFunction:
            case ASTNodeType.Lambda:
            case ASTNodeType.Package:
            case ASTNodeType.Struct:
                break;
        }
    }

    // -- Slot management -------------------------------------------------------

    private int DeclareLocal(string name)
    {
        int slot = _slotCount++;
        _locals.Add(new LocalVar { Name = name, Slot = slot });
        return slot;
    }

    private int ResolveLocal(string name)
    {
        for (int i = _locals.Count - 1; i >= 0; i--)
            if (_locals[i].Name == name) return _locals[i].Slot;
        return -1;
    }

    private int ResolveUpvalue(string name)
    {
        if (_enclosing == null) return -1;

        int localSlot = _enclosing.ResolveLocal(name);
        if (localSlot >= 0)
        {
            for (int i = 0; i < _enclosing._locals.Count; i++)
            {
                if (_enclosing._locals[i].Slot == localSlot)
                {
                    var lv = _enclosing._locals[i];
                    lv.Captured = true;
                    _enclosing._locals[i] = lv;
                    break;
                }
            }
            return AddUv(isLocal: true, index: localSlot);
        }

        int uvIdx = _enclosing.ResolveUpvalue(name);
        if (uvIdx >= 0) return AddUv(isLocal: false, index: uvIdx);
        return -1;
    }

    private int AddUv(bool isLocal, int index)
    {
        for (int i = 0; i < _upvalues.Count; i++)
            if (_upvalues[i].IsLocal == isLocal && _upvalues[i].Index == index) return i;
        _upvalues.Add(new UpvalueDescriptor { IsLocal = isLocal, Index = index });
        return _upvalues.Count - 1;
    }

    // -- Variable emit helpers -------------------------------------------------

    private void EmitLoad(string name, int line = 0)
    {
        if (!_isGlobal)
        {
            int s = ResolveLocal(name);
            if (s >= 0) { _chunk.Emit(Opcode.LoadLocal,   s,                   0, line); return; }
            int u = ResolveUpvalue(name);
            if (u >= 0) { _chunk.Emit(Opcode.LoadUpvalue, u,                   0, line); return; }
        }
        _chunk.Emit(Opcode.LoadGlobal, _chunk.AddName(name), 0, line);
    }

    private void EmitStore(string name, int line = 0)
    {
        if (!_isGlobal)
        {
            int s = ResolveLocal(name);
            if (s >= 0) { _chunk.Emit(Opcode.StoreLocal,   s,                   0, line); return; }
            int u = ResolveUpvalue(name);
            if (u >= 0) { _chunk.Emit(Opcode.StoreUpvalue, u,                   0, line); return; }
        }
        _chunk.Emit(Opcode.StoreGlobal, _chunk.AddName(name), 0, line);
    }

    // -- Jump helpers ----------------------------------------------------------

    private int EmitJump(Opcode op, int line = 0) => _chunk.Emit(op, 0, 0, line);

    private void PatchJump(int idx) => _chunk.PatchJump(idx, _chunk.Code.Count);

    private void PatchJumpTo(int idx, int tgt) => _chunk.PatchJump(idx, tgt);

    // -- Return ----------------------------------------------------------------

    private void EmitFinalReturn()
    {
        if (!_isGlobal && _locals.Any(l => l.Captured))
            _chunk.Emit(Opcode.CloseUpvalue, 0);
        _chunk.Emit(Opcode.ReturnNull);
    }

    // -- Compound-op helper ----------------------------------------------------

    private void EmitCompoundOp(TokenName op, int line)
    {
        switch (op)
        {
            case TokenName.Ops_AddAssign:              _chunk.Emit(Opcode.Add,   0, 0, line); break;
            case TokenName.Ops_SubtractAssign:         _chunk.Emit(Opcode.Sub,   0, 0, line); break;
            case TokenName.Ops_MultiplyAssign:         _chunk.Emit(Opcode.Mul,   0, 0, line); break;
            case TokenName.Ops_DivideAssign:           _chunk.Emit(Opcode.Div,   0, 0, line); break;
            case TokenName.Ops_ModuloAssign:           _chunk.Emit(Opcode.Mod,   0, 0, line); break;
            case TokenName.Ops_ExponentAssign:         _chunk.Emit(Opcode.Pow,   0, 0, line); break;
            case TokenName.Ops_BitwiseAndAssign:       _chunk.Emit(Opcode.BAnd,  0, 0, line); break;
            case TokenName.Ops_BitwiseOrAssign:        _chunk.Emit(Opcode.BOr,   0, 0, line); break;
            case TokenName.Ops_BitwiseXorAssign:       _chunk.Emit(Opcode.BXor,  0, 0, line); break;
            case TokenName.Ops_BitwiseLeftShiftAssign: _chunk.Emit(Opcode.BLSh,  0, 0, line); break;
            case TokenName.Ops_BitwiseRightShiftAssign: _chunk.Emit(Opcode.BRSh, 0, 0, line); break;
            case TokenName.Ops_BitwiseUnsignedRightShiftAssign: _chunk.Emit(Opcode.BURSh, 0, 0, line); break;
            default: /* leave as-is — callers handle && / || */ break;
        }
    }

    // -- Main compile dispatch -------------------------------------------------

    /// <summary>
    /// Compile a node in expression context.
    /// Returns true if a value was left on the stack, false otherwise.
    /// </summary>
    private bool CompileNode(ASTNode node)
    {
        int ln = node.Token.Span.Line;
        switch (node.Type)
        {
            case ASTNodeType.NoOp:             return false;
            case ASTNodeType.Literal:          CompileLiteral        ((LiteralNode)node,          ln); return true;
            case ASTNodeType.Identifier:       EmitLoad              (((IdentifierNode)node).Name, ln); return true;
            case ASTNodeType.Variable:         CompileVariable       ((VariableNode)node,          ln); return false;
            case ASTNodeType.Assignment:       return CompileAssignment((AssignmentNode)node,        ln);
            case ASTNodeType.ConstAssignment:  CompileConstAssign    ((ConstAssignmentNode)node,   ln); return false;
            case ASTNodeType.PackAssignment:   CompilePackAssign     ((PackAssignmentNode)node,    ln); return false;
            case ASTNodeType.BinaryOperation:  CompileBinary         ((BinaryOperationNode)node,   ln); return true;
            case ASTNodeType.UnaryOperation:   CompileUnary          ((UnaryOperationNode)node,    ln); return true;
            case ASTNodeType.TernaryOperation: CompileTernary        ((TernaryOperationNode)node,  ln); return true;
            case ASTNodeType.Interpolation:    CompileInterp         ((InterpolationNode)node,     ln); return true;
            case ASTNodeType.ListLiteral:      CompileList           ((ListLiteralNode)node,       ln); return true;
            case ASTNodeType.HashLiteral:      CompileHash           ((HashLiteralNode)node,       ln); return true;
            case ASTNodeType.RangeLiteral:     CompileRange          ((RangeLiteralNode)node,      ln); return true;
            case ASTNodeType.Index:            CompileIndex          ((IndexingNode)node,          ln); return true;
            case ASTNodeType.IndexAssignment:  CompileIndexAssign    ((IndexAssignmentNode)node,   ln); return true;
            case ASTNodeType.Slice:            CompileSlice          ((SliceNode)node,             ln); return true;
            case ASTNodeType.MemberAccess:     CompileMemberAccess   ((MemberAccessNode)node,      ln); return true;
            case ASTNodeType.MemberAssignment: CompileMemberAssign   ((MemberAssignmentNode)node,  ln); return true;
            case ASTNodeType.Self:             CompileSelf           ((SelfNode)node,              ln); return true;
            case ASTNodeType.StaticSelf:       CompileStaticSelf     ((StaticSelfNode)node,        ln); return true;
            case ASTNodeType.Print:            CompilePrint          ((PrintNode)node,             ln); return false;
            case ASTNodeType.If:               CompileIf             ((IfNode)node,                ln); return true;
            case ASTNodeType.Case:             CompileCase           ((CaseNode)node,              ln); return true;
            case ASTNodeType.CaseWhen:         return false; // handled inside CompileCase
            case ASTNodeType.WhileLoop:        CompileWhile          ((WhileLoopNode)node,         ln); return false;
            case ASTNodeType.RepeatLoop:       CompileRepeat         ((RepeatLoopNode)node,        ln); return false;
            case ASTNodeType.ForLoop:          CompileForLoop        ((ForLoopNode)node,           ln); return false;
            case ASTNodeType.Break:            CompileBreak          ((BreakNode)node,             ln); return false;
            case ASTNodeType.Next:             CompileNext           ((NextNode)node,              ln); return false;
            case ASTNodeType.Return:           CompileReturn         ((ReturnNode)node,            ln); return false;
            case ASTNodeType.Yield:            CompileYield          ((YieldNode)node,             ln); return false;
            case ASTNodeType.Throw:            CompileThrow          ((ThrowNode)node,             ln); return false;
            case ASTNodeType.Try:              CompileTry            ((TryNode)node,               ln); return false;
            case ASTNodeType.Function:         CompileFunction       ((FunctionNode)node,          ln); return false;
            case ASTNodeType.DecoratedFunction: Fallback             (node);                            return true;
            case ASTNodeType.Lambda:           CompileLambdaDef      ((LambdaNode)node,            ln); return true;
            case ASTNodeType.LambdaCall:       CompileLambdaCall     ((LambdaCallNode)node,        ln); return true;
            case ASTNodeType.FunctionCall:     CompileFuncCall       ((FunctionCallNode)node,      ln); return true;
            case ASTNodeType.MethodCall:       CompileMethodCall     ((MethodCallNode)node,        ln); return true;
            case ASTNodeType.Program:
                foreach (var s in ((ProgramNode)node).Statements)
                    if (s != null) CompileStatement(s);
                return false;
            default:
                Fallback(node); return true;
        }
    }

    /// <summary>
    /// Compile a node in statement context: pop its result if it left one on the stack.
    /// </summary>
    private void CompileStatement(ASTNode node)
    {
        if (CompileNode(node))
            _chunk.Emit(Opcode.Pop);
    }

    // -- Fallback to interpreter -----------------------------------------------

    private void Fallback(ASTNode node)
    {
        int idx = _chunk.AddNodeFallback(node);
        _chunk.Emit(Opcode.InterpFallback, idx, 0, node.Token.Span.Line);
    }

    // -- Literal ---------------------------------------------------------------

    private void CompileLiteral(LiteralNode node, int ln)
    {
        var v = node.Value;
        if      (v.IsNull())    _chunk.Emit(Opcode.Null,  0, 0, ln);
        else if (v.IsBoolean()) _chunk.Emit(v.GetBoolean() ? Opcode.True : Opcode.False, 0, 0, ln);
        else                   _chunk.Emit(Opcode.Const, _chunk.AddConstant(v), 0, ln);
    }

    // -- Variable declaration --------------------------------------------------

    private void CompileVariable(VariableNode node, int ln)
    {
        foreach (var (name, init) in node.Variables)
        {
            if (init != null)
            {
                CompileNode(init);
                EmitStore(name, ln);
            }
            else if (node.TypeHints.TryGetValue(name, out var hints) && hints.Count > 0
                     && TypeRegistry.IsPrimitive(hints[0]))
            {
                // Emit a type-appropriate zero/default so that compound ops like += work.
                switch (TypeRegistry.GetValueType(hints[0]))
                {
                    case Typing.ValueType.Boolean: _chunk.Emit(Opcode.False,  0, 0, ln); EmitStore(name, ln); break;
                    case Typing.ValueType.Integer: _chunk.Emit(Opcode.Const,  _chunk.AddConstant(Value.CreateInteger(0)), 0, ln); EmitStore(name, ln); break;
                    case Typing.ValueType.Float:   _chunk.Emit(Opcode.Const,  _chunk.AddConstant(Value.CreateFloat(0.0)), 0, ln); EmitStore(name, ln); break;
                    case Typing.ValueType.String:  _chunk.Emit(Opcode.Const,  _chunk.AddConstant(Value.CreateString("")), 0, ln); EmitStore(name, ln); break;
                    case Typing.ValueType.List:    _chunk.Emit(Opcode.BuildList, 0, 0, ln); EmitStore(name, ln); break;
                    case Typing.ValueType.Hashmap: _chunk.Emit(Opcode.BuildHashmap, 0, 0, ln); EmitStore(name, ln); break;
                }
            }
        }
    }

    // -- Assignment ------------------------------------------------------------

    /// <summary>
    /// Returns true if a value is left on the stack (expression context).
    /// Conditional (when-guard) assignments never leave a value — the condition may be
    /// false, making the stack state after the skip-jump unpredictable.
    /// </summary>
    private bool CompileAssignment(AssignmentNode node, int ln)
    {
        if (node.Condition != null)
        {
            CompileNode(node.Condition);
            int skip = EmitJump(Opcode.JumpF, ln);
            DoAssignment(node, ln, leaveValue: false);
            PatchJump(skip);
            return false;
        }

        return DoAssignment(node, ln, leaveValue: true);
    }

    /// <summary>
    /// Emits code that evaluates and stores an assignment.
    /// When <paramref name="leaveValue"/> is true, leaves the assigned value on the stack
    /// for use as an expression (e.g. implicit return from a lambda body).
    /// StoreSelfAttr / StoreStaticAttr already push the value back in the VM, so they
    /// always leave a value regardless of <paramref name="leaveValue"/>.
    /// </summary>
    private bool DoAssignment(AssignmentNode node, int ln, bool leaveValue = false)
    {
        // Simple identifier LHS is when Left is null or is an IdentifierNode.
        bool isSimple = node.Left == null || node.Left is IdentifierNode;
        if (!isSimple)
        {
            // @.name = val  or  @.name op= val  → StoreSelfAttr
            if (node.Left is SelfNode)
            {
                int ni = _chunk.AddName(node.Name);
                if (node.Op != TokenName.Ops_Assign)
                {
                    _chunk.Emit(Opcode.LoadSelfAttr, ni, 0, ln);
                    if (node.Initializer != null) CompileNode(node.Initializer);
                    EmitCompoundOp(node.Op, ln);
                }
                else
                {
                    if (node.Initializer != null) CompileNode(node.Initializer);
                    else _chunk.Emit(Opcode.Null, 0, 0, ln);
                }
                if (leaveValue) _chunk.Emit(Opcode.Dup, 0, 0, ln);
                _chunk.Emit(Opcode.StoreSelfAttr, ni, 0, ln);
                return leaveValue;
            }
            // @@name = val  or  @@name op= val  → StoreStaticAttr
            if (node.Left is StaticSelfNode)
            {
                int ni = _chunk.AddName(node.Name);
                if (node.Op != TokenName.Ops_Assign)
                {
                    _chunk.Emit(Opcode.LoadStaticAttr, ni, 0, ln);
                    if (node.Initializer != null) CompileNode(node.Initializer);
                    EmitCompoundOp(node.Op, ln);
                }
                else
                {
                    if (node.Initializer != null) CompileNode(node.Initializer);
                    else _chunk.Emit(Opcode.Null, 0, 0, ln);
                }
                if (leaveValue) _chunk.Emit(Opcode.Dup, 0, 0, ln);
                _chunk.Emit(Opcode.StoreStaticAttr, ni, 0, ln);
                return leaveValue;
            }
            // Other complex LHS → fallback
            Fallback(node); return false;
        }

        bool compound = node.Op != TokenName.Ops_Assign;
        if (compound)
        {
            EmitLoad(node.Name, ln);
            if (node.Initializer != null) CompileNode(node.Initializer);
            EmitCompoundOp(node.Op, ln);
        }
        else
        {
            if (node.Initializer != null) CompileNode(node.Initializer);
            else                         _chunk.Emit(Opcode.Null, 0, 0, ln);
        }
        if (leaveValue) _chunk.Emit(Opcode.Dup, 0, 0, ln);
        EmitStore(node.Name, ln);
        return leaveValue;
    }

    // -- Const assignment ------------------------------------------------------

    private void CompileConstAssign(ConstAssignmentNode node, int ln)
    {
        if (node.Initializer != null) CompileNode(node.Initializer);
        else                         _chunk.Emit(Opcode.Null, 0, 0, ln);
        _chunk.Emit(Opcode.StoreGlobal, _chunk.AddName(node.Name), 0, ln);
    }

    // -- Binary ----------------------------------------------------------------

    private void CompileBinary(BinaryOperationNode node, int ln)
    {
        switch (node.Op)
        {
            case TokenName.Ops_And:
            case TokenName.Ops_AndAssign:
                // JumpAnd: falsy → jump to skip (left stays on stack as result);
                //          truthy → pop left, fall through to right.
                CompileNode(node.Left!);
                int andSkip = EmitJump(Opcode.JumpAnd, ln);
                CompileNode(node.Right!);
                PatchJump(andSkip);
                return;

            case TokenName.Ops_Or:
            case TokenName.Ops_OrAssign:
                // JumpOr: truthy → jump to skip (left stays on stack as result);
                //         falsy → pop left, fall through to right.
                CompileNode(node.Left!);
                int orSkip = EmitJump(Opcode.JumpOr, ln);
                CompileNode(node.Right!);
                PatchJump(orSkip);
                return;

            case TokenName.Ops_NullCoalesce:
                CompileNode(node.Left!);
                CompileNode(node.Right!);
                _chunk.Emit(Opcode.NullCoalesce, 0, 0, ln);
                return;
        }

        CompileNode(node.Left!);
        CompileNode(node.Right!);

        Opcode op = node.Op switch
        {
            TokenName.Ops_Add              or TokenName.Ops_AddAssign              => Opcode.Add,
            TokenName.Ops_Subtract         or TokenName.Ops_SubtractAssign         => Opcode.Sub,
            TokenName.Ops_Multiply         or TokenName.Ops_MultiplyAssign         => Opcode.Mul,
            TokenName.Ops_Divide           or TokenName.Ops_DivideAssign           => Opcode.Div,
            TokenName.Ops_Modulus          or TokenName.Ops_ModuloAssign           => Opcode.Mod,
            TokenName.Ops_Exponent         or TokenName.Ops_ExponentAssign         => Opcode.Pow,
            TokenName.Ops_BitwiseAnd       or TokenName.Ops_BitwiseAndAssign       => Opcode.BAnd,
            TokenName.Ops_BitwiseOr        or TokenName.Ops_BitwiseOrAssign        => Opcode.BOr,
            TokenName.Ops_BitwiseXor       or TokenName.Ops_BitwiseXorAssign       => Opcode.BXor,
            TokenName.Ops_BitwiseLeftShift or TokenName.Ops_BitwiseLeftShiftAssign => Opcode.BLSh,
            TokenName.Ops_BitwiseRightShift or TokenName.Ops_BitwiseRightShiftAssign => Opcode.BRSh,
            TokenName.Ops_BitwiseUnsignedRightShift or TokenName.Ops_BitwiseUnsignedRightShiftAssign => Opcode.BURSh,
            TokenName.Ops_Equal            => Opcode.Eq,
            TokenName.Ops_NotEqual         => Opcode.NEq,
            TokenName.Ops_LessThan         => Opcode.Lt,
            TokenName.Ops_LessThanOrEqual  => Opcode.LtE,
            TokenName.Ops_GreaterThan      => Opcode.Gt,
            TokenName.Ops_GreaterThanOrEqual => Opcode.GtE,
            TokenName.KW_In                => Opcode.In,
            _                              => (Opcode)255,
        };

        if (op == (Opcode)255)
        {
            // Unknown binary op — reconstruct the node and fallback
            // (stack already has left and right on it — pop them first)
            _chunk.Emit(Opcode.Pop, 0, 0, ln);
            _chunk.Emit(Opcode.Pop, 0, 0, ln);
            Fallback(node);
        }
        else
        {
            _chunk.Emit(op, 0, 0, ln);
        }
    }

    // -- Unary -----------------------------------------------------------------

    private void CompileUnary(UnaryOperationNode node, int ln)
    {
        CompileNode(node.Operand!);
        Opcode op = node.Op switch
        {
            TokenName.Ops_Not                                        => Opcode.Not,
            TokenName.Ops_Subtract                                   => Opcode.Neg,
            TokenName.Ops_BitwiseNot or TokenName.Ops_BitwiseNotAssign => Opcode.BNot,
            _ => (Opcode)255
        };
        if (op == (Opcode)255) { _chunk.Emit(Opcode.Pop); Fallback(node); }
        else                    _chunk.Emit(op, 0, 0, ln);
    }

    // -- Ternary ---------------------------------------------------------------

    private void CompileTernary(TernaryOperationNode node, int ln)
    {
        CompileNode(node.EvalExpression!);
        int fj = EmitJump(Opcode.JumpF, ln);
        CompileNode(node.TrueExpression!);
        int ej = EmitJump(Opcode.Jump, ln);
        PatchJump(fj);
        CompileNode(node.FalseExpression!);
        PatchJump(ej);
    }

    // -- Interpolation ---------------------------------------------------------

    private void CompileInterp(InterpolationNode node, int ln)
    {
        foreach (var p in node.Parts) CompileNode(p);
        _chunk.Emit(Opcode.Interpolate, node.Parts.Count, 0, ln);
    }

    // -- Collections -----------------------------------------------------------

    private void CompileList(ListLiteralNode node, int ln)
    {
        foreach (var e in node.Elements) CompileNode(e);
        _chunk.Emit(Opcode.BuildList, node.Elements.Count, 0, ln);
    }

    private void CompileHash(HashLiteralNode node, int ln)
    {
        foreach (var (k, v) in node.Elements)
        {
            CompileNode(k);
            if (v != null) CompileNode(v);
            else           _chunk.Emit(Opcode.Null, 0, 0, ln);
        }
        _chunk.Emit(Opcode.BuildHashmap, node.Elements.Count, 0, ln);
    }

    private void CompileRange(RangeLiteralNode node, int ln)
    {
        CompileNode(node.RangeStart!);
        CompileNode(node.RangeEnd!);
        _chunk.Emit(Opcode.BuildRange, 0, 0, ln);
    }

    // -- Indexing --------------------------------------------------------------

    private void CompileIndex(IndexingNode node, int ln)
    {
        if (node.IndexedObject != null) CompileNode(node.IndexedObject);
        CompileNode(node.IndexExpression);
        _chunk.Emit(Opcode.IndexGet, 0, 0, ln);
    }

    // -- Case / when -----------------------------------------------------------

    private void CompileCase(CaseNode node, int ln)
    {
        bool isSwitch = node.TestValue != null;
        var endJumps = new List<int>();

        if (isSwitch)
        {
            // Switch-style: evaluate test value and keep it on stack throughout
            CompileNode(node.TestValue!);

            // Store alias if present (Dup so testVal stays on stack)
            if (node.TestValueAlias is IdentifierNode ali)
            {
                _chunk.Emit(Opcode.Dup, 0, 0, ln);
                EmitStore(ali.Name, ln);
            }

            foreach (var when in node.WhenNodes)
            {
                // testVal is at top of stack.  For each condition, Dup then compare.
                var nextWhenJumps = new List<int>();
                var bodyJumps     = new List<int>();

                foreach (var condNode in when.Conditions)
                {
                    if (condNode == null) continue;

                    if (condNode is RangeLiteralNode rng)
                    {
                        // Range check: testVal >= start AND testVal <= end
                        // Stack before: [testVal]
                        _chunk.Emit(Opcode.Dup, 0, 0, ln);    // [testVal, testVal]
                        CompileNode(rng.RangeStart!);          // [testVal, testVal, start]
                        _chunk.Emit(Opcode.GtE, 0, 0, ln);    // [testVal, tv>=start]
                        int failLow = _chunk.Emit(Opcode.JumpF, 0, 0, ln); // skip if out of range
                        _chunk.Emit(Opcode.Dup, 0, 0, ln);    // [testVal, testVal]
                        CompileNode(rng.RangeEnd!);            // [testVal, testVal, end]
                        _chunk.Emit(Opcode.LtE, 0, 0, ln);    // [testVal, tv<=end]
                        bodyJumps.Add(_chunk.Emit(Opcode.JumpT, 0, 0, ln)); // match → body
                        PatchJump(failLow);
                    }
                    else
                    {
                        _chunk.Emit(Opcode.Dup, 0, 0, ln);    // [testVal, testVal]
                        CompileNode(condNode);                 // [testVal, testVal, cond]
                        _chunk.Emit(Opcode.Eq, 0, 0, ln);     // [testVal, matched?]
                        bodyJumps.Add(_chunk.Emit(Opcode.JumpT, 0, 0, ln));
                    }
                }

                // No condition matched → skip to next when
                nextWhenJumps.Add(_chunk.Emit(Opcode.Jump, 0, 0, ln));

                // Body target: pop testVal and run body as expression
                int bodyTarget = _chunk.Code.Count;
                foreach (var j in bodyJumps) PatchJumpTo(j, bodyTarget);
                _chunk.Emit(Opcode.Pop, 0, 0, ln); // discard testVal
                CompileBodyExpr(when.Body);         // leaves value on stack
                endJumps.Add(_chunk.Emit(Opcode.Jump, 0, 0, ln));

                // Patch "no match" jumps to here
                int nextTarget = _chunk.Code.Count;
                foreach (var j in nextWhenJumps) PatchJumpTo(j, nextTarget);
            }

            // Else: pop testVal then run else body as expression
            _chunk.Emit(Opcode.Pop, 0, 0, ln);
        }
        else
        {
            // Guard-style: each condition is evaluated as truthy/falsy
            foreach (var when in node.WhenNodes)
            {
                var bodyJumps    = new List<int>();
                var nextWhenJump = new List<int>();

                foreach (var condNode in when.Conditions)
                {
                    if (condNode == null) continue;
                    CompileNode(condNode);
                    bodyJumps.Add(_chunk.Emit(Opcode.JumpT, 0, 0, ln));
                }

                nextWhenJump.Add(_chunk.Emit(Opcode.Jump, 0, 0, ln));

                int bodyTarget = _chunk.Code.Count;
                foreach (var j in bodyJumps) PatchJumpTo(j, bodyTarget);
                CompileBodyExpr(when.Body);         // leaves value on stack
                endJumps.Add(_chunk.Emit(Opcode.Jump, 0, 0, ln));

                int nextTarget = _chunk.Code.Count;
                foreach (var j in nextWhenJump) PatchJumpTo(j, nextTarget);
            }
        }

        // Else body (or null if absent)
        CompileBodyExpr(node.ElseBody);

        int end = _chunk.Code.Count;
        foreach (var j in endJumps) PatchJumpTo(j, end);
    }

    private void CompilePackAssign(PackAssignmentNode node, int ln)
    {
        // All LHS elements must be simple identifiers
        foreach (var lhs in node.Left)
            if (lhs != null && lhs is not IdentifierNode)
            { Fallback(node); return; }

        int lhsCount = node.Left.Count;
        int rhsCount = node.Right.Count(r => r != null);

        if (rhsCount == 1 && lhsCount > 1)
        {
            // Single RHS: evaluate it, then unpack (runtime list detection)
            var rhs = node.Right.First(r => r != null)!;
            CompileNode(rhs);
            _chunk.Emit(Opcode.UnpackList, lhsCount, 0, ln);
            // Stack now has lhsCount values (bottom=index0, top=indexN-1)
            // Store in reverse order (pop from top = index N-1 first)
            for (int i = lhsCount - 1; i >= 0; i--)
            {
                var lhs = node.Left[i];
                if (lhs is IdentifierNode id) EmitStore(id.Name, ln);
                else _chunk.Emit(Opcode.Pop, 0, 0, ln);
            }
        }
        else
        {
            // Multiple RHS (or 1:1 mapping): evaluate all RHS first, then store in reverse
            int n = Math.Min(lhsCount, rhsCount);
            foreach (var rhs in node.Right.Where(r => r != null).Take(lhsCount))
                CompileNode(rhs!);
            // Push nulls for any extra LHS
            for (int i = rhsCount; i < lhsCount; i++)
                _chunk.Emit(Opcode.Null, 0, 0, ln);
            // Store in reverse (top of stack = last RHS)
            for (int i = lhsCount - 1; i >= 0; i--)
            {
                var lhs = node.Left[i];
                if (lhs is IdentifierNode id) EmitStore(id.Name, ln);
                else _chunk.Emit(Opcode.Pop, 0, 0, ln);
            }
        }
    }

    private void CompileIndexAssign(IndexAssignmentNode node, int ln)
    {
        var idx = (IndexingNode)node.Object!;

        if (node.Op == TokenName.Ops_Assign)
        {
            // Simple assignment: push obj, key, val → IndexSet
            if (idx.IndexedObject != null) CompileNode(idx.IndexedObject);
            else                           EmitLoad(idx.Name ?? "", ln);
            CompileNode(idx.IndexExpression);
            if (node.Initializer != null) CompileNode(node.Initializer);
            else                         _chunk.Emit(Opcode.Null, 0, 0, ln);
            _chunk.Emit(Opcode.IndexSet, 0, 0, ln);
        }
        else
        {
            // Compound assignment (lst[i] += v): push obj, key, rhs → IndexOpAssign(inner_op)
            Opcode innerOp = node.Op switch
            {
                TokenName.Ops_AddAssign              => Opcode.Add,
                TokenName.Ops_SubtractAssign         => Opcode.Sub,
                TokenName.Ops_MultiplyAssign         => Opcode.Mul,
                TokenName.Ops_DivideAssign           => Opcode.Div,
                TokenName.Ops_ModuloAssign           => Opcode.Mod,
                TokenName.Ops_ExponentAssign         => Opcode.Pow,
                TokenName.Ops_BitwiseAndAssign       => Opcode.BAnd,
                TokenName.Ops_BitwiseOrAssign        => Opcode.BOr,
                TokenName.Ops_BitwiseXorAssign       => Opcode.BXor,
                TokenName.Ops_BitwiseLeftShiftAssign => Opcode.BLSh,
                TokenName.Ops_BitwiseRightShiftAssign=> Opcode.BRSh,
                TokenName.Ops_BitwiseUnsignedRightShiftAssign => Opcode.BURSh,
                _ => (Opcode)255
            };

            if (innerOp == (Opcode)255) { Fallback(node); return; }

            if (idx.IndexedObject != null) CompileNode(idx.IndexedObject);
            else                           EmitLoad(idx.Name ?? "", ln);
            CompileNode(idx.IndexExpression);
            if (node.Initializer != null) CompileNode(node.Initializer);
            else                         _chunk.Emit(Opcode.Null, 0, 0, ln);
            _chunk.Emit(Opcode.IndexOpAssign, (int)innerOp, 0, ln);
        }
    }

    private void CompileSlice(SliceNode node, int ln)
    {
        if (node.SlicedObject != null) CompileNode(node.SlicedObject);
        int flags = 0;
        if (node.StartExpression != null) { CompileNode(node.StartExpression); flags |= 1; }
        if (node.StopExpression  != null) { CompileNode(node.StopExpression);  flags |= 2; }
        if (node.StepExpression  != null) { CompileNode(node.StepExpression);  flags |= 4; }
        _chunk.Emit(Opcode.SliceGet, flags, 0, ln);
    }

    // -- Member access ---------------------------------------------------------

    private void CompileMemberAccess(MemberAccessNode node, int ln)
    {
        CompileNode(node.Object!);
        _chunk.Emit(Opcode.GetMember, _chunk.AddName(node.MemberName), 0, ln);
    }

    private void CompileMemberAssign(MemberAssignmentNode node, int ln)
    {
        bool compound = node.Op != TokenName.Ops_Assign;
        if (compound)
        {
            CompileNode(node.Object!);
            _chunk.Emit(Opcode.GetMember, _chunk.AddName(node.MemberName), 0, ln);
            if (node.Initializer != null) CompileNode(node.Initializer);
            EmitCompoundOp(node.Op, ln);
        }
        else
        {
            if (node.Initializer != null) CompileNode(node.Initializer);
            else                         _chunk.Emit(Opcode.Null, 0, 0, ln);
        }
        CompileNode(node.Object!);
        _chunk.Emit(Opcode.SetMember, _chunk.AddName(node.MemberName), 0, ln);
    }

    // -- Self / StaticSelf -----------------------------------------------------

    private void CompileSelf(SelfNode node, int ln)
    {
        if (string.IsNullOrEmpty(node.Name)) _chunk.Emit(Opcode.LoadSelf,     0, 0, ln);
        else                                 _chunk.Emit(Opcode.LoadSelfAttr, _chunk.AddName(node.Name), 0, ln);
    }

    private void CompileStaticSelf(StaticSelfNode node, int ln)
        => _chunk.Emit(Opcode.LoadStaticAttr, _chunk.AddName(node.Name), 0, ln);

    // -- Print -----------------------------------------------------------------

    private void CompilePrint(PrintNode node, int ln)
    {
        if (node.Expression != null) CompileNode(node.Expression);
        else                        _chunk.Emit(Opcode.Null, 0, 0, ln);
        int flags = (node.PrintNewline ? 1 : 0) | (node.PrintStdError ? 2 : 0);
        _chunk.Emit(Opcode.Print, flags, 0, ln);
    }

    // -- If --------------------------------------------------------------------

    private void CompileIf(IfNode node, int ln)
    {
        var endJumps = new List<int>();

        // Main branch
        CompileNode(node.Condition!);
        int nextBranch = EmitJump(Opcode.JumpF, ln);
        CompileBodyExpr(node.Body);
        endJumps.Add(EmitJump(Opcode.Jump, ln));
        PatchJump(nextBranch);

        // Elsif branches
        foreach (var elsif in node.ElsifNodes)
        {
            if (elsif == null) continue;
            CompileNode(elsif.Condition!);
            nextBranch = EmitJump(Opcode.JumpF, ln);
            CompileBodyExpr(elsif.Body);
            endJumps.Add(EmitJump(Opcode.Jump, ln));
            PatchJump(nextBranch);
        }

        // Else (or null if absent)
        CompileBodyExpr(node.ElseBody);

        int end = _chunk.Code.Count;
        foreach (var j in endJumps) PatchJumpTo(j, end);
    }

    // -- While -----------------------------------------------------------------

    private void CompileWhile(WhileLoopNode node, int ln)
    {
        int loopTop = _chunk.Code.Count;

        CompileNode(node.Condition!);
        int exitJump = EmitJump(Opcode.JumpF, ln);

        var ctx = new LoopCtx { LoopTop = loopTop, ForIterSlot = -1 };
        _loops.Add(ctx);
        CompileBody(node.Body);

        _chunk.Emit(Opcode.Jump, loopTop, 0, ln);

        PatchJump(exitJump);
        int afterLoop = _chunk.Code.Count;

        foreach (var j in ctx.BreakPatches) PatchJumpTo(j, afterLoop);
        foreach (var j in ctx.NextPatches)  PatchJumpTo(j, loopTop);
        _loops.RemoveAt(_loops.Count - 1);
    }

    // -- Repeat ----------------------------------------------------------------

    private void CompileRepeat(RepeatLoopNode node, int ln)
    {
        // Allocate two hidden local slots that live beyond the pre-scanned locals.
        // We must update LocalCount so the VM frame reserves enough stack space.
        int limitSlot   = _slotCount++;
        int counterSlot = _slotCount++;
        _chunk.LocalCount = _slotCount;

        // Optional alias name (pre-scanned by ScanNode; use EmitStore to handle global scope too).
        string? aliasName = node.Alias is IdentifierNode ali ? ali.Name : null;

        // limit = evaluate count expression
        CompileNode(node.Count!);
        _chunk.Emit(Opcode.StoreLocal, limitSlot, 0, ln);

        // counter = 1
        int one = _chunk.AddConstant(Value.CreateInteger(1));
        _chunk.Emit(Opcode.Const,       one, 0, ln);
        _chunk.Emit(Opcode.StoreLocal,  counterSlot, 0, ln);

        // LOOP TOP: if counter > limit → exit
        int loopTop  = _chunk.Code.Count;
        _chunk.Emit(Opcode.LoadLocal, counterSlot, 0, ln);
        _chunk.Emit(Opcode.LoadLocal, limitSlot,   0, ln);
        _chunk.Emit(Opcode.Gt, 0, 0, ln);
        int doneJump = EmitJump(Opcode.JumpT, ln);

        // if alias: store counter into the alias variable
        if (aliasName != null)
        {
            _chunk.Emit(Opcode.LoadLocal, counterSlot, 0, ln);
            EmitStore(aliasName, ln);
        }

        // Body
        var ctx = new LoopCtx { LoopTop = loopTop, ForIterSlot = -1 };
        _loops.Add(ctx);
        CompileBody(node.Body);

        // INCREMENT POINT — next patches jump here
        int incTop = _chunk.Code.Count;
        _chunk.Emit(Opcode.LoadLocal, counterSlot, 0, ln);
        _chunk.Emit(Opcode.Const,     one, 0, ln);
        _chunk.Emit(Opcode.Add, 0, 0, ln);
        _chunk.Emit(Opcode.StoreLocal, counterSlot, 0, ln);
        _chunk.Emit(Opcode.Jump, loopTop, 0, ln);

        int done = _chunk.Code.Count;
        PatchJump(doneJump);

foreach (var j in ctx.BreakPatches) PatchJumpTo(j, done);
        foreach (var j in ctx.NextPatches)  PatchJumpTo(j, incTop);
        _loops.RemoveAt(_loops.Count - 1);
    }

    // -- For loop --------------------------------------------------------------

    private void CompileForLoop(ForLoopNode node, int ln)
    {
        string valName = (node.ValueIterator is IdentifierNode vi) ? vi.Name : "";
        string idxName = (node.IndexIterator is IdentifierNode ii) ? ii.Name : "";
        bool   hasDual = !string.IsNullOrEmpty(idxName);

        // Evaluate collection, push ForIterState
        CompileNode(node.DataSet!);
        _chunk.Emit(Opcode.ForIterInit, 0, 0, ln);

        // LOOP_TOP
        int loopTop  = _chunk.Code.Count;
        int numVars  = hasDual ? 2 : 1;
        int doneJump = _chunk.Emit(Opcode.ForIterNext, 0 /*patched*/, numVars, ln);

        // Store iterator values into variables
        if (hasDual)
        {
            // After ForIterNext (dual): stack has [..., state, key, val]
            EmitStore(valName, ln); // pop val
            EmitStore(idxName, ln); // pop key
        }
        else
        {
            EmitStore(valName, ln); // pop val
        }

        // Body
        var ctx = new LoopCtx { LoopTop = loopTop, ForIterSlot = 1 };
        _loops.Add(ctx);
        CompileBody(node.Body);

        _chunk.Emit(Opcode.Jump, loopTop, 0, ln);

        // DONE (ForIterNext pops state and jumps here)
        int done = _chunk.Code.Count;
        PatchJumpTo(doneJump, done);

        // Breaks jump here (ForIterState already cleaned up by ForIterNext or by break emit)
        foreach (var j in ctx.BreakPatches) PatchJumpTo(j, done);
        // Next jumps back to loopTop (ForIterNext)
        foreach (var j in ctx.NextPatches)  PatchJumpTo(j, loopTop);
        _loops.RemoveAt(_loops.Count - 1);
    }

    // -- Break / Next ----------------------------------------------------------

    private void CompileBreak(BreakNode node, int ln)
    {
        if (node.Condition != null)
        {
            CompileNode(node.Condition);
            int skip = EmitJump(Opcode.JumpF, ln);
            EmitBreak(ln);
            PatchJump(skip);
        }
        else EmitBreak(ln);
    }

    private void EmitBreak(int ln)
    {
        if (_loops.Count == 0) return;
        var ctx = _loops[^1];
        if (ctx.ForIterSlot >= 0) _chunk.Emit(Opcode.Pop, 0, 0, ln); // pop ForIterState
        ctx.BreakPatches.Add(EmitJump(Opcode.Jump, ln));
    }

    private void CompileNext(NextNode node, int ln)
    {
        if (node.Condition != null)
        {
            CompileNode(node.Condition);
            int skip = EmitJump(Opcode.JumpF, ln);
            EmitNext(ln);
            PatchJump(skip);
        }
        else EmitNext(ln);
    }

    private void EmitNext(int ln)
    {
        if (_loops.Count == 0) return;
        _loops[^1].NextPatches.Add(EmitJump(Opcode.Jump, ln));
    }

    // -- Return ----------------------------------------------------------------

    private void CompileReturn(ReturnNode node, int ln)
    {
        if (node.Condition != null)
        {
            CompileNode(node.Condition);
            int skip = EmitJump(Opcode.JumpF, ln);
            DoReturn(node, ln);
            PatchJump(skip);
        }
        else DoReturn(node, ln);
    }

    private void DoReturn(ReturnNode node, int ln)
    {
        if (!_isGlobal && _locals.Any(l => l.Captured))
            _chunk.Emit(Opcode.CloseUpvalue, 0, 0, ln);

        if (node.ReturnValue != null)
        {
            CompileNode(node.ReturnValue);
            _chunk.Emit(Opcode.Return, 0, 0, ln);
        }
        else _chunk.Emit(Opcode.ReturnNull, 0, 0, ln);
    }

    // -- Yield -----------------------------------------------------------------

    private void CompileYield(YieldNode node, int ln)
    {
        if (node.YieldValue != null) CompileNode(node.YieldValue);
        else                        _chunk.Emit(Opcode.Null, 0, 0, ln);
        _chunk.Emit(Opcode.Yield, 0, 0, ln);
    }

    // -- Throw -----------------------------------------------------------------

    private void CompileThrow(ThrowNode node, int ln)
    {
        if (node.Condition != null)
        {
            CompileNode(node.Condition);
            int skip = EmitJump(Opcode.JumpF, ln);
            if (node.ErrorValue != null) CompileNode(node.ErrorValue);
            else                        _chunk.Emit(Opcode.Null, 0, 0, ln);
            _chunk.Emit(Opcode.Throw, 0, 0, ln);
            PatchJump(skip);
        }
        else
        {
            if (node.ErrorValue != null) CompileNode(node.ErrorValue);
            else                        _chunk.Emit(Opcode.Null, 0, 0, ln);
            _chunk.Emit(Opcode.Throw, 0, 0, ln);
        }
    }

    // -- Try/catch/finally -----------------------------------------------------

    private void CompileTry(TryNode node, int ln)
    {
        bool hasCatch   = node.CatchBody.Count  > 0;
        bool hasFinally = node.FinallyBody.Count > 0;

        // Emit PushTryHandler with placeholder catchIP (A); finallyIP unused (B=0).
        int handlerInstr = EmitJump(Opcode.PushTryHandler, ln); // A=0 placeholder

        // --- Try body ---
        foreach (var s in node.TryBody)
            if (s != null) CompileStatement(s);

        // Try body completed normally: disarm the handler.
        _chunk.Emit(Opcode.PopTryHandler, 0, 0, ln);

        // Inline finally for the success path.
        if (hasFinally)
            foreach (var s in node.FinallyBody)
                if (s != null) CompileStatement(s);

        // Jump past the catch section (patched after catch is emitted).
        int jumpPastCatch = EmitJump(Opcode.Jump, ln);

        // --- Catch section ---
        int catchStart = _chunk.Code.Count;
        // Patch PushTryHandler.A to point here.
        _chunk.PatchJump(handlerInstr, catchStart);

        if (hasCatch)
        {
            // Load error variable(s) from _caughtError.
            if (node.ErrorType != null)
            {
                // Two-param catch(type, msg): push type string.
                _chunk.Emit(Opcode.LoadCatchError, 1, 0, ln);
                EmitStore(((IdentifierNode)node.ErrorType).Name, ln);
            }
            if (node.ErrorMessage != null)
            {
                if (node.ErrorType == null)
                    // Single-param catch(err): push {error, message} hashmap.
                    _chunk.Emit(Opcode.LoadCatchError, 0, 0, ln);
                else
                    // Two-param catch(type, msg): push message string.
                    _chunk.Emit(Opcode.LoadCatchError, 2, 0, ln);
                EmitStore(((IdentifierNode)node.ErrorMessage).Name, ln);
            }

            // Catch body.
            foreach (var s in node.CatchBody)
                if (s != null) CompileStatement(s);
        }

        // Inline finally for the catch/error path.
        if (hasFinally)
            foreach (var s in node.FinallyBody)
                if (s != null) CompileStatement(s);

        // Patch the jump that skips the catch section.
        PatchJump(jumpPastCatch);
    }

    // -- Function definition ---------------------------------------------------

    private void CompileFunction(FunctionNode node, int ln)
    {
        var sub = CompileFunction(node, enclosing: this);
        int si  = _chunk.AddSubChunk(sub);
        int ni  = _chunk.AddName(node.Name);
        _chunk.Emit(Opcode.DefFunc, si, ni, ln);
    }

    // -- Lambda ----------------------------------------------------------------

    private void CompileLambdaDef(LambdaNode node, int ln)
    {
        var sub  = CompileLambda(node, enclosing: this);
        int si   = _chunk.AddSubChunk(sub);
        _chunk.Emit(Opcode.MakeClosure, si, sub.Upvalues.Count, ln);
    }

    // -- Lambda call -----------------------------------------------------------

    private void CompileLambdaCall(LambdaCallNode node, int ln)
    {
        if (HasSplatArg(node.Arguments)) { Fallback(node); return; }
        CompileNode(node.LambdaNode!);
        int argc = CompileArgs(node.Arguments);
        _chunk.Emit(Opcode.Call, argc, 0, ln);
    }

    // -- Function call ---------------------------------------------------------

    private void CompileFuncCall(FunctionCallNode node, int ln)
    {
        if (HasSplatArg(node.Arguments)) { Fallback(node); return; }
        // Any function whose Op is not a plain Identifier token is a language builtin
        // (e.g. typeof, deserialize, serialize) handled entirely by the interpreter —
        // not findable via LoadGlobal, so fall back.
        if (node.Op != TokenName.Default) { Fallback(node); return; }
        // Package-prefixed calls (e.g. math::round): evaluate arguments via VM so that
        // VM-local variables (e.g. mangled var-block names) are read from the correct
        // stack slots, then dispatch through DoCall which invokes the interpreter with
        // the already-evaluated argument values.
        EmitLoad(node.FunctionName, ln);
        int argc = CompileArgs(node.Arguments);
        _chunk.Emit(Opcode.Call, argc, 0, ln);
    }

    // -- Method call -----------------------------------------------------------

    private void CompileMethodCall(MethodCallNode node, int ln)
    {
        if (HasSplatArg(node.Arguments)) { Fallback(node); return; }
        CompileNode(node.Object!);
        int argc    = CompileArgs(node.Arguments);
        int nameIdx = _chunk.AddName(node.MethodName);
        _chunk.Emit(Opcode.CallMethod, argc, nameIdx, ln);
    }

    // -- Helpers ---------------------------------------------------------------

    private void CompileBody(IEnumerable<ASTNode?> stmts)
    {
        foreach (var s in stmts)
            if (s != null) CompileStatement(s);
    }

    /// <summary>
    /// Compile a body as an expression: all but the last statement in statement context;
    /// the last statement is compiled via CompileNode so its value stays on the stack.
    /// If the body is empty or the last node is not an expression, pushes Null.
    /// </summary>
    private void CompileBodyExpr(IEnumerable<ASTNode?> stmts)
    {
        var body = stmts.Where(s => s != null).ToList();
        if (body.Count == 0) { _chunk.Emit(Opcode.Null); return; }

        for (int i = 0; i < body.Count - 1; i++)
            CompileStatement(body[i]!);

        if (!CompileNode(body[^1]!))
            _chunk.Emit(Opcode.Null);
    }

    /// <summary>
    /// Compile a function/lambda body: all but the last statement in statement context;
    /// the last statement is compiled in expression context so its value is implicitly
    /// returned (matching the tree-walker's ExecuteBody semantics).
    /// Returns true if a Return opcode was emitted for the last statement.
    /// </summary>
    private bool CompileFunctionBody(List<ASTNode?> stmts)
    {
        var body = stmts.Where(s => s != null).ToList();
        if (body.Count == 0) return false;

        for (int i = 0; i < body.Count - 1; i++)
            CompileStatement(body[i]!);

        var last = body[^1]!;
        if (CompileNode(last))
        {
            if (!_isGlobal && _locals.Any(l => l.Captured))
                _chunk.Emit(Opcode.CloseUpvalue, 0);
            _chunk.Emit(Opcode.Return);
            return true;
        }
        // Last statement was control-flow (Return, loop, etc.) — it handled its own return.
        // Caller will emit EmitFinalReturn as a safety net (unreachable after explicit returns).
        return false;
    }

    private static bool HasSplatArg(IEnumerable<ASTNode?> args)
        => args.Any(a => a is SplatNode or NamedArgumentNode);

    private int CompileArgs(IEnumerable<ASTNode?> args)
    {
        int n = 0;
        foreach (var a in args)
        {
            if (a == null) continue;
            CompileNode(a);
            n++;
        }
        return n;
    }
}
