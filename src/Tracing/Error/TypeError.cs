using kiwi.Parsing;
using kiwi.Typing;

namespace kiwi.Tracing.Error;

public class TypeError(Token t, string message = "Unexpected type.")
    : KiwiError(t, "TypeError", message)
{
    public static void ExpectDate(Token token, Value value)
    {
        if (!value.IsDate())
        {
            throw new TypeError(token, $"Expected a date but instead received `{value.Type}`.");
        }
    }

    public static void ExpectBoolean(Token token, Value value)
    {
        if (!value.IsBoolean())
        {
            throw new TypeError(token, $"Expected a boolean but instead received `{value.Type}`.");
        }
    }

    public static void ExpectHashmap(Token token, Value value)
    {
        if (!value.IsHashmap())
        {
            throw new TypeError(token, $"Expected a hashmap but instead received `{value.Type}`");
        }
    }

    public static void ExpectInteger(Token token, Value value)
    {
        if (!value.IsInteger())
        {
            throw new TypeError(token, $"Expected an integer but instead received `{value.Type}`.");
        }
    }

    public static void ExpectList(Token token, Value value)
    {
        if (!value.IsList())
        {
            throw new TypeError(token, $"Expected a list but instead received `{value.Type}`.");
        }
    }

    public static void ExpectString(Token token, Value value)
    {
        if (!value.IsString())
        {
            throw new TypeError(token, $"Expected a string but instead received `{value.Type}`.");
        }
    }
}