using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Operation;

public struct ComparisonOp
{
    public static bool Equal(ref Value v1, ref Value v2) => BooleanOp.IsSame(v1, v2);

    public static Value NotEqual(ref Value left, ref Value right) => Value.CreateBoolean(!BooleanOp.IsSame(left, right));

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