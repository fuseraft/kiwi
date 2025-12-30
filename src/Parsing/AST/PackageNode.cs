using kiwi.Parsing.AST;

public class PackageNode : ASTNode
{
    public PackageNode() : base(ASTNodeType.Package) { }
    public PackageNode(ASTNode? packageName)
        : base(ASTNodeType.Package)
    {
        PackageName = packageName;
    }

    public ASTNode? PackageName { get; }
    public List<ASTNode?> Body { get; set; } = [];

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();
        PackageName?.Print(1 + depth);

        ASTTracer.PrintDepth(1 + depth);
        Print("Body:");
        foreach (var stmt in Body)
        {
            stmt?.Print(2 + depth);
        }
    }

    public override ASTNode Clone()
    {
        List<ASTNode?> clonedBody = [];
        foreach (var statement in Body)
        {
            clonedBody.Add(statement?.Clone());
        }

        return new PackageNode(PackageName?.Clone())
        {
            Body = clonedBody,
            Token = Token
        };
    }
}