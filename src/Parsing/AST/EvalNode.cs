namespace kiwi.Parsing.AST;

public class EvalNode : ASTNode
{
    public EvalNode() : base(ASTNodeType.Eval) { }
    public EvalNode(ASTNode? parseValue)
        : base(ASTNodeType.Eval)
    {
        ParseValue = parseValue;
    }

    public ASTNode? ParseValue { get; }

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