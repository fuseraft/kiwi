namespace kiwi.Parsing.AST;

public class WhileLoopNode() : ASTNode(ASTNodeType.WhileLoop)
{
    public ASTNode? Condition { get; set; }
    public List<ASTNode?> Body { get; set; } = [];

    public override void Print(int depth = 0)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();

        ASTTracer.PrintDepth(1 + depth);
        Print("Condition:");
        Condition?.Print(2 + depth);

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

        return new WhileLoopNode
        {
            Condition = Condition?.Clone(),
            Body = clonedBody,
            Token = Token,
        };
    }
}