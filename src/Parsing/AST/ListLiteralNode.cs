namespace kiwi.Parsing.AST;

public class ListLiteralNode(List<ASTNode> elements) : ASTNode(ASTNodeType.ListLiteral)
{
    public List<ASTNode> Elements { get; } = elements;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();

        foreach (var element in Elements)
        {
            element?.Print(1 + depth);
        }
    }

    public override ASTNode Clone()
    {
        List<ASTNode> clonedElements = [];

        foreach (var element in Elements)
        {
            clonedElements.Add(element.Clone());
        }

        return new ListLiteralNode(clonedElements)
        {
            Token = Token
        };
    }
}