namespace kiwi.Parsing.AST;

public class NextNode : ASTNode
{
    public NextNode()
        : base(ASTNodeType.Next) { }
    public NextNode(ASTNode? condition)
        : base(ASTNodeType.Next)
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

    public override ASTNode Clone()
    {
        return new NextNode(Condition?.Clone())
        {
            Token = Token
        };
    }
}