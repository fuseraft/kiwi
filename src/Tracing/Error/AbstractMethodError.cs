using kiwi.Parsing;

namespace kiwi.Tracing.Error;

public class AbstractMethodError(Token t, string structName, string methodName)
    : KiwiError(t, "AbstractMethodError", $"Struct `{structName}` must implement abstract method `{methodName}`.")
{
}
