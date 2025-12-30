namespace kiwi.Parsing.AST;

public class RepeatLoopNode : ASTNode
{
    public RepeatLoopNode()
        : base(ASTNodeType.RepeatLoop) { }

    public ASTNode? Count { get; set; }
    public ASTNode? Alias { get; set; }
    public List<ASTNode?> Body { get; set; } = [];

    public override void Print(int depth = 0)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();

        ASTTracer.PrintDepth(1 + depth);
        Print("Count:");
        Count?.Print(2 + depth);

        if (Alias != null)
        {
            ASTTracer.PrintDepth(1 + depth);
            Print("Alias:");
            Alias?.Print(2 + depth);
        }

        ASTTracer.PrintDepth(1 + depth);
        Print("Statements:");
        foreach (var stmt in Body)
        {
            stmt?.Print(2 + depth);
        }
    }

    public override ASTNode Clone()
    {
        List<ASTNode?> clonedBody = [];
        foreach (var stmt in Body)
        {
            clonedBody.Add(stmt?.Clone());
        }

        return new RepeatLoopNode
        {
            Count = Count?.Clone(),
            Alias = Alias?.Clone(),
            Body = clonedBody,
            Token = Token
        };
    }
}