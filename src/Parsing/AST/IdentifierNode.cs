namespace kiwi.Parsing.AST;

public class IdentifierNode(string name) : ASTNode(ASTNodeType.Identifier)
{
    public string Name { get; } = name;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        Print($"Identifier: `{ASTTracer.Unmangle(Name)}`");
    }

    public override ASTNode Clone() => new IdentifierNode(Name)
    {
        Token = Token
    };
}