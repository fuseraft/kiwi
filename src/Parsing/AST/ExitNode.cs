namespace kiwi.Parsing.AST;

public class ExitNode(ASTNode? exitValue, ASTNode? condition) : ASTNode(ASTNodeType.Exit)
{
    public ASTNode? ExitValue { get; set; } = exitValue;
    public ASTNode? Condition { get; set; } = condition;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();

        ExitValue?.Print(1 + depth);

        if (Condition != null)
        {
            ASTTracer.PrintDepth(1 + depth);
            Print("Condition:");
            Condition.Print(1 + depth);
        }
    }

    public override ASTNode Clone() => new ExitNode(ExitValue?.Clone(), Condition?.Clone())
    {
        Token = Token
    };
}