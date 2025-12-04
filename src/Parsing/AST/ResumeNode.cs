namespace kiwi.Parsing.AST;

public class ResumeNode(ASTNode expression) : ASTNode(ASTNodeType.Resume)
{
    public ASTNode Expression { get; } = expression;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();
        Expression?.Print(1 + depth);
    }

    public override ASTNode Clone()
    {
        return new ResumeNode(Expression.Clone())
        {
            Token = Token
        };
    }
}