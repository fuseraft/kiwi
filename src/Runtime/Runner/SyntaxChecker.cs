using kiwi.Parsing;
using kiwi.Tracing;
using kiwi.Tracing.Error;

namespace kiwi.Runtime.Runner;

/// <summary>
/// Parses a script (including the standard library) and reports all syntax
/// errors without executing the program.
/// </summary>
public class SyntaxChecker : ScriptRunner
{
    public SyntaxChecker() { }

    public override int Run(string script, List<string> args)
    {
        try
        {
            using Lexer lexer = new(script);

            Parser parser = new();
            List<TokenStream> streams = [];
            LoadStandardLibrary(ref streams);
            streams.Add(lexer.GetTokenStream());
            parser.ParseTokenStreamCollection(streams);

            if (parser.HasError)
            {
                return 1;
            }

            Console.WriteLine($"No syntax errors found in: {script}");
        }
        catch (KiwiError e)
        {
            ErrorHandler.PrintError(e);
            return 1;
        }
        catch (Exception e)
        {
            ErrorHandler.DumpCrashLog(e);
            return 1;
        }

        return 0;
    }
}
