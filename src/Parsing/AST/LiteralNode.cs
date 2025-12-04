using kiwi.Typing;

namespace kiwi.Parsing.AST;

public class LiteralNode(Value value) : ASTNode(ASTNodeType.Literal)
{
    public Value Value { get; } = value;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        Console.WriteLine($"Literal: {Serializer.Serialize(Value, true)}");
    }

    public override ASTNode Clone() => new LiteralNode(Value)
    {
        Token = Token
    };
};