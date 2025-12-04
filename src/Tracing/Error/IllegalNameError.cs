using kiwi.Parsing;

namespace kiwi.Tracing.Error;

public class IllegalNameError(Token t, string name)
    : KiwiError(t, "IllegalNameError", $"The following name cannot be declared: '{name}'")
{
}