namespace kiwi.Parsing.AST;

public class ConstAssignmentNode : ASTNode
{
    public ConstAssignmentNode() : base(ASTNodeType.ConstAssignment) { }
    public ConstAssignmentNode(string name, TokenName op,
                        ASTNode? initializer)
        : base(ASTNodeType.ConstAssignment)
    {
        Name = name;
        Op = op;
        Initializer = initializer;
    }

    public string Name { get; set; } = string.Empty;
    public TokenName Op { get; set; } = TokenName.Default;
    public ASTNode? Initializer { get; set; }

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        Console.WriteLine($"Constant Assignment: `{ASTTracer.Unmangle(Name)}`");
        ASTTracer.PrintDepth(1 + depth);
        Console.WriteLine("Initializer:");
        Initializer?.Print(2 + depth);
    }

    public override ASTNode Clone() => new ConstAssignmentNode(Name, Op, Initializer?.Clone())
    {
        Token = Token
    };
}