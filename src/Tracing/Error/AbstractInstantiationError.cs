using kiwi.Parsing;

namespace kiwi.Tracing.Error;

public class AbstractInstantiationError(Token t, string structName)
    : KiwiError(t, "AbstractInstantiationError", $"Cannot instantiate abstract struct `{structName}`.")
{
}
