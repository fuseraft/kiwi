using kiwi.Typing;
using kiwi.Parsing;
using kiwi.Tracing.Error;

namespace kiwi.Runtime.Builtin.Operation;

public struct ComparisonOp
{
    public static Value In(Token token, ref Value left, ref Value right)
    {
        if (right.IsList())
        {
            var leftValue = left;
            return Value.CreateBoolean(right.GetList().Any(item => BooleanOp.IsSame(leftValue, item)));
        }
        if (right.IsString())
        {
            if (!left.IsString())
            {
                throw new ConversionError(token, "Left operand must be a string when right operand is a string for 'in' operator.");
            }
            return Value.CreateBoolean(right.GetString().Contains(left.GetString()));
        }
        if (right.IsHashmap())
        {
            return Value.CreateBoolean(right.GetHashmap().ContainsKey(left));
        }
        if (right.IsBytes())
        {
            if (!left.IsInteger() || left.GetInteger() < 0 || left.GetInteger() > 255)
            {
                throw new ConversionError(token, "Left operand must be an integer between 0 and 255 when right operand is bytes for 'in' operator.");
            }
            return Value.CreateBoolean(Array.IndexOf(right.GetBytes(), (byte)left.GetInteger()) >= 0);
        }
        throw new InvalidOperationError(token, "The 'in' operator is not supported for the provided types.");
    }

    public static bool Equal(ref Value v1, ref Value v2) => BooleanOp.IsSame(v1, v2);

    public static Value NotEqual(ref Value left, ref Value right) => Value.CreateBoolean(!BooleanOp.IsSame(left, right));

    public static Value NullCoalesce(ref Value left, ref Value right) => left.IsNull() ? right : left;

    public static Value LessThan(ref Value left, ref Value right) => Value.CreateBoolean(GetLtResult(ref left, ref right));

    public static Value LessThanOrEqual(ref Value left, ref Value right) => Value.CreateBoolean(GetLteResult(ref left, ref right));

    public static Value GreaterThan(ref Value left, ref Value right) => Value.CreateBoolean(GetGtResult(ref left, ref right));

    public static Value GreaterThanOrEqual(ref Value left, ref Value right) => Value.CreateBoolean(GetGteResult(ref left, ref right));

    public static bool GetLtResult(ref Value lhs, ref Value rhs)
    {
        if (lhs.Type != rhs.Type)
        {
            return lhs.Type < rhs.Type;
        }

        return lhs.Type switch
        {
            Typing.ValueType.Integer => lhs.GetInteger() < rhs.GetInteger(),
            Typing.ValueType.Float => lhs.GetFloat() < rhs.GetFloat(),
            Typing.ValueType.Boolean => lhs.GetBoolean().CompareTo(rhs.GetBoolean()) < 0,
            Typing.ValueType.String => lhs.GetString().CompareTo(rhs.GetString()) < 0,
            Typing.ValueType.Date => lhs.GetDate().CompareTo(rhs.GetDate()) < 0,
            Typing.ValueType.List => lhs.GetList().Count < rhs.GetList().Count,
            Typing.ValueType.Hashmap => lhs.GetHashmap().Count < rhs.GetHashmap().Count,
            Typing.ValueType.None => !rhs.IsNull(),
            _ => false,
        };
    }

    private static bool GetLteResult(ref Value lhs, ref Value rhs)
    {
        if (lhs.Type != rhs.Type)
        {
            return lhs.Type <= rhs.Type;
        }

        return lhs.Type switch
        {
            Typing.ValueType.Integer => lhs.GetInteger() <= rhs.GetInteger(),
            Typing.ValueType.Float => lhs.GetFloat() <= rhs.GetFloat(),
            Typing.ValueType.Boolean => lhs.GetBoolean().CompareTo(rhs.GetBoolean()) <= 0,
            Typing.ValueType.String => lhs.GetString().CompareTo(rhs.GetString()) <= 0,
            Typing.ValueType.Date => lhs.GetDate().CompareTo(rhs.GetDate()) <= 0,
            Typing.ValueType.List => lhs.GetList().Count <= rhs.GetList().Count,
            Typing.ValueType.Hashmap => lhs.GetHashmap().Count <= rhs.GetHashmap().Count,
            Typing.ValueType.None => rhs.IsNull(),
            _ => false,
        };
    }

    public static bool GetGtResult(ref Value lhs, ref Value rhs)
    {
        if (lhs.Type != rhs.Type)
        {
            return lhs.Type > rhs.Type;
        }

        return lhs.Type switch
        {
            Typing.ValueType.Integer => lhs.GetInteger() > rhs.GetInteger(),
            Typing.ValueType.Float => lhs.GetFloat() > rhs.GetFloat(),
            Typing.ValueType.Boolean => lhs.GetBoolean().CompareTo(rhs.GetBoolean()) > 0,
            Typing.ValueType.String => lhs.GetString().CompareTo(rhs.GetString()) > 0,
            Typing.ValueType.Date => lhs.GetDate().CompareTo(rhs.GetDate()) > 0,
            Typing.ValueType.List => lhs.GetList().Count > rhs.GetList().Count,
            Typing.ValueType.Hashmap => lhs.GetHashmap().Count > rhs.GetHashmap().Count,
            Typing.ValueType.None => !rhs.IsNull(),
            _ => false,
        };
    }

    private static bool GetGteResult(ref Value lhs, ref Value rhs)
    {
        if (lhs.Type != rhs.Type)
        {
            return lhs.Type >= rhs.Type;
        }

        return lhs.Type switch
        {
            Typing.ValueType.Integer => lhs.GetInteger() >= rhs.GetInteger(),
            Typing.ValueType.Float => lhs.GetFloat() >= rhs.GetFloat(),
            Typing.ValueType.Boolean => lhs.GetBoolean().CompareTo(rhs.GetBoolean()) >= 0,
            Typing.ValueType.String => lhs.GetString().CompareTo(rhs.GetString()) >= 0,
            Typing.ValueType.Date => lhs.GetDate().CompareTo(rhs.GetDate()) >= 0,
            Typing.ValueType.List => lhs.GetList().Count >= rhs.GetList().Count,
            Typing.ValueType.Hashmap => lhs.GetHashmap().Count >= rhs.GetHashmap().Count,
            Typing.ValueType.None => rhs.IsNull(),
            _ => false,
        };
    }
}