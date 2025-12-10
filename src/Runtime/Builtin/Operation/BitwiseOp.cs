using kiwi.Parsing;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Operation;

public struct BitwiseOp
{
    public static Value And(Token token, ref Value left, ref Value right, bool doAssign = false)
    {
        Typing.ValueType type = Typing.ValueType.None;
        var res = GetAndResult(token, ref left, ref right, ref type);

        if (doAssign)
        {
            left.Set(res, type);
            return left;
        }

        return new Value(res, type);
    }

    public static Value Or(Token token, ref Value left, ref Value right, bool doAssign = false)
    {
        Typing.ValueType type = Typing.ValueType.None;
        var res = GetOrResult(token, ref left, ref right, ref type);

        if (doAssign)
        {
            left.Set(res, type);
            return left;
        }

        return new Value(res, type);
    }

    public static Value Xor(Token token, ref Value left, ref Value right, bool doAssign = false)
    {
        Typing.ValueType type = Typing.ValueType.None;
        var res = GetXorResult(token, ref left, ref right, ref type);

        if (doAssign)
        {
            left.Set(res, type);
            return left;
        }

        return new Value(res, type);
    }

    public static Value Not(Token token, ref Value left, bool doAssign = false)
    {
        Typing.ValueType type = Typing.ValueType.None;
        var res = GetNotResult(token, ref left, ref type);

        if (doAssign)
        {
            left.Set(res, type);
            return left;
        }

        return new Value(res, type);
    }

    public static Value Leftshift(Token token, ref Value left, ref Value right, bool doAssign = false)
    {
        if (!left.IsInteger() || !right.IsInteger())
        {
            throw new ConversionError(token, "Conversion error in bitwise << operation.");
        }

        long res = left.GetInteger() << (int)right.GetInteger();

        if (doAssign)
        {
            left.Set(res, Typing.ValueType.Integer);
            return left;
        }

        return Value.CreateInteger(res);
    }

    public static Value UnsignedRightshift(Token token, ref Value left, ref Value right, bool doAssign = false)
    {
        const long MaxShift = 32L;

        if (!left.IsInteger() || !right.IsInteger())
        {
            throw new ConversionError(token, "Conversion error in bitwise >>> operation.");
        }

        var a = left.GetInteger();
        var b = right.GetInteger();

        if (b >= MaxShift)
        {
            return Value.CreateInteger(0L);
        }

        var res = (int)a >>> (int)b;

        if (doAssign)
        {
            left.Set(res, Typing.ValueType.Integer);
            return left;
        }

        return Value.CreateInteger(res);
    }

    public static Value Rightshift(Token token, ref Value left, ref Value right, bool doAssign = false)
    {
        if (!left.IsInteger() || !right.IsInteger())
        {
            throw new ConversionError(token, "Conversion error in bitwise >> operation.");
        }

        long res = left.GetInteger() >> (int)right.GetInteger();

        if (doAssign)
        {
            left.Set(res, Typing.ValueType.Integer);
            return left;
        }

        return Value.CreateInteger(res);
    }

    private static long GetAndResult(Token token, ref Value left, ref Value right, ref Typing.ValueType type)
    {
        if (!left.IsInteger())
        {
            throw new ConversionError(token, "Conversion error in first operand of bitwise & operation.");
        }
        
        type = Typing.ValueType.Integer;
        var lhs = left.GetInteger();

        if (right.IsInteger())
        {
            return lhs & right.GetInteger();
        }
        else if (right.IsFloat())
        {
            return lhs & (long)(right.GetFloat());
        }
        else if (right.IsBoolean())
        {
            long rhs = right.GetBoolean() ? 1 : 0;
            return lhs & rhs;
        }
        
        throw new ConversionError(token, "Conversion error in second operand of bitwise & operation.");
    }

    private static long GetOrResult(Token token, ref Value left, ref Value right, ref Typing.ValueType type)
    {
        if (!left.IsInteger())
        {
            throw new ConversionError(token, "Conversion error in first operand of bitwise | operation.");
        }
        
        type = Typing.ValueType.Integer;
        var lhs = left.GetInteger();

        if (right.IsInteger())
        {
            return lhs | right.GetInteger();
        }
        else if (right.IsFloat())
        {
            return lhs | (long)(right.GetFloat());
        }
        else if (right.IsBoolean())
        {
            long rhs = right.GetBoolean() ? 1 : 0;
            return lhs | rhs;
        }

        throw new ConversionError(token, "Conversion error in second operand of bitwise | operation.");
    }

    private static long GetXorResult(Token token, ref Value left, ref Value right, ref Typing.ValueType type)
    {
        if (!left.IsInteger())
        {
            throw new ConversionError(token, "Conversion error in first operand of bitwise ^ operation.");
        }
    
        type = Typing.ValueType.Integer;
        var lhs = left.GetInteger();

        if (right.IsInteger())
        {
            return lhs ^ right.GetInteger();
        }
        else if (right.IsFloat())
        {
            return lhs ^ (long)(right.GetFloat());
        }
        else if (right.IsBoolean())
        {
            long rhs = right.GetBoolean() ? 1 : 0;
            return lhs ^ rhs;
        }

        throw new ConversionError(token, "Conversion error in second operand of bitwise ^ operation.");
    }

    private static long GetNotResult(Token token, ref Value left, ref Typing.ValueType type)
    {
        type = Typing.ValueType.Integer;

        if (left.IsInteger())
        {
            return ~left.GetInteger();
        }
        else if (left.IsFloat())
        {
            return ~(long)(left.GetFloat());
        }
        else if (left.IsBoolean())
        {
            return ~(long)(left.GetBoolean() ? 1 : 0);
        }

        throw new ConversionError(token, "Conversion error in bitwise ~ operation.");
    }
}