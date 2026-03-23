using kiwi.Parsing;

namespace kiwi.Tracing.Error;

public class SafeModeError(Token t, string feature)
    : KiwiError(t, "SafeModeError", $"'{feature}' is not available in safe mode.")
{
}
