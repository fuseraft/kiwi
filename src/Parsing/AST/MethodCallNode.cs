namespace kiwi.Parsing.AST;

public class MethodCallNode : ASTNode
{
    public MethodCallNode() : base(ASTNodeType.MethodCall) { }
    public MethodCallNode(ASTNode? obj, string methodName, TokenName op,
                     List<ASTNode?> arguments)
          : base(ASTNodeType.MethodCall)
    {
        Object = obj;
        MethodName = methodName;
        Op = op;
        Arguments = arguments;
    }

    public ASTNode? Object { get; set; }
    public string MethodName { get; set; } = string.Empty;
    public TokenName Op { get; set; } = TokenName.Default;
    public List<ASTNode?> Arguments { get; set; } = [];

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        Console.WriteLine($"MethodCall: `{MethodName}` on object:");
        Object?.Print(1 + depth);
        ASTTracer.PrintDepth(1 + depth);
        Console.WriteLine("Arguments:");
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