using kiwi.Parsing;

namespace kiwi.Tracing.Error;

public class HashKeyError(Token t, string key)
    : KiwiError(t, "HashKeyError", $"Invalid hashmap key: '{key}'")
{
}