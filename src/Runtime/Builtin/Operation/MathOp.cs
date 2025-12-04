using kiwi.Parsing;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Operation;

public struct MathOp
{
    public static Value Add(Token token, ref Value left, ref Value right, bool doAssign = false)
    {
        Typing.ValueType type = Typing.ValueType.None;
        Value res = GetAddResult(token, ref left, ref right, ref type);

        if (doAssign)
        {
            left.Set(res, type);
            return left;
        }

        return res;
    }

    public static Value Sub(Token token, ref Value left, ref Value right, bool doAssign = false)
    {
        Typing.ValueType type = Typing.ValueType.None;
        Value res = GetSubResult(token, ref left, ref right, ref type);

        if (doAssign)
        {
            left.Set(res, type);
            return left;
        }

        return res;
    }

    public static Value Exp(Token token, ref Value left, ref Value right, bool doAssign = false)
    {
        Typing.ValueType type = Typing.ValueType.None;
        Value res = GetExpResult(token, ref left, ref right, ref type);

        if (doAssign)
        {
            left.Set(res, type);
            return left;
        }

        return res;
    }

    public static Value Mod(Token token, ref Value left, ref Value right, bool doAssign = false)
    {
        Typing.ValueType type = Typing.ValueType.None;
        Value res = GetModResult(token, ref left, ref right, ref type);

        if (doAssign)
        {
            left.Set(res, type);
            return left;
        }

        return res;
    }

    public static Value Div(Token token, ref Value left, ref Value right,
                       bool doAssign = false)
    {
        Typing.ValueType type = Typing.ValueType.None;
        Value res = GetDivResult(token, ref left, ref right, ref type);

        if (doAssign)
        {
            left.Set(res, type);
            return left;
        }

        return res;
    }

    public static Value Mul(Token token, ref Value left, ref Value right, bool doAssign = false)
    {
        Typing.ValueType type = Typing.ValueType.None;
        var res = GetMulResult(token, ref left, ref right, ref type);

        if (doAssign)
        {
            left.Set(res, type);
            return left;
        }

        return res;
    }

    public static Value Negate(Token token, ref Value right)
    {
        if (right.IsInteger())
        {
            return Value.CreateInteger(-right.GetInteger());
        }
        else if (right.IsFloat())
        {
            return Value.CreateFloat(-right.GetFloat());
        }
        else
        {
            throw new ConversionError(token, "Unary minus applied to a non-numeric value.");
        }
    }
    
    private static long GetNonZero(Token token, long value)
    {
        if (value == 0)
        {
            throw new DivideByZeroError(token);
        }

        return value;
    }

    private static double GetNonZero(Token token, double value)
    {
        if (value == 0.0)
        {
            throw new DivideByZeroError(token);
        }

        return value;
    }

    private static Value GetAddResult(Token token, ref Value left, ref Value right, ref Typing.ValueType type)
    {
        var leftIsInt = left.IsInteger();
        var rightIsInt = right.IsInteger();
        var leftIsFloat = left.IsFloat();
        var rightIsFloat = right.IsFloat();
        var leftIsString = left.IsString();
        var rightIsString = right.IsString();

        if (leftIsInt && rightIsInt)
        {
            type = Typing.ValueType.Integer;
            return new(left.GetInteger() + right.GetInteger(), type);
        }
        else if (leftIsFloat && rightIsFloat)
        {
            type = Typing.ValueType.Float;
            return new(left.GetFloat() + right.GetFloat());
        }
        else if ((leftIsInt && rightIsFloat) || (leftIsFloat && rightIsInt))
        {
            type = Typing.ValueType.Float;
            double l = leftIsInt ? left.GetInteger() : left.GetFloat();
            double r = rightIsInt ? right.GetInteger() : right.GetFloat();
            return new(l + r, type);
        }
        else if (leftIsString && rightIsString)
        {
            type = Typing.ValueType.String;
            return new(left.GetString() + right.GetString(), type);
        }
        else if (leftIsString)
        {
            type = Typing.ValueType.String;
            return new(left.GetString() + ConversionOp.ToString(right), type);
        }
        else if (left.IsList())
        {
            type = Typing.ValueType.List;
            var listCopy = left.GetList();

            if (right.IsList())
            {
                listCopy.AddRange(right.GetList());
            }
            else
            {
                listCopy.Add(right);
            }

            return new(listCopy, type);
        }
        else if (rightIsString)
        {
            type = Typing.ValueType.String;
            return new(ConversionOp.ToString(left) + right.GetString(), type);
        }

        throw new ConversionError(token, "Conversion error in addition.");
    }

