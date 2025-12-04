namespace kiwi.Parsing.AST;

public class MemberAccessNode : ASTNode
{
    public MemberAccessNode() : base(ASTNodeType.MemberAccess) { }
    public MemberAccessNode(ASTNode? obj, string memberName)
        : base(ASTNodeType.MemberAccess)
    {
        Object = obj;
        MemberName = memberName;
    }

    public ASTNode? Object { get; }
    public string MemberName { get; } = string.Empty;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        Console.WriteLine($"MemberAccess: `{MemberName}` on object: ");
        Object?.Print(1 + depth);
    }

    public override ASTNode Clone() => new MemberAccessNode(Object?.Clone(), MemberName)
    {
        Token = Token
    };
}