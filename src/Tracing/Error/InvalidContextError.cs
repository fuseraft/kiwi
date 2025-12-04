using kiwi.Parsing;

namespace kiwi.Tracing.Error;

public class InvalidContextError(Token t, string message = "Invalid object context.")
    : KiwiError(t, "InvalidContextError", message)
{
}