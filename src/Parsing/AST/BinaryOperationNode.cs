using kiwi.Typing;

namespace kiwi.Parsing.AST;

public class BinaryOperationNode(ASTNode? left, TokenName op, ASTNode? right) : ASTNode(ASTNodeType.BinaryOperation)
{
    public ASTNode? Left { get; } = left;
    public TokenName Op { get; } = op;
    public ASTNode? Right { get; } = right;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        Print($"BinaryOperation: {Serializer.GetOperatorString(Op)}");
        Left?.Print(1 + depth);
        Right?.Print(1 + depth);
    }

    public override ASTNode Clone() => new BinaryOperationNode(Left?.Clone(), Op, Right?.Clone())
    {
        Token = Token
    };
}