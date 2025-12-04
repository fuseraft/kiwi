using kiwi.Parsing;

namespace kiwi.Tracing.Error;

public class DivideByZeroError(Token t, string message = "Division by zero.")
    : KiwiError(t, "DivideByZeroError", message)
{
}