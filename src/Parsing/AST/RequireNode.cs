namespace kiwi.Parsing.AST;

public class RequireNode(string packageName, bool satisified = false) : ASTNode(ASTNodeType.Require)
{
    public string PackageName { get; } = packageName;
    public bool Satisfied { get; set; } = satisified;

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();
        ASTTracer.PrintDepth(1 + depth);
        Print("Package: " + PackageName);
    }

    public override ASTNode Clone() => new RequireNode(PackageName)
    {
        Token = Token
    };
}