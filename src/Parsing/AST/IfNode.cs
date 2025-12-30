namespace kiwi.Parsing.AST;

public class IfNode : ASTNode
{
    public IfNode() : base(ASTNodeType.If) { }
    public ASTNode? Condition { get; set; }
    public List<ASTNode?> Body { get; set; } = [];
    public List<ASTNode?> ElseBody { get; set; } = [];
    public List<IfNode?> ElsifNodes { get; set; } = [];

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();
        Condition?.Print(1 + depth);
        ASTTracer.PrintDepth(depth);
        Print("Statements:");
        foreach (var stmt in Body)
        {
            stmt?.Print(1 + depth);
        }

        if (ElsifNodes.Count > 0)
        {
            foreach (var elsif in ElsifNodes)
            {
                ASTTracer.PrintDepth(depth);
                Print("Else-If:");
                elsif?.Print(1 + depth);
            }
        }

        if (ElseBody.Count > 0)
        {
            ASTTracer.PrintDepth(depth);
            Print("Else:");
            foreach (var stmt in ElseBody)
            {
                stmt?.Print(1 + depth);
            }
        }
    }

    public override ASTNode Clone()
    {
        List<ASTNode?> clonedBody = [];
        foreach (var stmt in Body)
        {
            clonedBody.Add(stmt?.Clone());
        }

        List<ASTNode?> clonedElseBody = [];
        foreach (var stmt in ElseBody)
        {
            clonedElseBody.Add(stmt?.Clone());
        }

        List<IfNode?> clonedElsifNodes = [];
        foreach (var elif in ElsifNodes)
        {
            clonedElsifNodes.Add(elif?.Clone() as IfNode);
        }

        return new IfNode
        {
            Condition = Condition?.Clone(),
            Body = clonedBody,
            ElseBody = clonedElseBody,
            ElsifNodes = clonedElsifNodes,
            Token = Token
        };
    }
}