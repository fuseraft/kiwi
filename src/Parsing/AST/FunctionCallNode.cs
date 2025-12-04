namespace kiwi.Parsing.AST;

public class FunctionCallNode : ASTNode
{
    public FunctionCallNode() : base(ASTNodeType.FunctionCall) { }
    public FunctionCallNode(string functionName, TokenName op,
                     List<ASTNode?> arguments)
          : base(ASTNodeType.FunctionCall)
    {
        FunctionName = functionName;
        Op = op;
        Arguments = arguments;
    }

    public string FunctionName { get; set; } = string.Empty;
    public TokenName Op { get; set; } = TokenName.Default;
    public List<ASTNode?> Arguments { get; set; } = [];

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        Console.WriteLine($"FunctionCall: `{ASTTracer.Unmangle(FunctionName)}`");
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

        return new FunctionCallNode(FunctionName, Op, clonedArguments)
        {
            Token = Token
        };
    }
}