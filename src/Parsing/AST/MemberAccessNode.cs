namespace kiwi.Parsing.AST;

public class MemberAccessNode(ASTNode? obj, string memberName) : ASTNode(ASTNodeType.MemberAccess)
{
    public ASTNode? Object { get; } = obj;
    public string MemberName { get; } = memberName;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        Print($"MemberAccess: `{MemberName}` on object: ");
        Object?.Print(1 + depth);
    }

    public override ASTNode Clone() => new MemberAccessNode(Object?.Clone(), MemberName)
    {
        Token = Token
    };
}