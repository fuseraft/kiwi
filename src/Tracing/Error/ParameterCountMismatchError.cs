using kiwi.Parsing;

namespace kiwi.Tracing.Error;

public class ParameterCountMismatchError : KiwiError
{
    public ParameterCountMismatchError(Token t, string name)
        : base(t, "ParameterCountMismatchError", $"The parameter count for function `{name}` does not match parameters passed.")
    {
    }

    public ParameterCountMismatchError(Token t, string name, int expectedCount)
        : base(t, "ParameterCountMismatchError", $"The parameter count for function `{name}` does not match parameters passed. Expected {expectedCount}.")
    {
    }

    public ParameterCountMismatchError(Token t, string name, int expectedCount, int actualCount)
        : base(t, "ParameterCountMismatchError", $"The parameter count for function `{name}` does not match parameters passed. Expected {expectedCount} but received {actualCount}.")
    {
    }

    public ParameterCountMismatchError(Token t, string name, int actualCount, List<int> expectedCount)
        : base(t, "ParameterCountMismatchError", $"The parameter count for function `{name}` does not match parameters passed. Expected {string.Join(" or ", expectedCount)} but received {actualCount}.")
    {
    }

    public static void Check(Token token, string name, int expectedCount, int actualCount)
    {
        if (expectedCount != actualCount)
        {
            throw new ParameterCountMismatchError(token, name, expectedCount, actualCount);
        }
    }

    public static void CheckRange(Token token, string name, int minCount, int maxCount, int actualCount)
    {
        if (actualCount < minCount || actualCount > maxCount)
        {
            throw new ParameterCountMismatchError(token, $"The parameter count for function `{name}` does not match parameters passed. Expected within {minCount} and {maxCount} but received {actualCount}.");
        }
    }
}
