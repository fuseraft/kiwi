using kiwi.Parsing;

namespace kiwi.Tracing.Error;

public class CallableError(Token t, string message = "Invalid callable.")
    : KiwiError(t, "CallableError", message)
{
}