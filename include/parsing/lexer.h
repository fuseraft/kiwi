#ifndef ASTRAL_PARSING_LEXER_H
#define ASTRAL_PARSING_LEXER_H

#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "parsing/builtins.h"
#include "parsing/keywords.h"
#include "parsing/tokens.h"
#include "parsing/tokentype.h"
#include "system/fileregistry.h"

class Lexer {
 public:
  Lexer(const std::string& file, const std::string& source, bool skipWS = true)
      : source(source), pos(0), skipWS(skipWS), row(0), col(0) {
    fileId = FileRegistry::getInstance().registerFile(file);
  }

  std::shared_ptr<TokenStream> getTokenStream() {
    return std::make_shared<TokenStream>(getAllTokens());
  }

  std::vector<Token> getAllTokens() {
    std::vector<Token> tokens;
    row = 0;
    col = 0;

    while (true) {
      auto token = _getNextToken();

      if (token.getType() == KTokenType::ENDOFFILE) {
        break;
      }

      tokens.emplace_back(token);
    }

    return tokens;
  }

 private:
  std::string source;
  size_t pos;
  bool skipWS;
  int fileId;
  int row;
  int col;

  char getCurrentChar() {
    char c = source[pos++];
    if (c == '\n') {
      row++;
      col = 0;
    } else {
      col++;
    }
    return c;
  }

  char peek() {
    if (pos + 1 < source.length()) {
      return source[pos + 1];
    }
    return '\0';
  }

  Token _getNextToken() {
    skipWhitespace();

    if (pos >= source.length()) {
      return Token::create(KTokenType::ENDOFFILE, KName::Default, fileId,
                           std::string(""), static_cast<k_int>(0), row, col);
    }

    char currentChar = getCurrentChar();

    if (isalpha(currentChar) || currentChar == '_') {
      return parseIdentifier(currentChar);
    } else if (isdigit(currentChar)) {
      if (currentChar == '0' && (pos < source.length() && source[pos] == 'x')) {
        return parseHexLiteral(currentChar);
      }
      return parseLiteral(currentChar);
    } else if (currentChar == '"') {
      return parseString();
    } else if (currentChar == '#') {
      return parseComment();
    } else if (currentChar == '/' &&
               (pos < source.length() && source[pos] == '#')) {
      return parseBlockComment();
    } else if (currentChar == '@') {
      return Token::create(KTokenType::DECLVAR, KName::KW_DeclVar, fileId, "@",
                           row, col);
    } else if (currentChar == '$') {
      return Token::create(KTokenType::OPERATOR, KName::Default, fileId, "$",
                           row, col);
    } else if (currentChar == '\n') {
      return Token::create(KTokenType::NEWLINE, KName::Default, fileId, "\n",
                           row, col);
    } else if (currentChar == '(') {
      return Token::create(KTokenType::OPEN_PAREN, KName::Default, fileId, "(",
                           row, col);
    } else if (currentChar == ')') {
      return Token::create(KTokenType::CLOSE_PAREN, KName::Default, fileId, ")",
                           row, col);
    } else if (currentChar == '[') {
      return Token::create(KTokenType::OPEN_BRACKET, KName::Default, fileId,
                           "[", row, col);
    } else if (currentChar == ']') {
      return Token::create(KTokenType::CLOSE_BRACKET, KName::Default, fileId,
                           "]", row, col);
    } else if (currentChar == '{') {
      return Token::create(KTokenType::OPEN_BRACE, KName::Default, fileId, "{",
                           row, col);
    } else if (currentChar == '}') {
      return Token::create(KTokenType::CLOSE_BRACE, KName::Default, fileId, "}",
                           row, col);
    } else if (currentChar == ',') {
      return Token::create(KTokenType::COMMA, KName::Default, fileId, ",", row,
                           col);
    } else if (currentChar == '?') {
      return Token::create(KTokenType::QUESTION, KName::Default, fileId, "?",
                           row, col);
    } else if (currentChar == '.') {
      return parseDot(currentChar);
    } else if (currentChar == '\\') {
      return parseEscapeCharacter();
    } else if (currentChar == ':') {
      return parseColon(currentChar);
    } else {
      return parseUnspecified(currentChar);
    }
  }

  void skipWhitespace() {
    if (!skipWS) {
      return;
    }

    while (pos < source.length() && isspace(source[pos])) {
      getCurrentChar();
    }
  }

  Token parseConditionalKeyword(const std::string& keyword) {
    if (keyword == Keywords.If) {
      return Token::create(KTokenType::CONDITIONAL, KName::KW_If, fileId,
                           keyword, row, col);
    } else if (keyword == Keywords.ElseIf) {
      return Token::create(KTokenType::CONDITIONAL, KName::KW_ElseIf, fileId,
                           keyword, row, col);
    } else if (keyword == Keywords.Else) {
      return Token::create(KTokenType::CONDITIONAL, KName::KW_Else, fileId,
                           keyword, row, col);
    } else if (keyword == Keywords.End) {
      return Token::create(KTokenType::CONDITIONAL, KName::KW_End, fileId,
                           keyword, row, col);
    }

    return Token::create(KTokenType::CONDITIONAL, KName::Default, fileId,
                         keyword, row, col);
  }

