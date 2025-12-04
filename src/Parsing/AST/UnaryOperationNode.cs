using kiwi.Typing;

namespace kiwi.Parsing.AST;

public class UnaryOperationNode : ASTNode
{
    public UnaryOperationNode() : base(ASTNodeType.UnaryOperation) { }
    public UnaryOperationNode(TokenName op, ASTNode? operand)
        : base(ASTNodeType.UnaryOperation)
    {
        Op = op;
        Operand = operand;
    }

    public TokenName Op { get; }
    public ASTNode? Operand { get; }

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        Console.WriteLine($"UnaryOperation: {Serializer.GetOperatorString(Op)}");
        Operand?.Print(1 + depth);
    }

    public override ASTNode Clone() => new UnaryOperationNode(Op, Operand?.Clone()) { Token = Token };
}