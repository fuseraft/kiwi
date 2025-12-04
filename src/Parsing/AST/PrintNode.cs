namespace kiwi.Parsing.AST;

public class PrintNode : ASTNode
{
    public PrintNode() : base(ASTNodeType.Print) { }
    public PrintNode(ASTNode? expression, bool printNewline, bool printStdError)
        : base(ASTNodeType.Print)
    {
        Expression = expression;
        PrintNewline = printNewline;
        PrintStdError = printStdError;
    }

    public ASTNode? Expression { get; }
    public bool PrintNewline { get; }
    public bool PrintStdError { get; }

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        if (PrintStdError)
        {
            Console.WriteLine(PrintNewline ? "PrintErrorLine:" : "PrintError:");
        }
        else
        {
            Console.WriteLine(PrintNewline ? "PrintLine:" : "Print:");
        }

        Expression?.Print(1 + depth);
    }

    public override ASTNode Clone() => new PrintNode(Expression?.Clone(), PrintNewline, PrintStdError)
    {
        Token = Token
    };
}

public class PrintXyNode : ASTNode
{
    public PrintXyNode() : base(ASTNodeType.PrintXy) { }
    public PrintXyNode(ASTNode? expression, ASTNode? x, ASTNode? y)
        : base(ASTNodeType.PrintXy)
    {
        Expression = expression;
        X = x;
        Y = y;
    }

    public ASTNode? Expression { get; set; }
    public ASTNode? X { get; set; }
    public ASTNode? Y { get; set; }

    public override void Print(int depth)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();
        Expression?.Print(1 + depth);
        X?.Print(1 + depth);
        Y?.Print(1 + depth);
    }

    public override ASTNode Clone() => new PrintXyNode(Expression?.Clone(), X?.Clone(), Y?.Clone())
    {
        Token = Token
    };
}
