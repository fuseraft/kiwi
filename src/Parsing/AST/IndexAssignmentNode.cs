namespace kiwi.Parsing.AST;

public class IndexAssignmentNode : ASTNode
{
    public IndexAssignmentNode() : base(ASTNodeType.IndexAssignment) { }
    public IndexAssignmentNode(ASTNode? obj, TokenName op, ASTNode? initializer)
          : base(ASTNodeType.IndexAssignment)
    {
        Object = obj;
        Initializer = initializer;
        Op = op;
    }

    public ASTNode? Object { get; set; }
    public TokenName Op { get; set; } = TokenName.Default;
    public ASTNode? Initializer { get; set; }

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();
        Object?.Print(1 + depth);
        ASTTracer.PrintDepth(1 + depth);
        Console.WriteLine("Initializer:");
        Initializer?.Print(2 + depth);
    }

    public override ASTNode Clone() => new IndexAssignmentNode(Object?.Clone(), Op, Initializer?.Clone())
    {
        Token = Token
    };
}