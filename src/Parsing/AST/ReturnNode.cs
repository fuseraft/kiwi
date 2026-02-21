namespace kiwi.Parsing.AST;

public class ReturnNode : ASTNode
{
    public ReturnNode() : base(ASTNodeType.Return) { }
    public ReturnNode(ASTNode? returnValue, ASTNode? condition)
        : base(ASTNodeType.Return)
    {
        ReturnValue = returnValue;
        Condition = condition;
    }

    public ASTNode? ReturnValue { get; set; }
    public ASTNode? Condition { get; set; }

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();

        ReturnValue?.Print(1 + depth);

        if (Condition != null)
        {
            ASTTracer.PrintDepth(1 + depth);
            Print("Condition:");
            Condition.Print(1 + depth);
        }
    }

    public override ASTNode Clone() => new ReturnNode(ReturnValue?.Clone(), Condition?.Clone())
    {
        Token = Token
    };
}