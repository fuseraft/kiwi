using kiwi.Typing;

namespace kiwi.Parsing.AST;

public class VariableNode() : ASTNode(ASTNodeType.Variable)
{
    public List<KeyValuePair<string, ASTNode?>> Variables { get; set; } = [];
    public Dictionary<string, int> TypeHints { get; set; } = [];

    public override void Print(int depth = 0)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();

        ASTTracer.PrintDepth(1 + depth);
        Print("Variables: ");

        foreach (var v in Variables)
        {
            ASTTracer.PrintDepth(2 + depth);

            Print($"Identifier: `{ASTTracer.Unmangle(v.Key)}`");

            if (TypeHints.TryGetValue(v.Key, out int value))
            {
                ASTTracer.PrintDepth(2 + depth);
                var typeHint = value;
                Print($"Type: {TypeRegistry.GetTypeName(typeHint)}");
            }

            if (v.Value == null)
            {
                continue;
            }

            ASTTracer.PrintDepth(2 + depth);
            Print("Default: ");
            v.Value.Print(1 + depth);
        }
    }

    public override ASTNode Clone()
    {
        List<KeyValuePair<string, ASTNode?>> clonedVariables = [];

        foreach (var v in Variables)
        {
            KeyValuePair<string, ASTNode?> item = new(v.Key, v.Value?.Clone());
            clonedVariables.Add(item);
        }

        return new VariableNode
        {
            Variables = clonedVariables,
            TypeHints = TypeHints,
            Token = Token
        };
    }
}