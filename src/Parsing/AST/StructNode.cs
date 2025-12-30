namespace kiwi.Parsing.AST;

public class StructNode : ASTNode
{
    public StructNode() : base(ASTNodeType.Struct) { }
    public StructNode(string name, string baseStruct, List<string> interfaces, List<ASTNode?> methods)
        : base(ASTNodeType.Struct)
    {
        this.Name = name;
        this.BaseStruct = baseStruct;
        this.Interfaces.AddRange(interfaces);
        this.Methods.AddRange(methods);
    }

    public string Name { get; } = string.Empty;
    public string BaseStruct { get; } = string.Empty;
    public List<string> Interfaces { get; } = [];
    public List<ASTNode?> Methods { get; } = [];

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();

        ASTTracer.PrintDepth(1 + depth);
        Print($"Name: {Name}");

        if (!string.IsNullOrEmpty(BaseStruct))
        {
            ASTTracer.PrintDepth(1 + depth);
            Print($"Base: {BaseStruct}");
        }

        if (Interfaces.Count > 0)
        {
            ASTTracer.PrintDepth(1 + depth);
            Print("Interfaces:");
            foreach (var iface in Interfaces)
            {
                ASTTracer.PrintDepth(2 + depth);
                Print(iface);
            }
        }

        if (Methods.Count > 0)
        {
            ASTTracer.PrintDepth(1 + depth);
            Print("Methods:");
            foreach (var method in Methods)
            {
                method?.Print(2 + depth);
            }
        }
    }

    public override ASTNode Clone()
    {
        List<ASTNode?> clonedMethods = [];
        foreach (var method in Methods)
        {
            clonedMethods.Add(method?.Clone());
        }

        return new StructNode(Name, BaseStruct, Interfaces, clonedMethods)
        {
            Token = Token
        };
    }
}