  Token parseKeywordSpecific(const std::string& keyword) {
    auto st = KName::Default;

    if (keyword == Keywords.Abstract) {
      st = KName::KW_Abstract;
    } else if (keyword == Keywords.As) {
      st = KName::KW_As;
    } else if (keyword == Keywords.Async) {
      st = KName::KW_Async;
    } else if (keyword == Keywords.Await) {
      st = KName::KW_Await;
    } else if (keyword == Keywords.Break) {
      st = KName::KW_Break;
    } else if (keyword == Keywords.Catch) {
      st = KName::KW_Catch;
    } else if (keyword == Keywords.Class) {
      st = KName::KW_Class;
    } else if (keyword == Keywords.Delete) {
      st = KName::KW_Delete;
    } else if (keyword == Keywords.Do) {
      st = KName::KW_Do;
    } else if (keyword == Keywords.Exit) {
      st = KName::KW_Exit;
    } else if (keyword == Keywords.Export) {
      st = KName::KW_Export;
    } else if (keyword == Keywords.False) {
      st = KName::KW_False;
    } else if (keyword == Keywords.For) {
      st = KName::KW_For;
    } else if (keyword == Keywords.Import) {
      st = KName::KW_Import;
    } else if (keyword == Keywords.In) {
      st = KName::KW_In;
    } else if (keyword == Keywords.Method) {
      st = KName::KW_Method;
    } else if (keyword == Keywords.Module) {
      st = KName::KW_Module;
    } else if (keyword == Keywords.Next) {
      st = KName::KW_Next;
    } else if (keyword == Keywords.Override) {
      st = KName::KW_Override;
    } else if (keyword == Keywords.Parse) {
      st = KName::KW_Parse;
    } else if (keyword == Keywords.Pass) {
      st = KName::KW_Pass;
    } else if (keyword == Keywords.Print) {
      st = KName::KW_Print;
    } else if (keyword == Keywords.PrintLn) {
      st = KName::KW_PrintLn;
    } else if (keyword == Keywords.Private) {
      st = KName::KW_Private;
    } else if (keyword == Keywords.Return) {
      st = KName::KW_Return;
    } else if (keyword == Keywords.Static) {
      st = KName::KW_Static;
    } else if (keyword == Keywords.Then) {
      st = KName::KW_Then;
    } else if (keyword == Keywords.This) {
      st = KName::KW_This;
    } else if (keyword == Keywords.Throw) {
      st = KName::KW_Throw;
    } else if (keyword == Keywords.Try) {
      st = KName::KW_Try;
    } else if (keyword == Keywords.While) {
      st = KName::KW_While;
    }

    return Token::create(KTokenType::KEYWORD, st, fileId, keyword, row, col);
  }

  Token parseKeyword(const std::string& keyword) {
    if (Keywords.is_conditional_keyword(keyword)) {
      return parseConditionalKeyword(keyword);
    } else if (keyword == Keywords.Lambda) {
      return Token::create(KTokenType::LAMBDA, KName::KW_Lambda, fileId,
                           keyword, row, col);
    } else if (Keywords.is_boolean(keyword)) {
      return Token::createBoolean(fileId, keyword, row, col);
    }

    return parseKeywordSpecific(keyword);
  }

  Token parseOperator(const std::string& op) {
    auto st = KName::Default;

    if (op == Operators.Add) {
      st = KName::Ops_Add;
    } else if (op == Operators.AddAssign) {
      st = KName::Ops_AddAssign;
    } else if (op == Operators.And) {
      st = KName::Ops_And;
    } else if (op == Operators.AndAssign) {
      st = KName::Ops_AndAssign;
    } else if (op == Operators.Assign) {
      st = KName::Ops_Assign;
    } else if (op == Operators.BitwiseAnd) {
      st = KName::Ops_BitwiseAnd;
    } else if (op == Operators.BitwiseAndAssign) {
      st = KName::Ops_BitwiseAndAssign;
    } else if (op == Operators.BitwiseLeftShift) {
      st = KName::Ops_BitwiseLeftShift;
    } else if (op == Operators.BitwiseLeftShiftAssign) {
      st = KName::Ops_BitwiseLeftShiftAssign;
    } else if (op == Operators.BitwiseNot) {
      st = KName::Ops_BitwiseNot;
    } else if (op == Operators.BitwiseNotAssign) {
      st = KName::Ops_BitwiseNotAssign;
    } else if (op == Operators.BitwiseOr) {
      st = KName::Ops_BitwiseOr;
    } else if (op == Operators.BitwiseOrAssign) {
      st = KName::Ops_BitwiseOrAssign;
    } else if (op == Operators.BitwiseRightShift) {
      st = KName::Ops_BitwiseRightShift;
    } else if (op == Operators.BitwiseRightShiftAssign) {
      st = KName::Ops_BitwiseRightShiftAssign;
    } else if (op == Operators.BitwiseXor) {
      st = KName::Ops_BitwiseXor;
    } else if (op == Operators.BitwiseXorAssign) {
      st = KName::Ops_BitwiseXorAssign;
    } else if (op == Operators.Divide) {
      st = KName::Ops_Divide;
    } else if (op == Operators.DivideAssign) {
      st = KName::Ops_DivideAssign;
    } else if (op == Operators.Equal) {
      st = KName::Ops_Equal;
    } else if (op == Operators.Exponent) {
      st = KName::Ops_Exponent;
    } else if (op == Operators.ExponentAssign) {
      st = KName::Ops_ExponentAssign;
    } else if (op == Operators.GreaterThan) {
      st = KName::Ops_GreaterThan;
    } else if (op == Operators.GreaterThanOrEqual) {
      st = KName::Ops_GreaterThanOrEqual;
    } else if (op == Operators.LessThan) {
      st = KName::Ops_LessThan;
    } else if (op == Operators.LessThanOrEqual) {
      st = KName::Ops_LessThanOrEqual;
    } else if (op == Operators.ModuloAssign) {
      st = KName::Ops_ModuloAssign;
    } else if (op == Operators.Modulus) {
      st = KName::Ops_Modulus;
    } else if (op == Operators.Multiply) {
      st = KName::Ops_Multiply;
    } else if (op == Operators.MultiplyAssign) {
      st = KName::Ops_MultiplyAssign;
    } else if (op == Operators.Not) {
      st = KName::Ops_Not;
    } else if (op == Operators.NotEqual) {
      st = KName::Ops_NotEqual;
    } else if (op == Operators.Or) {
      st = KName::Ops_Or;
    } else if (op == Operators.OrAssign) {
      st = KName::Ops_OrAssign;
    } else if (op == Operators.Subtract) {
      st = KName::Ops_Subtract;
    } else if (op == Operators.SubtractAssign) {
      st = KName::Ops_SubtractAssign;
    }

    return Token::create(KTokenType::OPERATOR, st, fileId, op, row, col);
  }

