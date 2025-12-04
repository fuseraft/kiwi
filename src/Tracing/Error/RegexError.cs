using kiwi.Parsing;

namespace kiwi.Tracing.Error;

public class RegexError(Token t, string message = "A regular expression error occurred.")
    : KiwiError(t, "Regex", message)
{
}