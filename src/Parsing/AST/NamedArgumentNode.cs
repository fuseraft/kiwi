namespace kiwi.Parsing.AST;

public class NamedArgumentNode(string name, ASTNode? value) : ASTNode(ASTNodeType.NamedArgument)
{
    public string Name { get; set; } = name;
    public ASTNode? Value { get; set; } = value;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        Print($"NamedArgument: `{Name}`");
        Value?.Print(1 + depth);
    }

    public override ASTNode Clone() =>
        new NamedArgumentNode(Name, Value?.Clone()) { Token = Token };
}
