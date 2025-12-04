using kiwi.Parsing;

namespace kiwi.Tracing.Error;

public class FunctionUndefinedError(Token t, string name)
    : KiwiError(t, "FunctionUndefinedError", $"Function `{name}` is undefined.")
{
}