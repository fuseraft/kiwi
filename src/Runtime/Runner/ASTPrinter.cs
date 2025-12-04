using kiwi.Parsing;

namespace kiwi.Runtime.Runner;
public class ASTPrinter : IRunner
{
    /// <summary>
    /// The success return code. A placeholder until a smarter mechanism is implemented.
    /// </summary>
    private const int SuccessReturnCode = 0;

    /// <summary>
    /// Prints the abstract syntax tree of a given script.
    /// </summary>
    /// <param name="script">The script.</param>
    /// <param name="args">The arguments.</param>
    /// <returns>Returns <c>0</c> for now.</returns>
    public int Run(string script, List<string> args)
    {
        using Lexer lexer = new(script);
        var stream = lexer.GetTokenStream();

        Console.WriteLine($"Generating AST: {script}\n");
        var ast = new Parser().ParseTokenStream(stream, isEntryPoint: true);
        ast.Print(0);

        Console.WriteLine();

        return SuccessReturnCode;
    }
}