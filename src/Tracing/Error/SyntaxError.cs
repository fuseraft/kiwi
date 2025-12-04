using kiwi.Parsing;

namespace kiwi.Tracing.Error;

public class SyntaxError(Token t, string message = "A syntax error occurred.")
    : KiwiError(t, "SyntaxError", message)
{
}