  Token parseUnspecified(char initialChar) {
    std::string s(1, initialChar);

    if (pos < source.length()) {
      char nextChar = source[pos];
      bool isArithmeticOpChar =
          Operators.is_arithmetic_operator_char(initialChar);
      bool isBooleanOpChar = Operators.is_boolean_operator_char(initialChar);
      bool isArithmeticOp =
          (nextChar == '=' && (isArithmeticOpChar || isBooleanOpChar)) ||
          (initialChar == '*' && nextChar == '*');
      bool isBooleanOp =
          (nextChar == '|' || nextChar == '&') && isBooleanOpChar;
      bool isBitwiseOp = (Operators.is_bitwise_operator_char(initialChar) &&
                          nextChar == '=') ||
                         (initialChar == '<' && nextChar == '<') ||
                         (initialChar == '>' && nextChar == '>');

      if (isArithmeticOp || isBooleanOp || isBitwiseOp) {
        s += nextChar;
        getCurrentChar();

        nextChar = source[pos];
        if (nextChar == '=' && Operators.is_large_operator(s)) {
          s += nextChar;
          getCurrentChar();
        }
      }
    }

    return parseOperator(s);
  }

  Token parseColon(char initialChar) {
    std::string s(1, initialChar);

    if (pos < source.length()) {
      char nextChar = source[pos];
      if (nextChar == ':') {
        s += nextChar;
        getCurrentChar();
        return Token::create(KTokenType::QUALIFIER, KName::Default, fileId, s,
                             row, col);
      }
    }

    return Token::create(KTokenType::COLON, KName::Default, fileId, s, row,
                         col);
  }

  Token parseEscapeCharacter() {
    if (pos < source.length()) {
      char nextChar = getCurrentChar();

      switch (nextChar) {
        case 'n':
          return Token::create(KTokenType::ESCAPED, KName::Default, fileId,
                               "\n", row, col);
        case 'r':
          return Token::create(KTokenType::ESCAPED, KName::Default, fileId,
                               "\r", row, col);
        case 't':
          return Token::create(KTokenType::ESCAPED, KName::Default, fileId,
                               "\t", row, col);
        case '"':
          return Token::create(KTokenType::ESCAPED, KName::Default, fileId,
                               "\"", row, col);
        case 'b':
          return Token::create(KTokenType::ESCAPED, KName::Default, fileId,
                               "\b", row, col);
        case 'f':
          return Token::create(KTokenType::ESCAPED, KName::Default, fileId,
                               "\f", row, col);
        case '\\':
          return Token::create(KTokenType::ESCAPED, KName::Default, fileId,
                               "\\", row, col);
      }
    }

    getCurrentChar();

    return Token::create(KTokenType::ESCAPED, KName::Default, fileId, "\\", row,
                         col);
  }

  Token parseDot(char initialChar) {
    std::string s(1, initialChar);

    if (pos < source.length()) {
      char nextChar = source[pos];
      if (nextChar == '.') {
        s += nextChar;
        getCurrentChar();
        return Token::create(KTokenType::RANGE, KName::Default, fileId, s, row,
                             col);
      }
    }

    return Token::create(KTokenType::DOT, KName::Default, fileId, ".", row,
                         col);
  }

  Token parseTypeName(const std::string& typeName) {
    auto st = KName::Default;

    if (typeName == TypeNames.Integer) {
      st = KName::Types_Integer;
    } else if (typeName == TypeNames.Boolean) {
      st = KName::Types_Boolean;
    } else if (typeName == TypeNames.Double) {
      st = KName::Types_Double;
    } else if (typeName == TypeNames.Hash) {
      st = KName::Types_Hash;
    } else if (typeName == TypeNames.Lambda) {
      st = KName::Types_Lambda;
    } else if (typeName == TypeNames.List) {
      st = KName::Types_List;
    } else if (typeName == TypeNames.Object) {
      st = KName::Types_Object;
    } else if (typeName == TypeNames.String) {
      st = KName::Types_String;
    } else if (typeName == TypeNames.None) {
      st = KName::Types_None;
    }

    return Token::create(KTokenType::TYPENAME, st, fileId, typeName, row, col);
  }

