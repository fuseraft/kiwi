#ifndef KIWI_PARSING_KEYWORDS_H
#define KIWI_PARSING_KEYWORDS_H

#include <string>
#include <unordered_set>

struct {
  const std::string DeclVar = "@";
  const std::string OpenCurlyBrace = "{";
  const std::string CloseCurlyBrace = "}";
  const std::string OpenParenthesis = "(";
  const std::string CloseParenthesis = ")";
  const std::string Interpolate = "$";
  const std::string Qualifier = "::";
  const std::string Range = "..";
} Symbols;

struct {
  const std::string Add = "+";
  const std::string AddAssign = "+=";
  const std::string Assign = "=";
  const std::string Divide = "/";
  const std::string DivideAssign = "/=";
  const std::string Equal = "==";
  const std::string Exponent = "**";
  const std::string ExponentAssign = "**=";
  const std::string GreaterThan = ">";
  const std::string GreaterThanOrEqual = ">=";
  const std::string LessThan = "<";
  const std::string LessThanOrEqual = "<=";
  const std::string ModuloAssign = "%=";
  const std::string Modulus = "%";
  const std::string Multiply = "*";
  const std::string MultiplyAssign = "*=";
  const std::string NotEqual = "!=";
  const std::string Subtract = "-";
  const std::string SubtractAssign = "-=";
  const std::string Or = "||";
  const std::string OrAssign = "||=";
  const std::string And = "&&";
  const std::string AndAssign = "&&=";
  const std::string Not = "!";
  const std::string BitwiseOr = "|";
  const std::string BitwiseOrAssign = "|=";
  const std::string BitwiseAnd = "&";
  const std::string BitwiseAndAssign = "&=";
  const std::string BitwiseXor = "^";
  const std::string BitwiseXorAssign = "^=";
  const std::string BitwiseNot = "~";
  const std::string BitwiseNotAssign = "~=";
  const std::string BitwiseLeftShift = "<<";
  const std::string BitwiseLeftShiftAssign = "<<=";
  const std::string BitwiseRightShift = ">>";
  const std::string BitwiseRightShiftAssign = ">>=";
  const std::string ListAppend = "<<";
  const std::string ListInsert = "<<=";

  std::unordered_set<std::string> large_operators = {
      Exponent, Or, And, BitwiseLeftShift, BitwiseRightShift};

  std::unordered_set<char> arithmetic_operator_chars = {'+', '-', '/', '*',
                                                        '%'};

  std::unordered_set<char> boolean_operator_chars = {'=', '!', '<',
                                                     '>', '|', '&'};

  std::unordered_set<char> bitwise_operator_chars = {'^', '~', '&', '|'};

  std::unordered_set<std::string> assignment_operators = {
      Assign,
      AddAssign,
      SubtractAssign,
      MultiplyAssign,
      DivideAssign,
      ExponentAssign,
      OrAssign,
      AndAssign,
      BitwiseOrAssign,
      BitwiseAndAssign,
      BitwiseXorAssign,
      BitwiseNotAssign,
      BitwiseLeftShiftAssign,
      BitwiseRightShiftAssign};

  std::unordered_set<std::string> arithmetic_operators = {
      Add, Subtract, Multiply, Divide, Exponent, Modulus};

  std::unordered_set<std::string> relational_operators = {
      Equal,           NotEqual,    LessThan,
      LessThanOrEqual, GreaterThan, GreaterThanOrEqual};

  std::unordered_set<std::string> logical_operators = {And, Or, Not};

  std::unordered_set<std::string> bitwise_operators = {
      BitwiseAnd, BitwiseOr,        BitwiseXor,
      BitwiseNot, BitwiseLeftShift, BitwiseRightShift};

  bool is_large_operator(std::string& arg) {
    return large_operators.find(arg) != large_operators.end();
  }

  bool is_assignment_operator(std::string& arg) {
    return assignment_operators.find(arg) != assignment_operators.end();
  }

  bool is_arithmetic_operator_char(char arg) {
    return arithmetic_operator_chars.find(arg) !=
           arithmetic_operator_chars.end();
  }