    private static Value GetSubResult(Token token, ref Value left, ref Value right, ref Typing.ValueType type)
    {
        if (left.IsInteger() && right.IsInteger())
        {
            type = Typing.ValueType.Integer;
            return new(left.GetInteger() - right.GetInteger(), type);
        }
        else if (left.IsFloat() && right.IsFloat())
        {
            type = Typing.ValueType.Float;
            return new(left.GetFloat() - right.GetFloat(), type);
        }
        else if (left.IsInteger() && right.IsFloat())
        {
            type = Typing.ValueType.Float;
            return new(left.GetInteger() - right.GetFloat(), type);
        }
        else if (left.IsFloat() && right.IsInteger())
        {
            type = Typing.ValueType.Float;
            return new(left.GetFloat() - right.GetInteger(), type);
        }
        else if (left.IsList() && !right.IsList())
        {
            type = Typing.ValueType.List;
            List<Value> listValues = [];
            var leftList = left.GetList();
            var found = false;

            foreach (var item in leftList)
            {
                if (!found && BooleanOp.IsSame(item, right))
                {
                    found = true;
                    continue;
                }

                listValues.Add(item);
            }

            return new(listValues, type);
        }
        else if (left.IsList() && right.IsList())
        {
            type = Typing.ValueType.List;
            List<Value> listValues = [];
            var leftList = left.GetList();
            var rightList = right.GetList();

            foreach (var item in leftList)
            {
                bool found = false;

                foreach (var ritem in rightList)
                {
                    if (BooleanOp.IsSame(item, ritem))
                    {
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    listValues.Add(item);
                }
            }

            return new(listValues, type);
        }

        throw new ConversionError(token, "Conversion error in subtraction.");
    }

    private static Value GetExpResult(Token token, ref Value left, ref Value right, ref Typing.ValueType type)
    {
        if (left.IsInteger() && right.IsInteger())
        {
            type = Typing.ValueType.Integer;
            return new((long)Math.Pow(left.GetInteger(), right.GetInteger()), type);
        }
        else if (left.IsFloat() && right.IsFloat())
        {
            type = Typing.ValueType.Float;
            return new(Math.Pow(left.GetFloat(), right.GetFloat()), type);
        }
        else if (left.IsInteger() && right.IsFloat())
        {
            type = Typing.ValueType.Float;
            return new(Math.Pow(left.GetInteger(), right.GetFloat()), type);
        }
        else if (left.IsFloat() && right.IsInteger())
        {
            type = Typing.ValueType.Float;
            return new(Math.Pow(left.GetFloat(), right.GetInteger()), type);
        }

        throw new ConversionError(token, "Conversion error in exponentiation.");
    }

    private static Value GetModResult(Token token, ref Value left, ref Value right, ref Typing.ValueType type)
    {
        if (left.IsInteger() && right.IsInteger())
        {
            type = Typing.ValueType.Integer;
            var rhs = GetNonZero(token, right.GetInteger());
            return new(left.GetInteger() % rhs, type);
        }
        else if (left.IsFloat() && right.IsFloat())
        {
            type = Typing.ValueType.Float;
            var rhs = GetNonZero(token, right.GetFloat());
            return new(left.GetFloat() % rhs, type);
        }
        else if (left.IsInteger() && right.IsFloat())
        {
            type = Typing.ValueType.Float;
            var rhs = GetNonZero(token, right.GetFloat());
            return new(left.GetInteger() % rhs, type);
        }
        else if (left.IsFloat() && right.IsInteger())
        {
            type = Typing.ValueType.Float;
            var rhs = GetNonZero(token, (double)right.GetInteger());
            return new(left.GetFloat() % rhs, type);
        }

        throw new ConversionError(token, "Conversion error in modulus.");
    }

    private static Value GetDivResult(Token token, ref Value left, ref Value right, ref Typing.ValueType type)
    {
        if (left.IsInteger() && right.IsInteger())
        {
            type = Typing.ValueType.Integer;
            var rhs = GetNonZero(token, right.GetInteger());
            return new(left.GetInteger() / rhs, type);
        }
        else if (left.IsFloat() && right.IsFloat())
        {
            type = Typing.ValueType.Float;
            var rhs = GetNonZero(token, right.GetFloat());
            return new(left.GetFloat() / rhs, type);
        }
        else if (left.IsInteger() && right.IsFloat())
        {
            type = Typing.ValueType.Float;
            var rhs = GetNonZero(token, right.GetFloat());
            return new(left.GetInteger() / rhs, type);
        }
        else if (left.IsFloat() && right.IsInteger())
        {
            type = Typing.ValueType.Float;
            var rhs = GetNonZero(token, (double)right.GetInteger());
            return new(left.GetFloat() / rhs, type);
        }

        throw new ConversionError(token, "Conversion error in division.");
    }

    private static Value GetMulResult(Token token, ref Value left, ref Value right, ref Typing.ValueType type)
    {
        if (left.IsInteger() && right.IsInteger())
        {
            type = Typing.ValueType.Integer;
            return new(left.GetInteger() * right.GetInteger(), type);
        }
        else if (left.IsFloat() && right.IsFloat())
        {
            type = Typing.ValueType.Float;
            return new(left.GetFloat() * right.GetFloat(), type);
        }
        else if (left.IsInteger() && right.IsFloat())
        {
            type = Typing.ValueType.Float;
            return new(left.GetInteger() * right.GetFloat(), type);
        }
        else if (left.IsFloat() && right.IsInteger())
        {
            type = Typing.ValueType.Float;
            return new(left.GetFloat() * right.GetInteger(), type);
        }
        else if (left.IsString() && right.IsInteger())
        {
            type = Typing.ValueType.String;
            return new(StringOp.StringMul(ref left, ref right), type);
        }
        else if (left.IsList() && right.IsInteger())
        {
            type = Typing.ValueType.List;
            return new(ListOp.ListMul(token, ref left, ref right), type);
        }

        throw new ConversionError(token, "Conversion error in multiplication.");
    }
}