  Token parseArgvBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == ArgvBuiltins.GetArgv) {
      st = KName::Builtin_Argv_GetArgv;
    } else if (builtin == ArgvBuiltins.GetXarg) {
      st = KName::Builtin_Argv_GetXarg;
    }

    return Token::create(KTokenType::IDENTIFIER, st, fileId, builtin, row, col);
  }

  Token parseConsoleBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == ConsoleBuiltins.Input) {
      st = KName::Builtin_Console_Input;
    } else if (builtin == ConsoleBuiltins.Silent) {
      st = KName::Builtin_Console_Silent;
    }

    return Token::create(KTokenType::IDENTIFIER, st, fileId, builtin, row, col);
  }

  Token parseEnvBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == EnvBuiltins.GetEnvironmentVariable) {
      st = KName::Builtin_Env_GetEnvironmentVariable;
    } else if (builtin == EnvBuiltins.SetEnvironmentVariable) {
      st = KName::Builtin_Env_SetEnvironmentVariable;
    }

    return Token::create(KTokenType::IDENTIFIER, st, fileId, builtin, row, col);
  }

  Token parseFileIOBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == FileIOBuiltIns.AppendText) {
      st = KName::Builtin_FileIO_AppendText;
    } else if (builtin == FileIOBuiltIns.ChangeDirectory) {
      st = KName::Builtin_FileIO_ChangeDirectory;
    } else if (builtin == FileIOBuiltIns.CopyFile) {
      st = KName::Builtin_FileIO_CopyFile;
    } else if (builtin == FileIOBuiltIns.CopyR) {
      st = KName::Builtin_FileIO_CopyR;
    } else if (builtin == FileIOBuiltIns.Combine) {
      st = KName::Builtin_FileIO_Combine;
    } else if (builtin == FileIOBuiltIns.CreateFile) {
      st = KName::Builtin_FileIO_CreateFile;
    } else if (builtin == FileIOBuiltIns.DeleteFile) {
      st = KName::Builtin_FileIO_DeleteFile;
    } else if (builtin == FileIOBuiltIns.FileExists) {
      st = KName::Builtin_FileIO_FileExists;
    } else if (builtin == FileIOBuiltIns.FileName) {
      st = KName::Builtin_FileIO_FileName;
    } else if (builtin == FileIOBuiltIns.FileSize) {
      st = KName::Builtin_FileIO_FileSize;
    } else if (builtin == FileIOBuiltIns.GetCurrentDirectory) {
      st = KName::Builtin_FileIO_GetCurrentDirectory;
    } else if (builtin == FileIOBuiltIns.GetFileAbsolutePath) {
      st = KName::Builtin_FileIO_GetFileAbsolutePath;
    } else if (builtin == FileIOBuiltIns.GetFileAttributes) {
      st = KName::Builtin_FileIO_GetFileAttributes;
    } else if (builtin == FileIOBuiltIns.GetFileExtension) {
      st = KName::Builtin_FileIO_GetFileExtension;
    } else if (builtin == FileIOBuiltIns.GetFilePath) {
      st = KName::Builtin_FileIO_GetFilePath;
    } else if (builtin == FileIOBuiltIns.Glob) {
      st = KName::Builtin_FileIO_Glob;
    } else if (builtin == FileIOBuiltIns.IsDirectory) {
      st = KName::Builtin_FileIO_IsDirectory;
    } else if (builtin == FileIOBuiltIns.ListDirectory) {
      st = KName::Builtin_FileIO_ListDirectory;
    } else if (builtin == FileIOBuiltIns.MakeDirectory) {
      st = KName::Builtin_FileIO_MakeDirectory;
    } else if (builtin == FileIOBuiltIns.MakeDirectoryP) {
      st = KName::Builtin_FileIO_MakeDirectoryP;
    } else if (builtin == FileIOBuiltIns.MoveFile) {
      st = KName::Builtin_FileIO_MoveFile;
    } else if (builtin == FileIOBuiltIns.ReadFile) {
      st = KName::Builtin_FileIO_ReadFile;
    } else if (builtin == FileIOBuiltIns.ReadLines) {
      st = KName::Builtin_FileIO_ReadLines;
    } else if (builtin == FileIOBuiltIns.ReadBytes) {
      st = KName::Builtin_FileIO_ReadBytes;
    } else if (builtin == FileIOBuiltIns.RemoveDirectory) {
      st = KName::Builtin_FileIO_RemoveDirectory;
    } else if (builtin == FileIOBuiltIns.RemoveDirectoryF) {
      st = KName::Builtin_FileIO_RemoveDirectoryF;
    } else if (builtin == FileIOBuiltIns.TempDir) {
      st = KName::Builtin_FileIO_TempDir;
    } else if (builtin == FileIOBuiltIns.WriteBytes) {
      st = KName::Builtin_FileIO_WriteBytes;
    } else if (builtin == FileIOBuiltIns.WriteLine) {
      st = KName::Builtin_FileIO_WriteLine;
    } else if (builtin == FileIOBuiltIns.WriteText) {
      st = KName::Builtin_FileIO_WriteText;
    }

    return Token::create(KTokenType::IDENTIFIER, st, fileId, builtin, row, col);
  }

  Token parseListBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == ListBuiltins.Map) {
      st = KName::Builtin_List_Map;
    } else if (builtin == ListBuiltins.None) {
      st = KName::Builtin_List_None;
    } else if (builtin == ListBuiltins.Reduce) {
      st = KName::Builtin_List_Reduce;
    } else if (builtin == ListBuiltins.Select) {
      st = KName::Builtin_List_Select;
    } else if (builtin == ListBuiltins.Sort) {
      st = KName::Builtin_List_Sort;
    } else if (builtin == ListBuiltins.ToH) {
      st = KName::Builtin_List_ToH;
    }

    return Token::create(KTokenType::IDENTIFIER, st, fileId, builtin, row, col);
  }

  Token parseMathBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == MathBuiltins.Abs) {
      st = KName::Builtin_Math_Abs;
    } else if (builtin == MathBuiltins.Acos) {
      st = KName::Builtin_Math_Acos;
    } else if (builtin == MathBuiltins.Asin) {
      st = KName::Builtin_Math_Asin;
    } else if (builtin == MathBuiltins.Atan) {
      st = KName::Builtin_Math_Atan;
    } else if (builtin == MathBuiltins.Atan2) {
      st = KName::Builtin_Math_Atan2;
    } else if (builtin == MathBuiltins.Cbrt) {
      st = KName::Builtin_Math_Cbrt;
    } else if (builtin == MathBuiltins.Ceil) {
      st = KName::Builtin_Math_Ceil;
    } else if (builtin == MathBuiltins.CopySign) {
      st = KName::Builtin_Math_CopySign;
    } else if (builtin == MathBuiltins.Cos) {
      st = KName::Builtin_Math_Cos;
    } else if (builtin == MathBuiltins.Cosh) {
      st = KName::Builtin_Math_Cosh;
    } else if (builtin == MathBuiltins.Divisors) {
      st = KName::Builtin_Math_Divisors;
    } else if (builtin == MathBuiltins.Epsilon) {
      st = KName::Builtin_Math_Epsilon;
    } else if (builtin == MathBuiltins.Erf) {
      st = KName::Builtin_Math_Erf;
    } else if (builtin == MathBuiltins.ErfC) {
      st = KName::Builtin_Math_ErfC;
    } else if (builtin == MathBuiltins.Exp) {
      st = KName::Builtin_Math_Exp;
    } else if (builtin == MathBuiltins.ExpM1) {
      st = KName::Builtin_Math_ExpM1;
    } else if (builtin == MathBuiltins.FDim) {
      st = KName::Builtin_Math_FDim;
    } else if (builtin == MathBuiltins.Floor) {
      st = KName::Builtin_Math_Floor;
    } else if (builtin == MathBuiltins.FMax) {
      st = KName::Builtin_Math_FMax;
    } else if (builtin == MathBuiltins.FMin) {
      st = KName::Builtin_Math_FMin;
    } else if (builtin == MathBuiltins.Fmod) {
      st = KName::Builtin_Math_Fmod;
    } else if (builtin == MathBuiltins.Hypot) {
      st = KName::Builtin_Math_Hypot;
    } else if (builtin == MathBuiltins.IsFinite) {
      st = KName::Builtin_Math_IsFinite;
    } else if (builtin == MathBuiltins.IsInf) {
      st = KName::Builtin_Math_IsInf;
    } else if (builtin == MathBuiltins.IsNaN) {
      st = KName::Builtin_Math_IsNaN;
    } else if (builtin == MathBuiltins.IsNormal) {
      st = KName::Builtin_Math_IsNormal;
    } else if (builtin == MathBuiltins.LGamma) {
      st = KName::Builtin_Math_LGamma;
    } else if (builtin == MathBuiltins.Log) {
      st = KName::Builtin_Math_Log;
    } else if (builtin == MathBuiltins.Log10) {
      st = KName::Builtin_Math_Log10;
    } else if (builtin == MathBuiltins.Log1P) {
      st = KName::Builtin_Math_Log1P;
    } else if (builtin == MathBuiltins.Log2) {
      st = KName::Builtin_Math_Log2;
    } else if (builtin == MathBuiltins.NextAfter) {
      st = KName::Builtin_Math_NextAfter;
    } else if (builtin == MathBuiltins.Pow) {
      st = KName::Builtin_Math_Pow;
    } else if (builtin == MathBuiltins.Random) {
      st = KName::Builtin_Math_Random;
    } else if (builtin == MathBuiltins.Remainder) {
      st = KName::Builtin_Math_Remainder;
    } else if (builtin == MathBuiltins.Round) {
      st = KName::Builtin_Math_Round;
    } else if (builtin == MathBuiltins.Sin) {
      st = KName::Builtin_Math_Sin;
    } else if (builtin == MathBuiltins.Sinh) {
      st = KName::Builtin_Math_Sinh;
    } else if (builtin == MathBuiltins.Sqrt) {
      st = KName::Builtin_Math_Sqrt;
    } else if (builtin == MathBuiltins.Tan) {
      st = KName::Builtin_Math_Tan;
    } else if (builtin == MathBuiltins.Tanh) {
      st = KName::Builtin_Math_Tanh;
    } else if (builtin == MathBuiltins.TGamma) {
      st = KName::Builtin_Math_TGamma;
    } else if (builtin == MathBuiltins.Trunc) {
      st = KName::Builtin_Math_Trunc;
    } else if (builtin == MathBuiltins.ListPrimes) {
      st = KName::Builtin_Math_ListPrimes;
    } else if (builtin == MathBuiltins.NthPrime) {
      st = KName::Builtin_Math_NthPrime;
    }

    return Token::create(KTokenType::IDENTIFIER, st, fileId, builtin, row, col);
  }

  Token parseModuleBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == ModuleBuiltins.Home) {
      st = KName::Builtin_Module_Home;
    }

    return Token::create(KTokenType::IDENTIFIER, st, fileId, builtin, row, col);
  }

  Token parseSysBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == SysBuiltins.EffectiveUserId) {
      st = KName::Builtin_Sys_EffectiveUserId;
    } else if (builtin == SysBuiltins.Exec) {
      st = KName::Builtin_Sys_Exec;
    } else if (builtin == SysBuiltins.ExecOut) {
      st = KName::Builtin_Sys_ExecOut;
    }

    return Token::create(KTokenType::IDENTIFIER, st, fileId, builtin, row, col);
  }

  Token parseWebClientBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == HttpBuiltins.Get) {
      st = KName::Builtin_WebClient_Get;
    } else if (builtin == HttpBuiltins.Post) {
      st = KName::Builtin_WebClient_Post;
    } else if (builtin == HttpBuiltins.Put) {
      st = KName::Builtin_WebClient_Put;
    } else if (builtin == HttpBuiltins.Delete) {
      st = KName::Builtin_WebClient_Delete;
    } else if (builtin == HttpBuiltins.Head) {
      st = KName::Builtin_WebClient_Head;
    } else if (builtin == HttpBuiltins.Options) {
      st = KName::Builtin_WebClient_Options;
    } else if (builtin == HttpBuiltins.Patch) {
      st = KName::Builtin_WebClient_Patch;
    }

    return Token::create(KTokenType::IDENTIFIER, st, fileId, builtin, row, col);
  }

  Token parseWebServerBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == WebServerBuiltins.Get) {
      st = KName::Builtin_WebServer_Get;
    } else if (builtin == WebServerBuiltins.Post) {
      st = KName::Builtin_WebServer_Post;
    } else if (builtin == WebServerBuiltins.Listen) {
      st = KName::Builtin_WebServer_Listen;
    } else if (builtin == WebServerBuiltins.Host) {
      st = KName::Builtin_WebServer_Host;
    } else if (builtin == WebServerBuiltins.Port) {
      st = KName::Builtin_WebServer_Port;
    } else if (builtin == WebServerBuiltins.Public) {
      st = KName::Builtin_WebServer_Public;
    }

    return Token::create(KTokenType::IDENTIFIER, st, fileId, builtin, row, col);
  }

  Token parseLoggingBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == LoggingBuiltins.Debug) {
      st = KName::Builtin_Logging_Debug;
    } else if (builtin == LoggingBuiltins.Error) {
      st = KName::Builtin_Logging_Error;
    } else if (builtin == LoggingBuiltins.FilePath) {
      st = KName::Builtin_Logging_FilePath;
    } else if (builtin == LoggingBuiltins.Info) {
      st = KName::Builtin_Logging_Info;
    } else if (builtin == LoggingBuiltins.Level) {
      st = KName::Builtin_Logging_Level;
    } else if (builtin == LoggingBuiltins.Mode) {
      st = KName::Builtin_Logging_Mode;
    } else if (builtin == LoggingBuiltins.EntryFormat) {
      st = KName::Builtin_Logging_EntryFormat;
    } else if (builtin == LoggingBuiltins.TimestampFormat) {
      st = KName::Builtin_Logging_TimestampFormat;
    } else if (builtin == LoggingBuiltins.Warn) {
      st = KName::Builtin_Logging_Warn;
    }

    return Token::create(KTokenType::IDENTIFIER, st, fileId, builtin, row, col);
  }

  Token parseTimeBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == TimeBuiltins.AMPM) {
      st = KName::Builtin_Time_AMPM;
    } else if (builtin == TimeBuiltins.Delay) {
      st = KName::Builtin_Time_Delay;
    } else if (builtin == TimeBuiltins.EpochMilliseconds) {
      st = KName::Builtin_Time_EpochMilliseconds;
    } else if (builtin == TimeBuiltins.Hour) {
      st = KName::Builtin_Time_Hour;
    } else if (builtin == TimeBuiltins.IsDST) {
      st = KName::Builtin_Time_IsDST;
    } else if (builtin == TimeBuiltins.Minute) {
      st = KName::Builtin_Time_Minute;
    } else if (builtin == TimeBuiltins.Month) {
      st = KName::Builtin_Time_Month;
    } else if (builtin == TimeBuiltins.MonthDay) {
      st = KName::Builtin_Time_MonthDay;
    } else if (builtin == TimeBuiltins.Second) {
      st = KName::Builtin_Time_Second;
    } else if (builtin == TimeBuiltins.Ticks) {
      st = KName::Builtin_Time_Ticks;
    } else if (builtin == TimeBuiltins.TicksToMilliseconds) {
      st = KName::Builtin_Time_TicksToMilliseconds;
    } else if (builtin == TimeBuiltins.Timestamp) {
      st = KName::Builtin_Time_Timestamp;
    } else if (builtin == TimeBuiltins.WeekDay) {
      st = KName::Builtin_Time_WeekDay;
    } else if (builtin == TimeBuiltins.Year) {
      st = KName::Builtin_Time_Year;
    } else if (builtin == TimeBuiltins.YearDay) {
      st = KName::Builtin_Time_YearDay;
    }

    return Token::create(KTokenType::IDENTIFIER, st, fileId, builtin, row, col);
  }

  Token parseBuiltinMethod(const std::string& builtin) {
    if (ArgvBuiltins.is_builtin(builtin)) {
      return parseArgvBuiltin(builtin);
    } else if (ConsoleBuiltins.is_builtin(builtin)) {
      return parseConsoleBuiltin(builtin);
    } else if (EnvBuiltins.is_builtin(builtin)) {
      return parseEnvBuiltin(builtin);
    } else if (FileIOBuiltIns.is_builtin(builtin)) {
      return parseFileIOBuiltin(builtin);
    } else if (LoggingBuiltins.is_builtin(builtin)) {
      return parseLoggingBuiltin(builtin);
    } else if (ListBuiltins.is_builtin(builtin)) {
      return parseListBuiltin(builtin);
    } else if (MathBuiltins.is_builtin(builtin)) {
      return parseMathBuiltin(builtin);
    } else if (ModuleBuiltins.is_builtin(builtin)) {
      return parseModuleBuiltin(builtin);
    } else if (SysBuiltins.is_builtin(builtin)) {
      return parseSysBuiltin(builtin);
    } else if (TimeBuiltins.is_builtin(builtin)) {
      return parseTimeBuiltin(builtin);
    } else if (WebServerBuiltins.is_builtin(builtin)) {
      return parseWebServerBuiltin(builtin);
    } else if (HttpBuiltins.is_builtin(builtin)) {
      return parseWebClientBuiltin(builtin);
    }

    return Token::create(KTokenType::IDENTIFIER, KName::Default, fileId,
                         builtin, row, col);
  }

  Token parseAstralBuiltin(const std::string& builtin) {
    auto st = KName::Default;

    if (builtin == AstralBuiltins.BeginsWith) {
      st = KName::Builtin_Astral_BeginsWith;
    } else if (builtin == AstralBuiltins.Chars) {
      st = KName::Builtin_Astral_Chars;
    } else if (builtin == AstralBuiltins.Contains) {
      st = KName::Builtin_Astral_Contains;
    } else if (builtin == AstralBuiltins.Downcase) {
      st = KName::Builtin_Astral_Downcase;
    } else if (builtin == AstralBuiltins.Empty) {
      st = KName::Builtin_Astral_Empty;
    } else if (builtin == AstralBuiltins.EndsWith) {
      st = KName::Builtin_Astral_EndsWith;
    } else if (builtin == AstralBuiltins.HasKey) {
      st = KName::Builtin_Astral_HasKey;
    } else if (builtin == AstralBuiltins.IndexOf) {
      st = KName::Builtin_Astral_IndexOf;
    } else if (builtin == AstralBuiltins.IsA) {
      st = KName::Builtin_Astral_IsA;
    } else if (builtin == AstralBuiltins.Join) {
      st = KName::Builtin_Astral_Join;
    } else if (builtin == AstralBuiltins.Keys) {
      st = KName::Builtin_Astral_Keys;
    } else if (builtin == AstralBuiltins.LastIndexOf) {
      st = KName::Builtin_Astral_LastIndexOf;
    } else if (builtin == AstralBuiltins.LeftTrim) {
      st = KName::Builtin_Astral_LeftTrim;
    } else if (builtin == AstralBuiltins.Members) {
      st = KName::Builtin_Astral_Members;
    } else if (builtin == AstralBuiltins.Replace) {
      st = KName::Builtin_Astral_Replace;
    } else if (builtin == AstralBuiltins.Reverse) {
      st = KName::Builtin_Astral_Reverse;
    } else if (builtin == AstralBuiltins.RightTrim) {
      st = KName::Builtin_Astral_RightTrim;
    } else if (builtin == AstralBuiltins.Size) {
      st = KName::Builtin_Astral_Size;
    } else if (builtin == AstralBuiltins.Split) {
      st = KName::Builtin_Astral_Split;
    } else if (builtin == AstralBuiltins.ToD) {
      st = KName::Builtin_Astral_ToD;
    } else if (builtin == AstralBuiltins.ToI) {
      st = KName::Builtin_Astral_ToI;
    } else if (builtin == AstralBuiltins.ToS) {
      st = KName::Builtin_Astral_ToS;
    } else if (builtin == AstralBuiltins.ToBytes) {
      st = KName::Builtin_Astral_ToBytes;
    } else if (builtin == AstralBuiltins.ToHex) {
      st = KName::Builtin_Astral_ToHex;
    } else if (builtin == AstralBuiltins.Trim) {
      st = KName::Builtin_Astral_Trim;
    } else if (builtin == AstralBuiltins.Type) {
      st = KName::Builtin_Astral_Type;
    } else if (builtin == AstralBuiltins.Upcase) {
      st = KName::Builtin_Astral_Upcase;
    } else if (builtin == ListBuiltins.Map) {
      st = KName::Builtin_List_Map;
    } else if (builtin == ListBuiltins.Select) {
      st = KName::Builtin_List_Select;
    } else if (builtin == ListBuiltins.Sort) {
      st = KName::Builtin_List_Sort;
    } else if (builtin == ListBuiltins.Reduce) {
      st = KName::Builtin_List_Reduce;
    } else if (builtin == ListBuiltins.None) {
      st = KName::Builtin_List_None;
    } else if (builtin == ListBuiltins.Sum) {
      st = KName::Builtin_List_Sum;
    } else if (builtin == ListBuiltins.Min) {
      st = KName::Builtin_List_Min;
    } else if (builtin == ListBuiltins.Max) {
      st = KName::Builtin_List_Max;
    } else if (builtin == ListBuiltins.ToH) {
      st = KName::Builtin_List_ToH;
    }

    return Token::create(KTokenType::IDENTIFIER, st, fileId, builtin, row, col);
  }

  Token parseIdentifier(const std::string& identifier) {
    auto st = KName::Default;
    return Token::create(KTokenType::IDENTIFIER, st, fileId, identifier, row,
                         col);
  }

  Token parseIdentifier(char initialChar) {
    std::string identifier(1, initialChar);
    char lastChar = '\0';

    if (pos > 2) {
      lastChar = source[pos - 2];
    }

    bool isCall = lastChar == '.';

    while (pos < source.length() &&
           (isalnum(source[pos]) || source[pos] == '_')) {
      identifier += getCurrentChar();
    }

    if (Keywords.is_keyword(identifier)) {
      return parseKeyword(identifier);
    } else if (TypeNames.is_typename(identifier)) {
      return parseTypeName(identifier);
    } else if (AstralBuiltins.is_builtin_method(identifier)) {
      return parseBuiltinMethod(identifier);
    } else if (isCall && AstralBuiltins.is_builtin(identifier)) {
      return parseAstralBuiltin(identifier);
    }

    return parseIdentifier(identifier);
  }

  Token parseLiteral(char initialChar) {
    std::string literal(1, initialChar);
    char lastChar = initialChar;

    while (pos < source.length() &&
           (isdigit(source[pos]) || source[pos] == '.')) {
      if (source[pos] == '.' && lastChar == '.') {
        literal.pop_back();
        --pos;
        break;
      }

      lastChar = source[pos];
      literal += getCurrentChar();
    }

    if (literal.find('.') != std::string::npos) {
      return Token::create(KTokenType::LITERAL, KName::Default, fileId, literal,
                           std::stod(literal), row, col);
    } else {
      std::istringstream ss(literal);
      k_int value;
      ss >> value;
      return Token::create(KTokenType::LITERAL, KName::Default, fileId, literal,
                           value, row, col);
    }
  }

  Token parseHexLiteral(char initialChar) {
    std::string hexLiteral(1, initialChar);
    getCurrentChar();  // Move past 'x'

    while (pos < source.length() && isxdigit(source[pos])) {
      hexLiteral += getCurrentChar();
    }

    std::istringstream ss(hexLiteral);
    k_int value;
    ss >> std::hex >> value;

    return Token::create(KTokenType::LITERAL, KName::Default, fileId,
                         hexLiteral, value, row, col);
  }

  Token parseString() {
    std::string str;
    bool escape = false;

    while (pos < source.length()) {
      char currentChar = source[pos];

      if (escape) {
        switch (currentChar) {
          case 'n':
            str += '\n';
            break;
          case 'r':
            str += '\r';
            break;
          case 't':
            str += '\t';
            break;
          case '\\':
            str += '\\';
            str += '\\';
            break;
          case 'b':
            str += '\b';
            break;
          case 'f':
            str += '\f';
            break;
          case '"':
            str += '"';
            break;
          default:
            str += currentChar;
        }
        escape = false;
      } else if (currentChar == '\\') {
        escape = true;
      } else if (currentChar == '"') {
        getCurrentChar();  // Move past the closing quote
        break;             // End of string
      } else if (currentChar == '$' && peek() == '{') {
        getCurrentChar();  // Skip '$'
        getCurrentChar();  // Skip '{'
        std::string interpolationExpression = parseInterpolatedExpression();
        str += interpolationExpression;
        continue;
      } else {
        str += currentChar;
      }

      getCurrentChar();
    }

    if (escape) {
      str += '\\';
    }

    return Token::create(KTokenType::STRING, KName::Default, fileId, str, row,
                         col);
  }

  std::string parseInterpolatedExpression() {
    std::string expression;
    int braceCount = 1;

    while (pos < source.length() && braceCount > 0) {
      char currentChar = getCurrentChar();

      if (currentChar == '}' && braceCount == 1) {
        --braceCount;
        break;
      } else if (currentChar == '{') {
        ++braceCount;
      } else if (currentChar == '}') {
        --braceCount;
      }

      if (braceCount > 0) {
        expression += currentChar;
      }
    }

    return "${" + expression + "}";
  }

  Token parseBlockComment() {
    std::string comment;
    pos++;  // Skip the "/#"

    while (pos + 1 < source.length()) {
      char currentChar = getCurrentChar();
      if (currentChar == '#' && source[pos] == '/') {
        pos++;  // Skip the "#/"
        break;
      } else {
        comment += currentChar;
      }
    }

    return Token::create(KTokenType::COMMENT, KName::Default, fileId, comment,
                         row, col);
  }

  Token parseComment() {
    std::string comment;

    while (pos < source.length() && source[pos] != '\n') {
      comment += getCurrentChar();
    }

    return Token::create(KTokenType::COMMENT, KName::Default, fileId, comment,
                         row, col);
  }
};

#endif
