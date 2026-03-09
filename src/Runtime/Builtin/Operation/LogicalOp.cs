using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Operation;

public struct LogicalOp
{
    public static Value And(ref Value left, ref Value right, bool doAssign = false)
    {
        var res = BooleanOp.IsTruthy(left) && BooleanOp.IsTruthy(right);

        if (doAssign)
        {
            // Guard: never mutate the global singletons Value.True/False/Default.
            // The caller always stores the return value back anyway (see Interpreter.cs scope.Assign calls).
            if (!ReferenceEquals(left, Value.True) && !ReferenceEquals(left, Value.False))
                left.SetValue(res);
            return res ? Value.True : Value.False;
        }

        return res ? Value.True : Value.False;
    }

    public static Value Or(ref Value left, ref Value right, bool doAssign = false)
    {
        var res = BooleanOp.IsTruthy(left) || BooleanOp.IsTruthy(right);

        if (doAssign)
        {
            if (!ReferenceEquals(left, Value.True) && !ReferenceEquals(left, Value.False))
                left.SetValue(res);
            return res ? Value.True : Value.False;
        }

        return res ? Value.True : Value.False;
    }

    public static Value Not(ref Value right)
    {
        if (right.IsBoolean())
        {
            return right.GetBoolean() ? Value.False : Value.True;
        }
        else if (right.IsNull())
        {
            return Value.True;
        }
        else if (right.IsInteger())
        {
            return right.GetInteger() == 0 ? Value.True : Value.False;
        }
        else if (right.IsFloat())
        {
            return right.GetFloat() == 0.0 ? Value.True : Value.False;
        }
        else if (right.IsString())
        {
            return string.IsNullOrEmpty(right.GetString()) ? Value.True : Value.False;
        }
        else if (right.IsDate())
        {
            return right.GetDate() == default ? Value.True : Value.False;
        }
        else if (right.IsList())
        {
            return right.GetList().Count == 0 ? Value.True : Value.False;
        }
        else if (right.IsHashmap())
        {
            return right.GetHashmap().Count == 0 ? Value.True : Value.False;
        }
        else if (right.IsBytes())
        {
            return right.GetBytes().Length == 0 ? Value.True : Value.False;
        }
        else
        {
            return Value.False;  // Object, Lambda, etc.
        }
    }
}