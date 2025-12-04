using kiwi.Parsing;

namespace kiwi.Tracing.Error;

public class RangeError(Token t, string message = "Invalid range.")
    : KiwiError(t, "RangeError", message)
{
}