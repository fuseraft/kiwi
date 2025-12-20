namespace kiwi.Parsing.AST;

public class IncludeNode(ASTNode path) : ASTNode(ASTNodeType.Include)
{
    public ASTNode Path { get; } = path;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();
        Path?.Print(1 + depth);
    }

    public override ASTNode Clone() => new IncludeNode(Path.Clone())
    {
        Token = Token
    };
}