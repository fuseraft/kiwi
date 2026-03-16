namespace kiwi.Parsing.AST;

public class OnceNode(ASTNode eventName, ASTNode callback, int priority = 0) : ASTNode(ASTNodeType.Once)
{
    public ASTNode EventName { get; } = eventName;
    public ASTNode Callback { get; } = callback;
    public int Priority { get; } = priority;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();
        EventName?.Print(1 + depth);
        Callback?.Print(1 + depth);
    }

    public override ASTNode Clone() => new OnceNode(EventName.Clone(), Callback.Clone(), Priority)
    {
        Token = Token
    };
}
