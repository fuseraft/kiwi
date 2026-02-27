namespace kiwi.Parsing.AST;

public class CaseWhenNode() : ASTNode(ASTNodeType.CaseWhen)
{
    public List<ASTNode?> Conditions { get; set; } = [];
    public List<ASTNode?> Body { get; set; } = [];

    public override void Print(int depth = 0)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();

        ASTTracer.PrintDepth(1 + depth);
        Print("Conditions:");
        foreach (var cond in Conditions)
        {
            cond?.Print(2 + depth);
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
        List<ASTNode?> clonedConditions = [];
        foreach (var cond in Conditions)
        {
            clonedConditions.Add(cond?.Clone());
        }

        List<ASTNode?> clonedBody = [];
        foreach (var stmt in Body)
        {
            clonedBody.Add(stmt?.Clone());
        }

        return new CaseWhenNode
        {
            Conditions = clonedConditions,
            Body = clonedBody,
            Token = Token
        };
    }
}
