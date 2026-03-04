namespace kiwi.Parsing.AST;

public class InterpolationNode() : ASTNode(ASTNodeType.Interpolation)
{
    public List<ASTNode> Parts { get; } = [];

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        Print("Interpolation:");
        foreach (var part in Parts)
        {
            part.Print(depth + 1);
        }
    }

    public override ASTNode Clone()
    {
        var clone = new InterpolationNode { Token = Token };
        foreach (var part in Parts)
        {
            clone.Parts.Add(part.Clone());
        }
        return clone;
    }
}
