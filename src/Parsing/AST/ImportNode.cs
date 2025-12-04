namespace kiwi.Parsing.AST;

public class ImportNode : ASTNode
{
    public ImportNode() : base(ASTNodeType.Import) { }
    public ImportNode(ASTNode? packageName)
        : base(ASTNodeType.Import)
    {
        PackageName = packageName;
    }

    public ASTNode? PackageName { get; }

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