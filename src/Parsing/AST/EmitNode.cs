namespace kiwi.Parsing.AST;

public class EmitNode(ASTNode eventName, List<ASTNode?> eventArgs) : ASTNode(ASTNodeType.Emit)
{
    public ASTNode EventName { get; } = eventName;
    public List<ASTNode?> EventArgs { get; } = eventArgs;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();
        EventName?.Print(1 + depth);
        Print("Arguments:");
        foreach (var arg in EventArgs)
        {
            arg?.Print(2 + depth);
        }
    }

    public override ASTNode Clone()
    {
        List<ASTNode?> eventArgs = [];
        foreach (var arg in EventArgs)
        {
            eventArgs.Add(arg?.Clone());
        }

        return new EmitNode(EventName.Clone(), eventArgs)
        {
            Token = Token
        };
    }
}