using kiwi.Parsing;

namespace kiwi.Tracing.Error;

public class ValueError(Token t, string message = "Value error.")
    : KiwiError(t, "ValueError", message)
{
}