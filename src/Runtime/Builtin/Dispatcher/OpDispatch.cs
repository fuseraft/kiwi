using kiwi.Runtime.Builtin.Operation;
using kiwi.Parsing;
using kiwi.Tracing.Error;
using kiwi.Typing;

namespace kiwi.Runtime.Builtin.Dispatcher;

public struct OpDispatch
{
    public static Value DoUnary(Token token, TokenName op, ref Value right)
    {
        return op switch
        {
            TokenName.Ops_Not => LogicalOp.Not(ref right),
            TokenName.Ops_BitwiseNot or TokenName.Ops_BitwiseNotAssign => BitwiseOp.Not(token, ref right, op == TokenName.Ops_BitwiseNotAssign),
            TokenName.Ops_Subtract => MathOp.Negate(token, ref right),
            _ => throw new InvalidOperationError(token, "Unknown unary operation."),
        };
    }

    public static Value DoBinary(Token token, TokenName op, ref Value left, ref Value right)
    {
        return op switch
        {
            TokenName.Ops_Add => MathOp.Add(token, ref left, ref right),
            TokenName.Ops_AddAssign => MathOp.Add(token, ref left, ref right, true),
            TokenName.Ops_Subtract => MathOp.Sub(token, ref left, ref right),
            TokenName.Ops_SubtractAssign => MathOp.Sub(token, ref left, ref right, true),
            TokenName.Ops_Multiply => MathOp.Mul(token, ref left, ref right),
            TokenName.Ops_MultiplyAssign => MathOp.Mul(token, ref left, ref right, true),
            TokenName.Ops_Divide => MathOp.Div(token, ref left, ref right),
            TokenName.Ops_DivideAssign => MathOp.Div(token, ref left, ref right, true),
            TokenName.Ops_Modulus => MathOp.Mod(token, ref left, ref right),
            TokenName.Ops_ModuloAssign => MathOp.Mod(token, ref left, ref right, true),
            TokenName.Ops_Exponent => MathOp.Exp(token, ref left, ref right),
            TokenName.Ops_ExponentAssign => MathOp.Exp(token, ref left, ref right, true),
            TokenName.Ops_BitwiseAnd => BitwiseOp.And(token, ref left, ref right),
            TokenName.Ops_BitwiseAndAssign => BitwiseOp.And(token, ref left, ref right, true),
            TokenName.Ops_BitwiseOr => BitwiseOp.Or(token, ref left, ref right),
            TokenName.Ops_BitwiseOrAssign => BitwiseOp.Or(token, ref left, ref right, true),
            TokenName.Ops_BitwiseXor => BitwiseOp.Xor(token, ref left, ref right),
            TokenName.Ops_BitwiseXorAssign => BitwiseOp.Xor(token, ref left, ref right, true),
            TokenName.Ops_BitwiseLeftShift => BitwiseOp.Leftshift(token, ref left, ref right),
            TokenName.Ops_BitwiseLeftShiftAssign => BitwiseOp.Leftshift(token, ref left, ref right, true),
            TokenName.Ops_BitwiseRightShift => BitwiseOp.Rightshift(token, ref left, ref right),
            TokenName.Ops_BitwiseRightShiftAssign => BitwiseOp.Rightshift(token, ref left, ref right, true),
            TokenName.Ops_BitwiseUnsignedRightShift => BitwiseOp.UnsignedRightshift(token, ref left, ref right),
            TokenName.Ops_BitwiseUnsignedRightShiftAssign => BitwiseOp.UnsignedRightshift(token, ref left, ref right, true),
            TokenName.Ops_And => LogicalOp.And(ref left, ref right),
            TokenName.Ops_AndAssign => LogicalOp.And(ref left, ref right, true),
            TokenName.Ops_Or => LogicalOp.Or(ref left, ref right),
            TokenName.Ops_OrAssign => LogicalOp.Or(ref left, ref right, true),
            TokenName.Ops_LessThan => ComparisonOp.LessThan(ref left, ref right),
            TokenName.Ops_LessThanOrEqual => ComparisonOp.LessThanOrEqual(ref left, ref right),
            TokenName.Ops_GreaterThan => ComparisonOp.GreaterThan(ref left, ref right),
            TokenName.Ops_GreaterThanOrEqual => ComparisonOp.GreaterThanOrEqual(ref left, ref right),
            TokenName.Ops_Equal => Value.CreateBoolean(ComparisonOp.Equal(ref left, ref right)),
            TokenName.Ops_NotEqual => ComparisonOp.NotEqual(ref left, ref right),
            TokenName.Ops_NullCoalesce => ComparisonOp.NullCoalesce(ref left, ref right),
            _ => throw new InvalidOperationError(token, "Unknown binary operation."),
        };
    }
}