namespace kiwi.Parsing.AST;

public class NoOpNode : ASTNode
{
    public NoOpNode() : base(ASTNodeType.NoOp) { }

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();
    }

    public override ASTNode Clone()
    {
        return new NoOpNode()
        {
            Token = Token
        };
    }
}