#ifndef KIWI_PARSING_KEYWORDS_H
#define KIWI_PARSING_KEYWORDS_H

#include <string>
#include <unordered_set>
#include "parsing/tokentype.h"

struct {
  const std::string Qualifier = "::";
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

  std::unordered_set<std::string> large_operators = {
      Exponent, Or, And, BitwiseLeftShift, BitwiseRightShift};

  std::unordered_set<char> arithmetic_operator_chars = {'+', '-', '/', '*',
                                                        '%'};

  std::unordered_set<char> boolean_operator_chars = {'=', '!', '<',
                                                     '>', '|', '&'};

  std::unordered_set<char> bitwise_operator_chars = {'^', '~', '&', '|'};

  std::unordered_set<SubTokenType> assignment_operators = {
      SubTokenType::Ops_Assign,
      SubTokenType::Ops_AddAssign,
      SubTokenType::Ops_SubtractAssign,
      SubTokenType::Ops_MultiplyAssign,
      SubTokenType::Ops_DivideAssign,
      SubTokenType::Ops_ExponentAssign,
      SubTokenType::Ops_OrAssign,
      SubTokenType::Ops_AndAssign,
      SubTokenType::Ops_BitwiseOrAssign,
      SubTokenType::Ops_BitwiseAndAssign,
      SubTokenType::Ops_BitwiseXorAssign,
      SubTokenType::Ops_BitwiseNotAssign,
      SubTokenType::Ops_BitwiseLeftShiftAssign,
      SubTokenType::Ops_BitwiseRightShiftAssign};

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

  bool is_assignment_operator(const SubTokenType& arg) {
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
  const std::string Abstract = "abstract";
  const std::string As = "as";
  const std::string Break = "break";
  const std::string Catch = "catch";
  const std::string Class = "class";
  const std::string Ctor = "initialize";
  const std::string Delete = "delete";
  const std::string Do = "do";
  const std::string Each = "each";
  const std::string Else = "else";
  const std::string ElseIf = "elsif";
  const std::string End = "end";
  const std::string Exit = "exit";
  const std::string Export = "export";
  const std::string False = "false";
  const std::string For = "for";
  const std::string Go = "go";
  const std::string If = "if";
  const std::string Import = "import";
  const std::string In = "in";
  const std::string Lambda = "lambda";
  const std::string Method = "def";
  const std::string Module = "module";
  const std::string New = "new";
  const std::string Next = "next";
  const std::string Override = "override";
  const std::string Pass = "pass";
  const std::string Print = "print";
  const std::string PrintLn = "println";
  const std::string Private = "private";
  const std::string Return = "return";
  const std::string Static = "static";
  const std::string This = "this";
  const std::string True = "true";
  const std::string Try = "try";
  const std::string kiwiApp = "kiwi";
  const std::string While = "while";

  std::unordered_set<std::string> keywords = {
      Abstract, As,      Break,  Catch,  Class, Delete,   Do,   Else,
      ElseIf,   End,     Exit,   Export, False, For,      If,   Import,
      In,       Lambda,  Method, Module, Next,  Override, Pass, Print,
      PrintLn,  Private, Return, Static, This,  True,     Try,  While};

  std::unordered_set<std::string> conditional_keywords = {If, Else, ElseIf,
                                                          End};

  std::unordered_set<SubTokenType> block_keywords = {
      SubTokenType::KW_While, SubTokenType::KW_For,    SubTokenType::KW_Method,
      SubTokenType::KW_If,    SubTokenType::KW_Module, SubTokenType::KW_Try,
      SubTokenType::KW_Class, SubTokenType::KW_Lambda};

  bool is_keyword(std::string& arg) {
    return keywords.find(arg) != keywords.end();
  }

  bool is_boolean(std::string& arg) { return arg == True || arg == False; }

  bool is_conditional_keyword(std::string& arg) {
    return conditional_keywords.find(arg) != conditional_keywords.end();
  }

  bool is_loop_keyword(const SubTokenType& arg) {
    return arg == SubTokenType::KW_While || arg == SubTokenType::KW_For;
  }

  bool is_block_keyword(const SubTokenType& arg) {
    return block_keywords.find(arg) != block_keywords.end();
  }
} Keywords;

struct {
  const std::string Integer = "Integer";
  const std::string Double = "Double";
  const std::string Boolean = "Boolean";
  const std::string String = "String";
  const std::string List = "List";
  const std::string Hash = "Hash";
  const std::string Object = "Object";
  const std::string Lambda = "Lambda";
  const std::string None = "None";

  std::unordered_set<std::string> typenames = {
      Integer, Double, Boolean, String, List, Hash, Object, Lambda, None};

  bool is_typename(const std::string& arg) {
    return typenames.find(arg) != typenames.end();
  }
} TypeNames;

#endif