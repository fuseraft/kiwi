using kiwi.Typing;

namespace kiwi.Parsing.AST;

public class UnaryOperationNode(TokenName op, ASTNode? operand) : ASTNode(ASTNodeType.UnaryOperation)
{
    public TokenName Op { get; } = op;
    public ASTNode? Operand { get; } = operand;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        Print($"UnaryOperation: {Serializer.GetOperatorString(Op)}");
        Operand?.Print(1 + depth);
    }

    public override ASTNode Clone() => new UnaryOperationNode(Op, Operand?.Clone()) { Token = Token };
}