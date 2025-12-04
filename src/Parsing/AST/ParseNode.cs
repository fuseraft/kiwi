namespace kiwi.Parsing.AST;

public class ParseNode : ASTNode
{
    public ParseNode() : base(ASTNodeType.Parse) { }
    public ParseNode(ASTNode? parseValue)
        : base(ASTNodeType.Parse)
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

    public override ASTNode Clone() => new ParseNode(ParseValue?.Clone())
    {
        Token = Token
    };
}