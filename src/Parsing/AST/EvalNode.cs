namespace kiwi.Parsing.AST;

public class EvalNode(ASTNode? parseValue) : ASTNode(ASTNodeType.Eval)
{
    public ASTNode? ParseValue { get; } = parseValue;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();
        ParseValue?.Print(1 + depth);
    }

    public override ASTNode Clone() => new EvalNode(ParseValue?.Clone())
    {
        Token = Token
    };
}