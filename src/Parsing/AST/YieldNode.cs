namespace kiwi.Parsing.AST;

public class YieldNode : ASTNode
{
    public YieldNode() : base(ASTNodeType.Yield) { }

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();
    }

    public override ASTNode Clone()
    {
        return new YieldNode()
        {
            Token = Token
        };
    }
}