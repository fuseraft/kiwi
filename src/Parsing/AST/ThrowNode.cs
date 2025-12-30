namespace kiwi.Parsing.AST;

public class ThrowNode : ASTNode
{
    public ThrowNode()
        : base(ASTNodeType.Throw) { }
    public ThrowNode(ASTNode? errorValue, ASTNode? condition)
        : base(ASTNodeType.Throw)
    {
        ErrorValue = errorValue;
        Condition = condition;
    }

    public ASTNode? ErrorValue { get; set; }
    public ASTNode? Condition { get; set; }

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();

        ErrorValue?.Print(1 + depth);

        if (Condition != null)
        {
            ASTTracer.PrintDepth(1 + depth);
            Print("Condition:");
            Condition.Print(1 + depth);
        }
    }

    public override ASTNode Clone()
    {
        return new ThrowNode(ErrorValue?.Clone(), Condition?.Clone())
        {
            Token = Token
        };
    }
}