namespace kiwi.Parsing.AST;

public class LambdaCallNode : ASTNode
{
    public LambdaCallNode() : base(ASTNodeType.LambdaCall) { }
    public LambdaCallNode(ASTNode? lambdaNode,
                     List<ASTNode?> arguments)
          : base(ASTNodeType.LambdaCall)
    {
        LambdaNode = lambdaNode;
        Arguments = arguments;
    }

    public ASTNode? LambdaNode { get; set; }
    public List<ASTNode?> Arguments { get; set; } = [];

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