namespace kiwi.Parsing.AST;

public class BreakNode : ASTNode
{
    public BreakNode()
        : base(ASTNodeType.Break) { }
    public BreakNode(ASTNode? condition)
        : base(ASTNodeType.Break)
    {
        Condition = condition;
    }

    public ASTNode? Condition { get; set; }

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();

        if (Condition != null)
        {
            ASTTracer.PrintDepth(1 + depth);
            Print("Condition:");
            Condition.Print(1 + depth);
        }
    }

    public override ASTNode Clone() => new BreakNode(Condition?.Clone())
    {
        Token = Token
    };
}