namespace kiwi.Parsing.AST;

public class ForLoopNode(ASTNode valueIterator) : ASTNode(ASTNodeType.ForLoop)
{
    public ASTNode? DataSet { get; set; }
    public ASTNode ValueIterator { get; set; } = valueIterator;
    public ASTNode? IndexIterator { get; set; }
    public List<ASTNode?> Body { get; set; } = [];

    public override void Print(int depth = 0)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();

        ASTTracer.PrintDepth(1 + depth);
        Print("LoopSet:");
        DataSet?.Print(2 + depth);

        ASTTracer.PrintDepth(1 + depth);
        Print("ValueIterator:");
        ValueIterator?.Print(2 + depth);

        if (IndexIterator != null)
        {
            ASTTracer.PrintDepth(1 + depth);
            Print("IndexIterator:");
            IndexIterator.Print(2 + depth);
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

        return new ForLoopNode(ValueIterator.Clone())
        {
            DataSet = DataSet?.Clone(),
            IndexIterator = IndexIterator?.Clone(),
            Body = clonedBody,
            Token = Token
        };
    }
}