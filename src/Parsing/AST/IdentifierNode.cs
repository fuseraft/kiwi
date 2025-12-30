namespace kiwi.Parsing.AST;

public class IdentifierNode : ASTNode
{
    public IdentifierNode() : base(ASTNodeType.Identifier) { }
    public IdentifierNode(string name)
        : base(ASTNodeType.Identifier)
    {
        Name = name;
    }

    public string Name { get; } = string.Empty;
    public string Package { get; set; } = string.Empty;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        Print($"Identifier: `{ASTTracer.Unmangle(Name)}`");
    }

    public override ASTNode Clone() => new IdentifierNode(Name)
    {
        Package = Package,
        Token = Token
    };
}