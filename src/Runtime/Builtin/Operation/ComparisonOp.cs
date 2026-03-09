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
            return right.GetList().Any(item => BooleanOp.IsSame(leftValue, item)) ? Value.True : Value.False;
        }
        if (right.IsString())
        {
            if (!left.IsString())
            {
                throw new ConversionError(token, "Left operand must be a string when right operand is a string for 'in' operator.");
            }
            return right.GetString().Contains(left.GetString()) ? Value.True : Value.False;
        }
        if (right.IsHashmap())
        {
            return right.GetHashmap().ContainsKey(left) ? Value.True : Value.False;
        }
        if (right.IsBytes())
        {
            if (!left.IsInteger() || left.GetInteger() < 0 || left.GetInteger() > 255)
            {
                throw new ConversionError(token, "Left operand must be an integer between 0 and 255 when right operand is bytes for 'in' operator.");
            }
            return Array.IndexOf(right.GetBytes(), (byte)left.GetInteger()) >= 0 ? Value.True : Value.False;
        }
        throw new InvalidOperationError(token, "The 'in' operator is not supported for the provided types.");
    }

    public static bool Equal(ref Value v1, ref Value v2) => BooleanOp.IsSame(v1, v2);

    public static Value NotEqual(ref Value left, ref Value right) => BooleanOp.IsSame(left, right) ? Value.False : Value.True;

    public static Value NullCoalesce(ref Value left, ref Value right) => left.IsNull() ? right : left;

    public static Value LessThan(ref Value left, ref Value right) => GetLtResult(ref left, ref right) ? Value.True : Value.False;

    public static Value LessThanOrEqual(ref Value left, ref Value right) => GetLteResult(ref left, ref right) ? Value.True : Value.False;

    public static Value GreaterThan(ref Value left, ref Value right) => GetGtResult(ref left, ref right) ? Value.True : Value.False;

    public static Value GreaterThanOrEqual(ref Value left, ref Value right) => GetGteResult(ref left, ref right) ? Value.True : Value.False;

    private static bool IsNumeric(Value v) => v.IsInteger() || v.IsFloat();
    
    private static double ToDouble(Value v) => v.IsFloat() ? v.GetFloat() : (double)v.GetInteger();

    public static bool GetLtResult(ref Value lhs, ref Value rhs)
    {
        if (lhs.Type != rhs.Type)
        {
            if (IsNumeric(lhs) && IsNumeric(rhs))
            {
                return ToDouble(lhs) < ToDouble(rhs);
            }
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
            if (IsNumeric(lhs) && IsNumeric(rhs))
            {
                return ToDouble(lhs) <= ToDouble(rhs);
            }
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
            if (IsNumeric(lhs) && IsNumeric(rhs))
            {
                return ToDouble(lhs) > ToDouble(rhs);
            }
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
            if (IsNumeric(lhs) && IsNumeric(rhs))
            {
                return ToDouble(lhs) >= ToDouble(rhs);
            }
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