namespace kiwi.Parsing.AST;

public class HashLiteralNode(Dictionary<ASTNode, ASTNode?> elements, List<string> keys) : ASTNode(ASTNodeType.HashLiteral)
{
    public Dictionary<ASTNode, ASTNode?> Elements { get; } = elements;
    public List<string> Keys { get; } = keys;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();
        foreach (var element in Elements)
        {
            ASTTracer.PrintDepth(1 + depth);
            Console.WriteLine("KVP:");

            ASTTracer.PrintDepth(2 + depth);
            Console.WriteLine("Key:");
            element.Key.Print(3 + depth);

            ASTTracer.PrintDepth(2 + depth);
            Console.WriteLine("Value: ");
            element.Value?.Print(3 + depth);
        }
    }

    public override ASTNode Clone()
    {
        Dictionary<ASTNode, ASTNode?> clonedElements = [];
        foreach (var (key, value) in Elements)
        {
            clonedElements.Add(key.Clone(), value?.Clone());
        }

        return new HashLiteralNode(clonedElements, Keys)
        {
            Token = Token
        };
    }
}