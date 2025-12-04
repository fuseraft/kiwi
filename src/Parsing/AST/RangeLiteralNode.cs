namespace kiwi.Parsing.AST;

public class RangeLiteralNode(ASTNode? rangeStart, ASTNode? rangeEnd) : ASTNode(ASTNodeType.RangeLiteral)
{
    public ASTNode? RangeStart { get; } = rangeStart;
    public ASTNode? RangeEnd { get; } = rangeEnd;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();
        RangeStart?.Print(1 + depth);
        RangeEnd?.Print(1 + depth);
    }

    public override ASTNode Clone() => new RangeLiteralNode(RangeStart?.Clone(), RangeEnd?.Clone())
    {
        Token = Token
    };
}