namespace kiwi.Parsing.AST;

/// <summary>
/// An AST node.<br />
/// <br />
/// Each node in the tree has:<br />
///     - A token from the token stream used for error and stack tracing.<br />
///     - A type for the interpreter to understand.<br />
///     - A <c>Print</c> method for printing the node.<br />
///     - A <c>Clone</c> method for creating a deep copy of the node.<br />
/// </summary>
/// <param name="type">The AST node type.</param>
public class ASTNode(ASTNodeType type)
{
    /// <summary>
    /// In overridden classes, this method prints the AST node in a hierarchical tree format.
    /// </summary>
    /// <param name="depth">The number of spaces to the left of the text.</param>
    public virtual void Print(int depth = 0)
    {
        ASTTracer.PrintDepth(depth);
        PrintASTNodeType();
    }

    /// <summary>
    /// Gets or sets the token.
    /// </summary>
    public Token Token { get; set; }

    /// <summary>
    /// Gets or sets the type.
    /// </summary>
    public ASTNodeType Type { get; } = type;

    /// <summary>
    /// Prints the AST node type.
    /// </summary>
    public void PrintASTNodeType()
    {
        Print($"{Enum.GetName(Type)}:");
    }

    public void Print(string s)
    {
        Console.WriteLine(s);
    }

    /// <summary>
    /// In overriden classes, this method creates and returns a deep copy of the node.
    /// </summary>
    /// <returns>An instance of <see cref="ASTNode"/>.</returns>
    public virtual ASTNode Clone() => new(Type)
    {
        Token = Token
    };
}

public class ASTTracer
{
    public static void PrintDepth(int depth)
    {
        for (int i = 0; i < depth; ++i)
        {
            Console.Write("  ");
        }
    }

    public static string Unmangle(string? name)
    {
        if (name?.Length > 10)
        {
            var mangler = name.Substring(0, 10);

            if (mangler.StartsWith('_') && mangler.EndsWith('_'))
            {
                return name.Replace(mangler, string.Empty);
            }
        }

        return name ?? string.Empty;
    }
}