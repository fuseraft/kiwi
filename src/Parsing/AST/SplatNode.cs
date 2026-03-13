namespace kiwi.Parsing.AST;

// Represents *expr in a call argument list - spreads a list into positional args.
public class SplatNode(ASTNode? expression) : ASTNode(ASTNodeType.Splat)
{
    public ASTNode? Expression { get; set; } = expression;

    public override void Print(int depth = 0)
    {
        ASTTracer.PrintDepth(depth);
        Print("Splat:");
        Expression?.Print(1 + depth);
    }

    public override ASTNode Clone() => new SplatNode(Expression?.Clone()) { Token = Token };
}
