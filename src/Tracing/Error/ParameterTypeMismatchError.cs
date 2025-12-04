using kiwi.Parsing;
using Value = kiwi.Typing.Value;
using ValueType = kiwi.Typing.ValueType;

namespace kiwi.Tracing.Error;

public class ParameterTypeMismatchError : KiwiError
{
    public ParameterTypeMismatchError(Token t, string name, int position, ValueType expectedType, ValueType actualType)
        : base(t, "ParameterTypeMismatchError", $"The type for parameter {position} of `{name}` is not an expected type. Expected {expectedType} but received {actualType}.")
    {
    }

    public static void ExpectBoolean(Token token, string name, int position, Value value)
    {
        if (!value.IsBoolean())
        {
            throw new ParameterTypeMismatchError(token, name, position, ValueType.Boolean, value.Type);
        }
    }

    public static void ExpectHashmap(Token token, string name, int position, Value value)
    {
        if (!value.IsHashmap())
        {
            throw new ParameterTypeMismatchError(token, name, position, ValueType.Hashmap, value.Type);
        }
    }

    public static void ExpectDate(Token token, string name, int position, Value value)
    {
        if (!value.IsDate())
        {
            throw new ParameterTypeMismatchError(token, name, position, ValueType.Date, value.Type);
        }
    }

    public static void ExpectNumber(Token token, string name, int position, Value value)
    {
        if (!value.IsNumber())
        {
            throw new ParameterTypeMismatchError(token, name, position, ValueType.Number, value.Type);
        }
    }

    public static void ExpectInteger(Token token, string name, int position, Value value)
    {
        if (!value.IsInteger())
        {
            throw new ParameterTypeMismatchError(token, name, position, ValueType.Integer, value.Type);
        }
    }

    public static void ExpectFloat(Token token, string name, int position, Value value)
    {
        if (!value.IsFloat())
        {
            throw new ParameterTypeMismatchError(token, name, position, ValueType.Float, value.Type);
        }
    }

    public static void ExpectList(Token token, string name, int position, Value value)
    {
        if (!value.IsList())
        {
            throw new ParameterTypeMismatchError(token, name, position, ValueType.List, value.Type);
        }
    }

    public static void ExpectString(Token token, string name, int position, Value value)
    {
        if (!value.IsString())
        {
            throw new ParameterTypeMismatchError(token, name, position, ValueType.String, value.Type);
        }
    }
}