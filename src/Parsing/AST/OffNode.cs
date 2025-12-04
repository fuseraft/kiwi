namespace kiwi.Parsing.AST;

public class OffNode(ASTNode eventName, ASTNode? callback) : ASTNode(ASTNodeType.Off)
{
    public ASTNode EventName { get; } = eventName;
    public ASTNode? Callback { get; } = callback;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();
        EventName.Print(1 + depth);
        Callback?.Print(1 + depth);
    }

    public override ASTNode Clone() => new OffNode(EventName.Clone(), Callback?.Clone())
    {
        Token = Token
    };
}