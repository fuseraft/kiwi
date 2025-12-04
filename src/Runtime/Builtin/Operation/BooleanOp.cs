using kiwi.Parsing;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Operation;

public struct BooleanOp
{
    public static bool IsTruthy(Value value)
    {
        return value.Type switch
        {
            Typing.ValueType.Integer => value.GetInteger() != 0L,
            Typing.ValueType.Float => value.GetFloat() != 0D,
            Typing.ValueType.Boolean => value.GetBoolean(),
            Typing.ValueType.String => !string.IsNullOrEmpty(value.GetString()),
            Typing.ValueType.Date => value.GetDate() != default,
            Typing.ValueType.List => value.GetList().Count > 0,
            Typing.ValueType.Hashmap => value.GetHashmap().Count > 0,
            Typing.ValueType.Object => true,
            Typing.ValueType.Lambda => true,
            Typing.ValueType.None => false,
            _ => false,
        };
    }

    public static bool IsSame(Value v1, Value v2)
    {
        if (v1.Type != v2.Type)
        {
            return false;
        }

        return v1.Type switch
        {
            Typing.ValueType.Integer => v2.IsInteger() && v1.GetInteger() == v2.GetInteger(),
            Typing.ValueType.Float => v2.IsFloat() && v1.GetFloat() == v2.GetFloat(),
            Typing.ValueType.Boolean => v2.IsBoolean() && v1.GetBoolean() == v2.GetBoolean(),
            Typing.ValueType.String => v2.IsString() && v1.GetString() == v2.GetString(),
            Typing.ValueType.Date => v2.IsDate() && v1.GetDate() == v2.GetDate(),
            Typing.ValueType.Hashmap => v2.IsHashmap() && v1.GetHashmap() == v2.GetHashmap(),
            Typing.ValueType.List => v2.IsList() && v1.GetList() == v2.GetList(),
            Typing.ValueType.None => v2.IsNull(),
            _ => false,
        };
    }

    public static bool IsZero(Token token, ref Value v)
    {
        if (v.IsFloat())
        {
            return v.GetFloat() == 0D;
        }
        else if (v.IsInteger())
        {
            return v.GetInteger() == 0L;
        }

        throw new ConversionError(token, "Cannot check non-numeric value for zero value.");
    }
}