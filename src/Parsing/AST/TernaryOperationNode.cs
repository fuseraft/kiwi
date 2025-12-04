namespace kiwi.Parsing.AST;

public class TernaryOperationNode : ASTNode
{
    public TernaryOperationNode()
        : base(ASTNodeType.TernaryOperation) { }
    public TernaryOperationNode(ASTNode? evalExpression, ASTNode? trueExpression, ASTNode? falseExpression)
      : base(ASTNodeType.TernaryOperation)
    {
        EvalExpression = evalExpression;
        TrueExpression = trueExpression;
        FalseExpression = falseExpression;
    }

    public ASTNode? EvalExpression { get; }
    public ASTNode? TrueExpression { get; }
    public ASTNode? FalseExpression { get; }

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        Console.WriteLine("TernaryOperation:");
        ASTTracer.PrintDepth(1 + depth);
        Console.WriteLine("Evaluate expression:");
        EvalExpression?.Print(2 + depth);
        ASTTracer.PrintDepth(1 + depth);
        Console.WriteLine("True expression:");
        TrueExpression?.Print(2 + depth);
        ASTTracer.PrintDepth(1 + depth);
        Console.WriteLine("False expression:");
        FalseExpression?.Print(2 + depth);
    }

    public override ASTNode Clone() => new TernaryOperationNode(EvalExpression?.Clone(), TrueExpression?.Clone(), FalseExpression?.Clone()) { Token = Token };
}