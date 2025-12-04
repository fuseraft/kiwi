namespace kiwi.Parsing.AST;

public class DoNode : ASTNode
{
    public DoNode() : base(ASTNodeType.Do) { }
    public ASTNode? Condition { get; set; }
    public List<ASTNode?> Body { get; set; } = [];
    
    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();
        Condition?.Print(1 + depth);
        ASTTracer.PrintDepth(depth);
        Console.WriteLine("Statements:");
        foreach (var stmt in Body)
        {
            stmt?.Print(1 + depth);
        }
    }

    public override ASTNode Clone()
    {
        List<ASTNode?> clonedBody = [];
        foreach (var stmt in Body)
        {
            clonedBody.Add(stmt?.Clone());
        }

        return new DoNode
        {
            Condition = Condition?.Clone(),
            Body = clonedBody,
            Token = Token
        };
    }
}