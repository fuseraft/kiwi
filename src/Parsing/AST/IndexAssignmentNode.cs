namespace kiwi.Parsing.AST;

public class IndexAssignmentNode(ASTNode? obj, TokenName op, ASTNode? initializer) 
    : ASTNode(ASTNodeType.IndexAssignment)
{
    public ASTNode? Object { get; set; } = obj;
    public TokenName Op { get; set; } = op;
    public ASTNode? Initializer { get; set; } = initializer;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();
        Object?.Print(1 + depth);
        ASTTracer.PrintDepth(1 + depth);
        Print("Initializer:");
        Initializer?.Print(2 + depth);
    }

    public override ASTNode Clone() => new IndexAssignmentNode(Object?.Clone(), Op, Initializer?.Clone())
    {
        Token = Token
    };
}