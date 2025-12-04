using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Operation;

public struct LogicalOp
{
    public static Value And(ref Value left, ref Value right, bool doAssign = false)
    {
        var res = BooleanOp.IsTruthy(left) && BooleanOp.IsTruthy(right);

        if (doAssign)
        {
            left.SetValue(res);
            return left;
        }
        
        return Value.CreateBoolean(res);
    }

    public static Value Or(ref Value left, ref Value right, bool doAssign = false)
    {
        var res = BooleanOp.IsTruthy(left) || BooleanOp.IsTruthy(right);

        if (doAssign)
        {
            left.SetValue(res);
            return left;
        }
        
        return Value.CreateBoolean(res);
    }

    public static Value Not(ref Value right)
    {
        if (right.IsBoolean())
        {
            return Value.CreateBoolean(!right.GetBoolean());
        }
        else if (right.IsNull())
        {
            return Value.True;
        }
        else if (right.IsInteger())
        {
            return Value.CreateBoolean(right.GetInteger() == 0);
        }
        else if (right.IsFloat())
        {
            return Value.CreateBoolean(right.GetFloat() == 0.0);
        }
        else if (right.IsString())
        {
            return Value.CreateBoolean(string.IsNullOrEmpty(right.GetString()));
        }
        else if (right.IsDate())
        {
            return Value.CreateBoolean(right.GetDate() == default);
        }
        else if (right.IsList())
        {
            return Value.CreateBoolean(right.GetList().Count == 0);
        }
        else if (right.IsHashmap())
        {
            return Value.CreateBoolean(right.GetHashmap().Count == 0);
        }
        else
        {
            return Value.False;  // Object, Lambda, etc.
        }
    }
}