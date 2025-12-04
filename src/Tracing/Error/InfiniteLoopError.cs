using kiwi.Parsing;

namespace kiwi.Tracing.Error;

public class InfiniteLoopError(Token t, string message = "An infinite loop occurred.")
    : KiwiError(t, "InfiniteLoopError", message)
{
}