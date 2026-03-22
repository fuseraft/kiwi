using kiwi.Parsing;
using kiwi.Tracing;
using kiwi.Tracing.Error;

namespace kiwi.Runtime.Runner;

/// <summary>
/// Runs a string of code passed via <c>-e</c>/<c>--execute</c>.
/// </summary>
public class CodeRunner(string code, Interpreter interpreter) : VMScriptRunner(interpreter)
{
    public override int Run(string script, List<string> args)
    {
        try
        {
            using var lexer = new Lexer(-1, code);
            return RunVMLexer(lexer);
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
    }
}
