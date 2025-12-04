using kiwi.Parsing;

namespace kiwi.Tracing.Error;

public class NullObjectError(Token t, string message = "Object context is null.")
    : KiwiError(t, "NullObjectError", message)
{
}