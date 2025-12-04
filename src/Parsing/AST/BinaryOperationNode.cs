using kiwi.Typing;

namespace kiwi.Parsing.AST;

public class BinaryOperationNode : ASTNode
{
    public BinaryOperationNode() : base(ASTNodeType.BinaryOperation) { }
    public BinaryOperationNode(ASTNode? left, TokenName op, ASTNode? right)
        : base(ASTNodeType.BinaryOperation)
    {
        Left = left;
        Op = op;
        Right = right;
    }

    public ASTNode? Left { get; }
    public TokenName Op { get; }
    public ASTNode? Right { get; }

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        Console.WriteLine($"BinaryOperation: {Serializer.GetOperatorString(Op)}");
        Left?.Print(1 + depth);
        Right?.Print(1 + depth);
    }

    public override ASTNode Clone() => new BinaryOperationNode(Left?.Clone(), Op, Right?.Clone())
    {
        Token = Token
    };
}