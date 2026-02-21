namespace kiwi.Parsing.AST;

public class ImportNode(ASTNode? packageName) : ASTNode(ASTNodeType.Import)
{
    public ASTNode? PackageName { get; } = packageName;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();
        PackageName?.Print(1 + depth);
    }

    public override ASTNode Clone() => new ImportNode(PackageName?.Clone())
    {
        Token = Token
    };
}