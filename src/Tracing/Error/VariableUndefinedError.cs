using kiwi.Parsing;

namespace kiwi.Tracing.Error;

public class VariableUndefinedError(Token t, string name)
    : KiwiError(t, "VariableUndefinedError", $"The variable has not been declared: '{name}'")
{
}