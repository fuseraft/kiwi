namespace kiwi.Parsing.AST;

public class TernaryOperationNode(ASTNode? evalExpression, ASTNode? trueExpression, ASTNode? falseExpression)
    : ASTNode(ASTNodeType.TernaryOperation)
{
    public ASTNode? EvalExpression { get; } = evalExpression;
    public ASTNode? TrueExpression { get; } = trueExpression;
    public ASTNode? FalseExpression { get; } = falseExpression;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        Print("TernaryOperation:");
        ASTTracer.PrintDepth(1 + depth);
        Print("Evaluate expression:");
        EvalExpression?.Print(2 + depth);
        ASTTracer.PrintDepth(1 + depth);
        Print("True expression:");
        TrueExpression?.Print(2 + depth);
        ASTTracer.PrintDepth(1 + depth);
        Print("False expression:");
        FalseExpression?.Print(2 + depth);
    }

    public override ASTNode Clone() => new TernaryOperationNode(EvalExpression?.Clone(), TrueExpression?.Clone(), FalseExpression?.Clone()) { Token = Token };
}