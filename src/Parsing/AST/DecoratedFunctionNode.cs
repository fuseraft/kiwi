namespace kiwi.Parsing.AST;

// A function definition with one or more decorator expressions applied.
// Decorators are listed top to bottom; the bottommost is applied first.
//   @A
//   @B(arg)
//   fn f() ... end
//   => f = A(B(f, arg))
public class DecoratedFunctionNode() : ASTNode(ASTNodeType.DecoratedFunction)
{
    public FunctionNode Function { get; set; } = new();

    // Each entry is (decoratorExpr, extraArgs).
    // decoratorExpr is an AST node that evaluates to (or names) the decorator callable.
    // At runtime the decorated function is prepended as the first argument.
    public List<(ASTNode? Expr, List<ASTNode?> ExtraArgs)> Decorators { get; set; } = [];

    public override void Print(int depth = 0)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();

        ASTTracer.PrintDepth(1 + depth);
        Print("Decorators:");
        foreach (var (expr, args) in Decorators)
        {
            ASTTracer.PrintDepth(2 + depth);
            Print($"@<expr>" + (args.Count > 0 ? $"({args.Count} args)" : ""));
        }

        Function.Print(1 + depth);
    }

    public override ASTNode Clone()
    {
        var clonedDecorators = Decorators
            .Select(d => (d.Expr?.Clone(), d.ExtraArgs.Select(a => a?.Clone()).ToList() as List<ASTNode?>))
            .ToList();

        return new DecoratedFunctionNode
        {
            Function = (FunctionNode)Function.Clone(),
            Decorators = clonedDecorators,
            Token = Token
        };
    }
}
