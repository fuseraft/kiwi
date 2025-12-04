namespace kiwi.Parsing.AST;

public class SpawnNode(ASTNode expression) : ASTNode(ASTNodeType.Spawn)
{
    public ASTNode Expression { get; } = expression;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();
        Expression?.Print(1 + depth);
    }

    public override ASTNode Clone() => new SpawnNode(Expression.Clone())
    {
        Token = Token
    };
}