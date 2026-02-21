namespace kiwi.Parsing.AST;

public class TryNode() : ASTNode(ASTNodeType.Try)
{
    public List<ASTNode?> TryBody { get; set; } = [];
    public List<ASTNode?> CatchBody { get; set; } = [];
    public List<ASTNode?> FinallyBody { get; set; } = [];
    public ASTNode? ErrorType { get; set; }
    public ASTNode? ErrorMessage { get; set; }

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        Print("Try: ");

        ASTTracer.PrintDepth(depth);
        Print("Try statements:");
        foreach (var stmt in TryBody)
        {
            stmt?.Print(1 + depth);
        }

        if (CatchBody.Count > 0)
        {
            ASTTracer.PrintDepth(depth);
            Print("Catch:");

            if (ErrorType != null)
            {
                ASTTracer.PrintDepth(1 + depth);
                Print("Error type:");
                ErrorType?.Print(2 + depth);
            }

            if (ErrorMessage != null)
            {
                ASTTracer.PrintDepth(1 + depth);
                Print("Error message:");
                ErrorMessage?.Print(2 + depth);
            }

            ASTTracer.PrintDepth(depth);
            Print("Catch statements:");
            foreach (var stmt in CatchBody)
            {
                stmt?.Print(1 + depth);
            }
        }

        if (FinallyBody.Count > 0)
        {
            ASTTracer.PrintDepth(depth);
            Print("Finally statements:");
            foreach (var stmt in FinallyBody)
            {
                stmt?.Print(1 + depth);
            }
        }
    }

    public override ASTNode Clone()
    {
        List<ASTNode?> clonedTryBody = [];
        foreach (var stmt in TryBody)
        {
            clonedTryBody.Add(stmt?.Clone());
        }

        List<ASTNode?> clonedCatchBody = [];
        foreach (var stmt in CatchBody)
        {
            clonedCatchBody.Add(stmt?.Clone());
        }

        List<ASTNode?> clonedFinallyBody = [];
        foreach (var stmt in FinallyBody)
        {
            clonedFinallyBody.Add(stmt?.Clone());
        }

        return new TryNode
        {
            TryBody = clonedTryBody,
            CatchBody = clonedCatchBody,
            FinallyBody = clonedFinallyBody,
            ErrorType = ErrorType?.Clone(),
            ErrorMessage = ErrorMessage?.Clone(),
            Token = Token,
        };
    }
}