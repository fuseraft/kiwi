namespace kiwi.Parsing.AST;

public class MethodCallNode(ASTNode? obj, string methodName, TokenName op, List<ASTNode?> arguments) 
    : ASTNode(ASTNodeType.MethodCall)
{
    public ASTNode? Object { get; set; } = obj;
    public string MethodName { get; set; } = methodName;
    public TokenName Op { get; set; } = op;
    public List<ASTNode?> Arguments { get; set; } = arguments;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        Print($"MethodCall: `{MethodName}` on object:");
        Object?.Print(1 + depth);
        ASTTracer.PrintDepth(1 + depth);
        Print("Arguments:");
        foreach (var arg in Arguments)
        {
            arg?.Print(2 + depth);
        }
    }

    public override ASTNode Clone()
    {
        List<ASTNode?> clonedArguments = [];
        foreach (var arg in Arguments)
        {
            clonedArguments.Add(arg?.Clone());
        }

        return new MethodCallNode(Object?.Clone(), MethodName, Op, clonedArguments)
        {
            Token = Token
        };
    }
}