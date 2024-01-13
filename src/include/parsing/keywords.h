#ifndef KW_H
#define KW_H

#include <string>
#include <unordered_set>

struct {
    const std::string All               = "all";
    const std::string Args              = "args";
    const std::string ArgValues         = "args.values";
    const std::string BeginInlineScript = "__begin__";
    const std::string BeginsWith        = "begins_with";
    const std::string Break             = "break";
    const std::string Case              = "case";
    const std::string Catch             = "catch";
    const std::string Caught            = "caught";
    const std::string ChangeDirectory   = "chdir";
    const std::string Class             = "class";
    const std::string Classes           = "classes";
    const std::string Clear             = "clear";
    const std::string Constants         = "constants";
    const std::string Contains          = "contains";
    const std::string CreateDirectory   = "dpush";
    const std::string CreateFile        = "fpush";
    const std::string CurrentDirectory  = "cwd";
    const std::string CurrentMachine    = "machine";
    const std::string CurrentUser       = "user";
    const std::string Default           = "default";
    const std::string Delay             = "delay";
    const std::string Dot               = ".";
    const std::string Each              = "each";
    const std::string Else              = "else";
    const std::string Elsif             = "elsif";
    const std::string End               = "end";
    const std::string EndFor            = "endfor";
    const std::string Endif             = "endif";
    const std::string EndInlineScript   = "__end__";
    const std::string EndsWith          = "ends_with";
    const std::string Env               = "env";
    const std::string Err               = "err";
    const std::string Exit              = "exit";
    const std::string Failif            = "failif";
    const std::string False             = "false";
    const std::string FalseyZero        = "0";
    const std::string FileAppend        = "append";
    const std::string FileAppendLine    = "appendl";
    const std::string FileSize          = "fsize";
    const std::string FileWrite         = "fwrite";
    const std::string For               = "for";
    const std::string GC                = "gc!";
    const std::string GetDirectories    = "get_dirs";
    const std::string GetFiles          = "get_files";
    const std::string Globalize         = "globalize!";
    const std::string Goto              = "goto";
    const std::string If                = "if";
    const std::string In                = "in";
    const std::string Infinity          = "inf";
    const std::string InitialDirectory  = "init_dir";
    const std::string InlineParse       = "!";
    const std::string InternalMethods   = "__methods";
    const std::string InternalVariables = "__variables";
    const std::string InvokeMethod      = "invoke!";
    const std::string IsClass           = "class?";
    const std::string IsCollectable     = "collect?";
    const std::string IsDirectory       = "directory?";
    const std::string IsFile            = "file?";
    const std::string IsList            = "list?";
    const std::string IsMethod          = "method?";
    const std::string IsNumber          = "number?";
    const std::string IsString          = "string?";
    const std::string IsVariable        = "variable?";
    const std::string LastError         = "last_error";
    const std::string LastValue         = "last_value";
    const std::string List              = "list";
    const std::string Lists             = "lists";
    const std::string Load              = "load!";
    const std::string Lock              = "lock";
    const std::string LockedMethod      = "[def]";
    const std::string Mask              = "mask";
    const std::string Method            = "def";
    const std::string Methods           = "methods";
    const std::string Module            = "module";
    const std::string Next              = "next";
    const std::string Parser            = "parser";
    const std::string Pass              = "pass";
    const std::string Print             = "print";
    const std::string PrintLn           = "println";
    const std::string Prompt            = "prompt";
    const std::string Random            = "random";
    const std::string RangeSeparator    = "..";
    const std::string Read              = "read";
    const std::string ReadLine          = "readline";
    const std::string Redefine          = "redefine";
    const std::string Remove            = "remove";
    const std::string RemoveDirectory   = "dpop";
    const std::string RemoveFile        = "fpop";
    const std::string Reverse           = "reverse";
    const std::string Revert            = "revert";
    const std::string Self              = "self";
    const std::string SelfDot           = "self.";
    const std::string ShellExec         = "?";
    const std::string Size              = "size";
    const std::string Sort              = "sort";
    const std::string Split             = "split";
    const std::string Switch            = "switch";
    const std::string Template          = "template";
    const std::string ToDouble          = "to_double";
    const std::string ToInteger         = "to_int";
    const std::string ToLower           = "to_lower";
    const std::string ToNumber          = "to_number";
    const std::string ToString          = "to_string";
    const std::string ToUpper           = "to_upper";
    const std::string True              = "true";
    const std::string TruthyOne         = "1";
    const std::string Try               = "try";
    const std::string Unlock            = "unlock";
    const std::string UslangApp         = "usl";
    const std::string Values            = "values";
    const std::string Variables         = "variables";
    const std::string While             = "while";
} Keywords;

struct {
    const std::string DeclVar          = "@";
    const std::string OpenCurlyBrace   = "{";
    const std::string CloseCurlyBrace  = "}";
    const std::string OpenParenthesis  = "(";
    const std::string CloseParenthesis = ")";
    const std::string Interpolate      = "$";
} Symbols;

struct {
    const std::string Public    = "public";
    const std::string Private   = "private";
    const std::string Protected = "protected";
} AccessModifiers;

struct {
    const std::string Add                = "+";
    const std::string AddAssign          = "+=";
    const std::string Assign             = "=";
    const std::string Divide             = "/";
    const std::string DivideAssign       = "/=";
    const std::string Equal              = "==";
    const std::string Exponent           = "^";
    const std::string ExponentAssign     = "^=";
    const std::string GreaterThan        = ">";
    const std::string GreaterThanOrEqual = ">=";
    const std::string LessThan           = "<";
    const std::string LessThanOrEqual    = "<=";
    const std::string ModuloAssign       = "%=";
    const std::string Modulus            = "%";
    const std::string Multiply           = "*";
    const std::string MultiplyAssign     = "*=";
    const std::string NotEqual           = "!=";
    const std::string Subtract           = "-";
    const std::string SubtractAssign     = "-=";
    const std::string To                 = "to";

    std::unordered_set<std::string> assignment_operators = {
        Assign, AddAssign, SubtractAssign, 
        MultiplyAssign, DivideAssign, ExponentAssign
    };

    std::unordered_set<char> arithmetic_operator_chars = { '+', '-', '/', '*', '^', '%' };

    bool is_assignment_operator(std::string& arg) {
        return assignment_operators.find(arg) != assignment_operators.end();
    }

    bool is_arithmetic_operator_char(char arg) {
        return arithmetic_operator_chars.find(arg) != arithmetic_operator_chars.end();
    }
} Operators;

struct {
    const std::string Sin   = "sin";
    const std::string Sinh  = "sinh";
    const std::string Asin  = "asin";
    const std::string Tan   = "tan";
    const std::string Tanh  = "tanh";
    const std::string Atan  = "atan";
    const std::string Cos   = "cos";
    const std::string Acos  = "acos";
    const std::string Cosh  = "cosh";
    const std::string Log   = "log";
    const std::string Sqrt  = "sqrt";
    const std::string Abs   = "abs";
    const std::string Floor = "floor";
    const std::string Ceil  = "ceil";
    const std::string Exp   = "exp";

    std::unordered_set<std::string> math_functions = {
        Abs, Acos, Asin, Atan, Ceil,
        Cos, Cosh, Exp, Floor, Log,
        Sin, Sinh, Sqrt, Tan, Tanh
    };
    
    bool is_math_function(const std::string &arg) {
        return math_functions.find(arg) != math_functions.end();
    }
} Math;

#endif