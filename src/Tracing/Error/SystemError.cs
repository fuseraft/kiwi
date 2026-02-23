using kiwi.Parsing;

namespace kiwi.Tracing.Error;

public class SystemError(Token t, string message = "A system error occurred.")
    : KiwiError(t, "SystemError", message)
{
}