  bool is_bitwise_operator_char(char arg) {
    return bitwise_operator_chars.find(arg) != bitwise_operator_chars.end();
  }

  bool is_boolean_operator_char(char arg) {
    return boolean_operator_chars.find(arg) != boolean_operator_chars.end();
  }

  bool is_logical_operator(std::string& arg) {
    return logical_operators.find(arg) != logical_operators.end();
  }

  bool is_bitwise_operator(std::string& arg) {
    return bitwise_operators.find(arg) != bitwise_operators.end();
  }

  bool is_relational_operator(std::string& arg) {
    return relational_operators.find(arg) != relational_operators.end();
  }

  bool is_arithmetic_operator(std::string& arg) {
    return arithmetic_operators.find(arg) != arithmetic_operators.end();
  }
} Operators;

struct {
  const std::string Sin = "sin";
  const std::string Sinh = "sinh";
  const std::string Asin = "asin";
  const std::string Tan = "tan";
  const std::string Tanh = "tanh";
  const std::string Atan = "atan";
  const std::string Cos = "cos";
  const std::string Acos = "acos";
  const std::string Cosh = "cosh";
  const std::string Log = "log";
  const std::string Sqrt = "sqrt";
  const std::string Abs = "abs";
  const std::string Floor = "floor";
  const std::string Ceil = "ceil";
  const std::string Exp = "exp";

  std::unordered_set<std::string> math_functions = {
      Abs,   Acos, Asin, Atan, Ceil, Cos, Cosh, Exp,
      Floor, Log,  Sin,  Sinh, Sqrt, Tan, Tanh};

  bool is_math_function(const std::string& arg) {
    return math_functions.find(arg) != math_functions.end();
  }
} Math;

struct {
  const std::string Break = "break";
  const std::string Case = "case";
  const std::string Catch = "catch";
  const std::string Caught = "caught";
  const std::string Class = "class";
  const std::string Default = "default";
  const std::string Delay = "delay";
  const std::string Do = "do";
  const std::string Dot = ".";
  const std::string Each = "each";
  const std::string Else = "else";
  const std::string ElseIf = "elsif";
  const std::string End = "end";
  const std::string Env = "env";
  const std::string Err = "err";
  const std::string Exit = "exit";
  const std::string False = "false";
  const std::string For = "for";
  const std::string If = "if";
  const std::string Import = "import";
  const std::string In = "in";
  const std::string Infinity = "inf";
  const std::string Method = "def";
  const std::string Module = "module";
  const std::string Next = "next";
  const std::string Pass = "pass";
  const std::string Print = "print";
  const std::string PrintLn = "println";
  const std::string Random = "random";
  const std::string RangeSeparator = "..";
  const std::string Read = "read";
  const std::string ReadLine = "readline";
  const std::string Return = "return";
  const std::string Self = "self";
  const std::string Switch = "switch";
  const std::string True = "true";
  const std::string Try = "try";
  const std::string kiwiApp = "kiwi";
  const std::string While = "while";

  std::unordered_set<std::string> keywords = {
      If,     Else,   ElseIf, True,    False, While, End, Module, Method,
      Return, Import, Print,  PrintLn, For,   In,    Do,  Next,   Break};

  std::unordered_set<std::string> conditional_keywords = {If, Else, ElseIf,
                                                          End};

  std::unordered_set<std::string> loop_keywords = {While, For};

  std::unordered_set<std::string> required_end_keywords = {While, For, Method,
                                                           If, Module};

  bool is_keyword(std::string& arg) {
    return keywords.find(arg) != keywords.end();
  }

  bool is_boolean(std::string& arg) { return arg == True || arg == False; }

  bool is_conditional_keyword(std::string& arg) {
    return conditional_keywords.find(arg) != conditional_keywords.end();
  }

  bool is_loop_keyword(const std::string& arg) {
    return loop_keywords.find(arg) != loop_keywords.end();
  }

  bool is_required_end_keyword(const std::string& arg) {
    return required_end_keywords.find(arg) != required_end_keywords.end();
  }
} Keywords;

struct {
  const std::string Public = "public";
  const std::string Private = "private";
  const std::string Protected = "protected";
} AccessModifiers;

#endif