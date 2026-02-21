namespace kiwi.Parsing.AST;

public class ExportNode(ASTNode? packageName) : ASTNode(ASTNodeType.Export)
{
    public ASTNode? PackageName { get; } = packageName;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();
        PackageName?.Print(1 + depth);
    }

    public override ASTNode Clone() => new ExportNode(PackageName?.Clone())
    {
        Token = Token
    };
}