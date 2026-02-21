namespace kiwi.Parsing.AST;

public class CaseNode() : ASTNode(ASTNodeType.Case)
{
    public ASTNode? TestValue { get; set; }
    public ASTNode? TestValueAlias { get; set; }
    public List<ASTNode?> ElseBody { get; set; } = [];
    public List<CaseWhenNode> WhenNodes { get; set; } = [];

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();

        if (TestValue != null)
        {
            ASTTracer.PrintDepth(1 + depth);
            Print("Test:");
            TestValue.Print(2 + depth);

            if (TestValueAlias != null)
            {
                ASTTracer.PrintDepth(1 + depth);
                Print("Alias:");
                TestValueAlias.Print(2 + depth);
            }
        }

        if (WhenNodes.Count > 0)
        {
            foreach (var when in WhenNodes)
            {
                when?.Print(1 + depth);
            }
        }

        if (ElseBody.Count > 0)
        {
            ASTTracer.PrintDepth(1 + depth);
            Print("Case else:");
            foreach (var stmt in ElseBody)
            {
                stmt?.Print(2 + depth);
            }
        }
    }

    public override ASTNode Clone()
    {
        List<ASTNode?> clonedElseBody = [];
        foreach (var stmt in ElseBody)
        {
            clonedElseBody.Add(stmt?.Clone());
        }

        List<CaseWhenNode> clonedWhenNodes = [];
        foreach (var when in WhenNodes)
        {
            clonedWhenNodes.Add((CaseWhenNode)when.Clone());
        }

        return new CaseNode
        {
            TestValue = TestValue?.Clone(),
            TestValueAlias = TestValueAlias?.Clone(),
            ElseBody = clonedElseBody,
            WhenNodes = clonedWhenNodes,
            Token = Token
        };
    }
}