namespace kiwi.Parsing.AST;

public class YieldNode : ASTNode
{
    public YieldNode() : base(ASTNodeType.Yield) { }

    public ASTNode? YieldValue { get; set; }

    public override void Print(int depth = 0)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();
        YieldValue?.Print(1 + depth);
    }

    public override ASTNode Clone() => new YieldNode
    {
        YieldValue = YieldValue?.Clone(),
        Token = Token
    };
}
