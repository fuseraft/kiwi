namespace kiwi.Parsing.AST;

public class ExportNode : ASTNode
{
    public ExportNode() : base(ASTNodeType.Export) { }
    public ExportNode(ASTNode? packageName)
        : base(ASTNodeType.Export)
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

    public override ASTNode Clone() => new ExportNode(PackageName?.Clone())
    {
        Token = Token
    };
}