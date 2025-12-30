namespace kiwi.Parsing.AST;

public class SelfNode : ASTNode
{
    public SelfNode() : base(ASTNodeType.Self) { }
    public SelfNode(string name)
        : base(ASTNodeType.Self) => Name = name;

    public string Name { get; } = string.Empty;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();

        if (!string.IsNullOrEmpty(Name))
        {
            ASTTracer.PrintDepth(1 + depth);
            Print($"Name: `{ASTTracer.Unmangle(Name)}`");
        }
    }

    public override ASTNode Clone() => new SelfNode(Name)
    {
        Token = Token
    };
}