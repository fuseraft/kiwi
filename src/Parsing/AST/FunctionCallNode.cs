namespace kiwi.Parsing.AST;

public class FunctionCallNode(string functionName, TokenName op, List<ASTNode?> arguments) 
    : ASTNode(ASTNodeType.FunctionCall)
{
    public string FunctionName { get; set; } = functionName;
    public TokenName Op { get; set; } = op;
    public List<ASTNode?> Arguments { get; set; } = arguments;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        Print($"FunctionCall: `{ASTTracer.Unmangle(FunctionName)}`");
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

        return new FunctionCallNode(FunctionName, Op, clonedArguments)
        {
            Token = Token
        };
    }
}