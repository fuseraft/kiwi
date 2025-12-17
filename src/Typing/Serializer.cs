using kiwi.Parsing;

namespace kiwi.Typing;

public partial class Serializer
{
    public static string GetOperatorString(TokenName name)
    {
        return name switch
        {
            TokenName.Ops_Add => "+",
            TokenName.Ops_AddAssign => "+=",
            TokenName.Ops_And => "&&",
            TokenName.Ops_AndAssign => "&&=",
            TokenName.Ops_Assign => "=",
            TokenName.Ops_BitwiseAnd => "&",
            TokenName.Ops_BitwiseAndAssign => "&=",
            TokenName.Ops_BitwiseLeftShift => "<<",
            TokenName.Ops_BitwiseLeftShiftAssign => "<<=",
            TokenName.Ops_BitwiseNot => "~",
            TokenName.Ops_BitwiseNotAssign => "~=",
            TokenName.Ops_BitwiseOr => "|",
            TokenName.Ops_BitwiseOrAssign => "|=",
            TokenName.Ops_BitwiseRightShift => ">>",
            TokenName.Ops_BitwiseRightShiftAssign => ">>=",
            TokenName.Ops_BitwiseUnsignedRightShift => ">>>",
            TokenName.Ops_BitwiseUnsignedRightShiftAssign => ">>>=",
            TokenName.Ops_BitwiseXor => "^",
            TokenName.Ops_BitwiseXorAssign => "^=",
            TokenName.Ops_Divide => "/",
            TokenName.Ops_DivideAssign => "/=",
            TokenName.Ops_Equal => "==",
            TokenName.Ops_Exponent => "**",
            TokenName.Ops_ExponentAssign => "**=",
            TokenName.Ops_GreaterThan => ">",
            TokenName.Ops_GreaterThanOrEqual => ">=",
            TokenName.Ops_LessThan => "<",
            TokenName.Ops_LessThanOrEqual => "<=",
            TokenName.Ops_ModuloAssign => "%=",
            TokenName.Ops_Modulus => "%",
            TokenName.Ops_Multiply => "*",
            TokenName.Ops_MultiplyAssign => "*=",
            TokenName.Ops_Not => "!",
            TokenName.Ops_NotEqual => "!=",
            TokenName.Ops_Or => "||",
            TokenName.Ops_OrAssign => "||=",
            TokenName.Ops_Subtract => "-",
            TokenName.Ops_SubtractAssign => "-=",
            _ => string.Empty,
        };
    }
}