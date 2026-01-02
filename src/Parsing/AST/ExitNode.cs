namespace kiwi.Parsing.AST;

public class ExitNode : ASTNode
{
    public ExitNode()
        : base(ASTNodeType.Exit) { }
    public ExitNode(ASTNode? exitValue, ASTNode? condition)
        : base(ASTNodeType.Exit)
    {
        ExitValue = exitValue;
        Condition = condition;
    }

    public ASTNode? ExitValue { get; set; }
    public ASTNode? Condition { get; set; }

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