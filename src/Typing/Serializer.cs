using kiwi.Parsing;

namespace kiwi.Typing;

public partial class Serializer
{
    public static bool AssertTypematch(Value v, TokenName typeName)
    {
        return typeName switch
        {
            TokenName.Types_Any => true,
            TokenName.Types_Boolean => v.IsBoolean(),
            TokenName.Types_Float => v.IsFloat(),
            TokenName.Types_Hashmap => v.IsHashmap(),
            TokenName.Types_Integer => v.IsInteger(),
            TokenName.Types_Date => v.IsDate(),
            TokenName.Types_Lambda => v.IsLambda(),
            TokenName.Types_List => v.IsList(),
            TokenName.Types_None => v.IsNull(),
            TokenName.Types_Object => v.IsObject(),
            TokenName.Types_String => v.IsString(),
            TokenName.Types_Pointer => v.IsPointer(),
            _ => false,
        };
    }

    public static string GetTypenameString(TokenName name)
    {
        return name switch
        {
            TokenName.Types_Any => "Any",
            TokenName.Types_Boolean => "Boolean",
            TokenName.Types_Float => "Float",
            TokenName.Types_Hashmap => "Hashmap",
            TokenName.Types_Integer => "Integer",
            TokenName.Types_Date => "Date",
            TokenName.Types_Lambda => "Lambda",
            TokenName.Types_List => "List",
            TokenName.Types_None => "None",
            TokenName.Types_Object => "Object",
            TokenName.Types_Pointer => "Pointer",
            TokenName.Types_String => "String",
            _ => string.Empty,
        };
    }

    public static string GetTypenameString(Value v)
    {
        if (v.IsInteger())
        {
            return "Integer";
        }
        else if (v.IsFloat())
        {
            return "Float";
        }
        else if (v.IsBoolean())
        {
            return "Boolean";
        }
        else if (v.IsString())
        {
            return "String";
        }
        else if (v.IsDate())
        {
            return "Date";
        }
        else if (v.IsNull())
        {
            return "None";
        }
        else if (v.IsList())
        {
            return "List";
        }
        else if (v.IsHashmap())
        {
            return "Hashmap";
        }
        else if (v.IsObject())
        {
            return v.GetObject().StructName;
        }
        else if (v.IsLambda())
        {
            return "Lambda";
        }
        else if (v.IsPointer())
        {
            return "Pointer";
        }
        else
        {
            return string.Empty;
        }
    }

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