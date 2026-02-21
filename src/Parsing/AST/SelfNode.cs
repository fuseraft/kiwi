namespace kiwi.Parsing.AST;

public class SelfNode(string name = "") : ASTNode(ASTNodeType.Self)
{
    public string Name { get; } = name;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();

        if (!string.IsNullOrEmpty(Name))
        {
            ASTTracer.PrintDepth(1 + depth);
            Print($"Name: `{ASTTracer.Unmangle(Name)}`");
        }
    }

    public override ASTNode Clone() => new SelfNode(Name)
    {
        Token = Token
    };
}