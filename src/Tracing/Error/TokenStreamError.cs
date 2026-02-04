using kiwi.Parsing;

namespace kiwi.Tracing.Error;

public class TokenStreamError(Token t, string message = "A token stream error occurred.")
    : KiwiError(t, "TokenStreamError", message)
{
}

public class UnexpectedEndOfFileError(Token t, string message = "Unexpected end of file.")
    : KiwiError(t, "UnexpectedEndOfFileError", message)
{
}