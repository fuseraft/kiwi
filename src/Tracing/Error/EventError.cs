using kiwi.Parsing;

namespace kiwi.Tracing.Error;

public class EventError(Token t, string message = "An event error occurred.")
    : KiwiError(t, "EventError", message)
{
}