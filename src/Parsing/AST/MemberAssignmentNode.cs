namespace kiwi.Parsing.AST;

public class MemberAssignmentNode(ASTNode? obj, string memberName, TokenName op, ASTNode? initializer) 
    : ASTNode(ASTNodeType.MemberAssignment)
{
    public ASTNode? Object { get; set; } = obj;
    public string MemberName { get; set; } = memberName;
    public TokenName Op { get; set; } = op;
    public ASTNode? Initializer { get; set; } = initializer;

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