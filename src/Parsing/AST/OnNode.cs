namespace kiwi.Parsing.AST;

public class OnNode(ASTNode eventName, ASTNode callback) : ASTNode(ASTNodeType.On)
{
    public ASTNode EventName { get; } = eventName;
    public ASTNode Callback { get; } = callback;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();
        EventName?.Print(1 + depth);
        Callback?.Print(1 + depth);
    }

    public override ASTNode Clone() => new OnNode(EventName.Clone(), Callback.Clone())
    {
        Token = Token
    };
}