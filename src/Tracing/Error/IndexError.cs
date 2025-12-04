using kiwi.Parsing;

namespace kiwi.Tracing.Error;

public class IndexError(Token t, string message = "Invalid index.")
    : KiwiError(t, "IndexError", message)
{
}