namespace kiwi.Parsing.AST;

public class MemberAssignmentNode : ASTNode
{
    public MemberAssignmentNode(ASTNode? obj,
                         string memberName, TokenName op,
                         ASTNode? initializer)
        : base(ASTNodeType.MemberAssignment)
    {
        Object = obj;
        MemberName = memberName;
        Op = op;
        Initializer = initializer;

    }

    public ASTNode? Object { get; set; }
    public string MemberName { get; set; } = string.Empty;
    public TokenName Op { get; set; }
    public ASTNode? Initializer { get; set; }

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        Print($"MemberAssignment: `{MemberName}` on object: ");
        ASTTracer.PrintDepth(1 + depth);
        Object?.Print(2 + depth);
        ASTTracer.PrintDepth(1 + depth);
        Print("Initializer:");
        Initializer?.Print(2 + depth);
    }

    public override ASTNode Clone() => new MemberAssignmentNode(Object?.Clone(), MemberName, Op, Initializer?.Clone())
    {
        Token = Token
    };
}