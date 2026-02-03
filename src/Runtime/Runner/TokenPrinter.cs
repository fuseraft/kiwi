using kiwi.Parsing;

namespace kiwi.Runtime.Runner;
public class TokenPrinter : IRunner
{
    /// <summary>
    /// A success return code. A placeholder until a smarter mechanism is implemented.
    /// </summary>
    private const int SuccessReturnCode = 0;

    /// <summary>
    /// Prints the tokens of a given script.
    /// </summary>
    /// <param name="script">The script.</param>
    /// <param name="args">The arguments.</param>
    /// <returns>Returns <c>0</c> for now.</returns>
    public int Run(string script, List<string> args)
    {
        Console.WriteLine($"Tokenizing: {script}\n");
        Console.WriteLine($"{"Line #",-7} {"Token #",-7} {"Type",12} {"Name",-20} {"Text",-20}");
        Console.WriteLine($"{"------",-7} {"-------",-7} {"----",12} {"----",-20} {"----",-20}");

        using Lexer lexer = new(script);
        var stream = lexer.GetTokenStream();
        var counter = 0;

        while (stream.CanRead)
        {
            var token = stream.Current();

            Console.WriteLine($"{token.Span.Line, -7} {++counter,-7} {token.Type,12} {token.Name,-20} {token.Text.Trim(),-20}");

            stream.Next();
        }

        Console.WriteLine();

        return SuccessReturnCode;
    }
}