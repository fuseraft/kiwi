using kiwi.Parsing;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Operation;

public struct BitwiseOp
{
    public static Value And(Token token, ref Value left, ref Value right, bool doAssign = false)
    {
        Typing.ValueType type = Typing.ValueType.None;
        return new Value(GetAndResult(token, ref left, ref right, ref type), type);
    }

    public static Value Or(Token token, ref Value left, ref Value right, bool doAssign = false)
    {
        Typing.ValueType type = Typing.ValueType.None;
        return new Value(GetOrResult(token, ref left, ref right, ref type), type);
    }

    public static Value Xor(Token token, ref Value left, ref Value right, bool doAssign = false)
    {
        Typing.ValueType type = Typing.ValueType.None;
        return new Value(GetXorResult(token, ref left, ref right, ref type), type);
    }

    public static Value Not(Token token, ref Value left, bool doAssign = false)
    {
        Typing.ValueType type = Typing.ValueType.None;
        return new Value(GetNotResult(token, ref left, ref type), type);
    }

    public static Value Leftshift(Token token, ref Value left, ref Value right, bool doAssign = false)
    {
        if (!left.IsInteger() || !right.IsInteger())
        {
            throw new ConversionError(token, "Conversion error in bitwise << operation.");
        }

        return Value.CreateInteger(left.GetInteger() << (int)right.GetInteger());
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

        return Value.CreateInteger((int)a >>> (int)b);
    }

    public static Value Rightshift(Token token, ref Value left, ref Value right, bool doAssign = false)
    {
        if (!left.IsInteger() || !right.IsInteger())
        {
            throw new ConversionError(token, "Conversion error in bitwise >> operation.");
        }

        return Value.CreateInteger(left.GetInteger() >> (int)right.GetInteger());
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
            return lhs & (long)right.GetFloat();
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
            return lhs | (long)right.GetFloat();
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
            return lhs ^ (long)right.GetFloat();
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
            return ~(long)left.GetFloat();
        }
        else if (left.IsBoolean())
        {
            return ~(left.GetBoolean() ? 1 : 0);
        }

        throw new ConversionError(token, "Conversion error in bitwise ~ operation.");
    }
}