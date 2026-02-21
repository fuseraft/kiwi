namespace kiwi.Parsing.AST;

public class PackAssignmentNode : ASTNode
{
    public PackAssignmentNode() : base(ASTNodeType.PackAssignment) { }
    public PackAssignmentNode(List<ASTNode?> left, List<ASTNode?> right, TokenName op)
        : base(ASTNodeType.PackAssignment)
    {
        Left = left;
        Right = right;
        Op = op;
    }

    public List<ASTNode?> Left { get; set; } = [];
    public List<ASTNode?> Right { get; set; } = [];
    public TokenName Op { get; set; } = TokenName.Default;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();
        ASTTracer.PrintDepth(1 + depth);
        Print("Left-hand side:");
        foreach (var lhs in Left)
        {
            lhs?.Print(2 + depth);
        }
        ASTTracer.PrintDepth(1 + depth);
        Print("Right-hand side:");
        foreach (var rhs in Right)
        {
            rhs?.Print(2 + depth);
        }
    }

    public override ASTNode Clone()
    {
        List<ASTNode?> clonedLeft = [];
        foreach (var lhs in Left)
        {
            clonedLeft.Add(lhs?.Clone());
        }

        List<ASTNode?> clonedRight = [];
        foreach (var rhs in Right)
        {
            clonedRight.Add(rhs?.Clone());
        }

        return new PackAssignmentNode(clonedLeft, clonedRight, Op)
        {
            Token = Token
        };
    }
}