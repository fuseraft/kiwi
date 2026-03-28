namespace kiwi.Parsing.AST;

/// <summary>
/// Represents a <c>super.method(args)</c> call — invokes the base struct's
/// version of a method on the current <c>self</c> instance.
/// </summary>
public class SuperCallNode(string methodName, List<ASTNode?> arguments)
    : ASTNode(ASTNodeType.SuperCall)
{
    public string MethodName { get; } = methodName;
    public List<ASTNode?> Arguments { get; } = arguments;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        Print($"SuperCall: `{MethodName}`");
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

        return new SuperCallNode(MethodName, clonedArguments)
        {
            Token = Token
        };
    }
}
