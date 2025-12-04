namespace kiwi.Parsing.AST;

public class ProgramNode : ASTNode
{
    /// <summary>
    /// Initializes a new program node.
    /// </summary>
    public ProgramNode()
        : base(ASTNodeType.Program) { }

    /// <summary>
    /// Initializes a new program node with statements.
    /// </summary>
    /// <param name="statements">The list of statements.</param>
    public ProgramNode(List<ASTNode> statements)
        : base(ASTNodeType.Program)
    {
        Statements.AddRange(statements);
    }

    /// <summary>
    /// Gets or sets the program statements.
    /// </summary>
    public List<ASTNode> Statements { get; set; } = [];

    /// <summary>
    /// Each file gets its own tree, but the <c>IsEntryPoint</c> flag tells us which tree is the entry point of the program.<br />
    /// <br />
    /// The <see cref="Parser"/> sets this value in the <c>ParseTokenStreamCollection</c> method.
    /// </summary>
    public bool IsEntryPoint { get; set; } = false;

    public override void Print(int depth = 0)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();
        foreach (var statement in Statements)
        {
            statement.Print(1 + depth);
        }
    }

    public override ASTNode Clone()
    {
        List<ASTNode> clonedStatements = [];

        foreach (var statement in Statements)
        {
            clonedStatements.Add(statement.Clone());
        }

        return new ProgramNode(clonedStatements)
        {
            Token = Token
        };
    }
}