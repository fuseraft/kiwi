namespace kiwi.Parsing.AST;

public class EnumNode(string name, List<(string Name, ASTNode? Value)> members) : ASTNode(ASTNodeType.Enum)
{
    public string Name { get; } = name;
    public List<(string Name, ASTNode? Value)> Members { get; } = members;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();

        ASTTracer.PrintDepth(1 + depth);
        Print($"Name: {Name}");

        foreach (var (memberName, value) in Members)
        {
            ASTTracer.PrintDepth(2 + depth);
            Print($"Member: {memberName}");
            value?.Print(3 + depth);
        }
    }

    public override ASTNode Clone()
    {
        List<(string Name, ASTNode? Value)> clonedMembers = [];
        foreach (var (memberName, value) in Members)
        {
            clonedMembers.Add((memberName, value?.Clone()));
        }

        return new EnumNode(Name, clonedMembers)
        {
            Token = Token
        };
    }
}
