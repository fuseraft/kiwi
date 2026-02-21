namespace kiwi.Parsing.AST;

public class ConstAssignmentNode(string name, TokenName op, ASTNode? initializer) : ASTNode(ASTNodeType.ConstAssignment)
{
    public string Name { get; set; } = name;
    public TokenName Op { get; set; } = op;
    public ASTNode? Initializer { get; set; } = initializer;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        Print($"Constant Assignment: `{ASTTracer.Unmangle(Name)}`");
        ASTTracer.PrintDepth(1 + depth);
        Print("Initializer:");
        Initializer?.Print(2 + depth);
    }

    public override ASTNode Clone() => new ConstAssignmentNode(Name, Op, Initializer?.Clone())
    {
        Token = Token
    };
}