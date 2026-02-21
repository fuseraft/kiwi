namespace kiwi.Parsing.AST;

public class ThrowNode(ASTNode? errorValue, ASTNode? condition) : ASTNode(ASTNodeType.Throw)
{
    public ASTNode? ErrorValue { get; set; } = errorValue;
    public ASTNode? Condition { get; set; } = condition;

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