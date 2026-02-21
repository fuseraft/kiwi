namespace kiwi.Parsing.AST;

public class LambdaCallNode(ASTNode? lambdaNode, List<ASTNode?> arguments) : ASTNode(ASTNodeType.LambdaCall)
{
    public ASTNode? LambdaNode { get; set; } = lambdaNode;
    public List<ASTNode?> Arguments { get; set; } = arguments;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();
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

        return new LambdaCallNode(LambdaNode?.Clone(), clonedArguments)
        {
            Token = Token
        };
    }
}