#ifndef KIWI_PARSING_LEXER_H
#define KIWI_PARSING_LEXER_H

#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "k_int.h"
#include "parsing/builtins.h"
#include "parsing/keywords.h"
#include "parsing/tokens.h"
#include "parsing/tokentype.h"

class Lexer {
 public:
  Lexer(const std::string& file, const std::string& source, bool skipWS = true)
      : file(file), source(source), pos(0), skipWS(skipWS) {
    std::istringstream stream(source);
    std::string line;

    while (std::getline(stream, line)) {
      lines.push_back(line);
    }
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

      if (token.getType() == TokenType::ENDOFFILE) {
        break;
      }

      tokens.push_back(token);
    }

    return tokens;
  }

  std::vector<std::string> getLines() { return std::move(lines); }
  std::string getFile() { return file; }

 private:
  std::string file;
  std::string source;
  size_t pos;
  bool skipWS;
  int row;
  int col;
  std::vector<std::string> lines;

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
      return Token::create(TokenType::ENDOFFILE, SubTokenType::Default, file,
                           "", 0, row, col);
    }

    char currentChar = getCurrentChar();

    if (isalpha(currentChar) || currentChar == '_') {
      return parseIdentifier(currentChar);
    } else if (isdigit(currentChar)) {
      return parseLiteral(currentChar);
    } else if (currentChar == '"') {
      return parseString();
    } else if (currentChar == '#') {
      return parseComment();
    } else if (currentChar == '@') {
      return Token::create(TokenType::DECLVAR, SubTokenType::KW_DeclVar, file,
                           "@", row, col);
    } else if (currentChar == '$') {
      return Token::create(TokenType::OPERATOR, SubTokenType::Default, file,
                           "$", row, col);
    } else if (currentChar == '\n') {
      return Token::create(TokenType::NEWLINE, SubTokenType::Default, file,
                           "\n", row, col);
    } else if (currentChar == '(') {
      return Token::create(TokenType::OPEN_PAREN, SubTokenType::Default, file,
                           "(", row, col);
    } else if (currentChar == ')') {
      return Token::create(TokenType::CLOSE_PAREN, SubTokenType::Default, file,
                           ")", row, col);
    } else if (currentChar == '[') {
      return Token::create(TokenType::OPEN_BRACKET, SubTokenType::Default, file,
                           "[", row, col);
    } else if (currentChar == ']') {
      return Token::create(TokenType::CLOSE_BRACKET, SubTokenType::Default,
                           file, "]", row, col);
    } else if (currentChar == '{') {
      return Token::create(TokenType::OPEN_BRACE, SubTokenType::Default, file,
                           "{", row, col);
    } else if (currentChar == '}') {
      return Token::create(TokenType::CLOSE_BRACE, SubTokenType::Default, file,
                           "}", row, col);
    } else if (currentChar == ',') {
      return Token::create(TokenType::COMMA, SubTokenType::Default, file, ",",
                           row, col);
    } else if (currentChar == '?') {
      return Token::create(TokenType::QUESTION, SubTokenType::Default, file,
                           "?", row, col);
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
      return Token::create(TokenType::CONDITIONAL, SubTokenType::KW_If, file,
                           keyword, row, col);
    } else if (keyword == Keywords.ElseIf) {
      return Token::create(TokenType::CONDITIONAL, SubTokenType::KW_ElseIf,
                           file, keyword, row, col);
    } else if (keyword == Keywords.Else) {
      return Token::create(TokenType::CONDITIONAL, SubTokenType::KW_Else, file,
                           keyword, row, col);
    } else if (keyword == Keywords.End) {
      return Token::create(TokenType::CONDITIONAL, SubTokenType::KW_End, file,
                           keyword, row, col);
    }

    return Token::create(TokenType::CONDITIONAL, SubTokenType::Default, file,
                         keyword, row, col);
  }

  Token parseKeywordSpecific(const std::string& keyword) {
    auto st = SubTokenType::Default;

    if (keyword == Keywords.Abstract) {
      st = SubTokenType::KW_Abstract;
    } else if (keyword == Keywords.As) {
      st = SubTokenType::KW_As;
    } else if (keyword == Keywords.Break) {
      st = SubTokenType::KW_Break;
    } else if (keyword == Keywords.Catch) {
      st = SubTokenType::KW_Catch;
    } else if (keyword == Keywords.Class) {
      st = SubTokenType::KW_Class;
    } else if (keyword == Keywords.Delete) {
      st = SubTokenType::KW_Delete;
    } else if (keyword == Keywords.Do) {
      st = SubTokenType::KW_Do;
    } else if (keyword == Keywords.Exit) {
      st = SubTokenType::KW_Exit;
    } else if (keyword == Keywords.Export) {
      st = SubTokenType::KW_Export;
    } else if (keyword == Keywords.False) {
      st = SubTokenType::KW_False;
    } else if (keyword == Keywords.For) {
      st = SubTokenType::KW_For;
    } else if (keyword == Keywords.Import) {
      st = SubTokenType::KW_Import;
    } else if (keyword == Keywords.In) {
      st = SubTokenType::KW_In;
    } else if (keyword == Keywords.Method) {
      st = SubTokenType::KW_Method;
    } else if (keyword == Keywords.Module) {
      st = SubTokenType::KW_Module;
    } else if (keyword == Keywords.Next) {
      st = SubTokenType::KW_Next;
    } else if (keyword == Keywords.Override) {
      st = SubTokenType::KW_Override;
    } else if (keyword == Keywords.Pass) {
      st = SubTokenType::KW_Pass;
    } else if (keyword == Keywords.Print) {
      st = SubTokenType::KW_Print;
    } else if (keyword == Keywords.PrintLn) {
      st = SubTokenType::KW_PrintLn;
    } else if (keyword == Keywords.Private) {
      st = SubTokenType::KW_Private;
    } else if (keyword == Keywords.Return) {
      st = SubTokenType::KW_Return;
    } else if (keyword == Keywords.Static) {
      st = SubTokenType::KW_Static;
    } else if (keyword == Keywords.This) {
      st = SubTokenType::KW_This;
    } else if (keyword == Keywords.Try) {
      st = SubTokenType::KW_Try;
    } else if (keyword == Keywords.While) {
      st = SubTokenType::KW_While;
    }

    return Token::create(TokenType::KEYWORD, st, file, keyword, row, col);
  }

  Token parseKeyword(const std::string& keyword) {
    if (Keywords.is_conditional_keyword(keyword)) {
      return parseConditionalKeyword(keyword);
    } else if (keyword == Keywords.Lambda) {
      return Token::create(TokenType::LAMBDA, SubTokenType::KW_Lambda, file,
                           keyword, row, col);
    } else if (Keywords.is_boolean(keyword)) {
      return Token::createBoolean(file, keyword, row, col);
    }

    return parseKeywordSpecific(keyword);
  }

  Token parseOperator(const std::string& op) {
    auto st = SubTokenType::Default;

    if (op == Operators.Add) {
      st = SubTokenType::Ops_Add;
    } else if (op == Operators.AddAssign) {
      st = SubTokenType::Ops_AddAssign;
    } else if (op == Operators.And) {
      st = SubTokenType::Ops_And;
    } else if (op == Operators.AndAssign) {
      st = SubTokenType::Ops_AndAssign;
    } else if (op == Operators.Assign) {
      st = SubTokenType::Ops_Assign;
    } else if (op == Operators.BitwiseAnd) {
      st = SubTokenType::Ops_BitwiseAnd;
    } else if (op == Operators.BitwiseAndAssign) {
      st = SubTokenType::Ops_BitwiseAndAssign;
    } else if (op == Operators.BitwiseLeftShift) {
      st = SubTokenType::Ops_BitwiseLeftShift;
    } else if (op == Operators.BitwiseLeftShiftAssign) {
      st = SubTokenType::Ops_BitwiseLeftShiftAssign;
    } else if (op == Operators.BitwiseNot) {
      st = SubTokenType::Ops_BitwiseNot;
    } else if (op == Operators.BitwiseNotAssign) {
      st = SubTokenType::Ops_BitwiseNotAssign;
    } else if (op == Operators.BitwiseOr) {
      st = SubTokenType::Ops_BitwiseOr;
    } else if (op == Operators.BitwiseOrAssign) {
      st = SubTokenType::Ops_BitwiseOrAssign;
    } else if (op == Operators.BitwiseRightShift) {
      st = SubTokenType::Ops_BitwiseRightShift;
    } else if (op == Operators.BitwiseRightShiftAssign) {
      st = SubTokenType::Ops_BitwiseRightShiftAssign;
    } else if (op == Operators.BitwiseXor) {
      st = SubTokenType::Ops_BitwiseXor;
    } else if (op == Operators.BitwiseXorAssign) {
      st = SubTokenType::Ops_BitwiseXorAssign;
    } else if (op == Operators.Divide) {
      st = SubTokenType::Ops_Divide;
    } else if (op == Operators.DivideAssign) {
      st = SubTokenType::Ops_DivideAssign;
    } else if (op == Operators.Equal) {
      st = SubTokenType::Ops_Equal;
    } else if (op == Operators.Exponent) {
      st = SubTokenType::Ops_Exponent;
    } else if (op == Operators.ExponentAssign) {
      st = SubTokenType::Ops_ExponentAssign;
    } else if (op == Operators.GreaterThan) {
      st = SubTokenType::Ops_GreaterThan;
    } else if (op == Operators.GreaterThanOrEqual) {
      st = SubTokenType::Ops_GreaterThanOrEqual;
    } else if (op == Operators.LessThan) {
      st = SubTokenType::Ops_LessThan;
    } else if (op == Operators.LessThanOrEqual) {
      st = SubTokenType::Ops_LessThanOrEqual;
    } else if (op == Operators.ModuloAssign) {
      st = SubTokenType::Ops_ModuloAssign;
    } else if (op == Operators.Modulus) {
      st = SubTokenType::Ops_Modulus;
    } else if (op == Operators.Multiply) {
      st = SubTokenType::Ops_Multiply;
    } else if (op == Operators.MultiplyAssign) {
      st = SubTokenType::Ops_MultiplyAssign;
    } else if (op == Operators.Not) {
      st = SubTokenType::Ops_Not;
    } else if (op == Operators.NotEqual) {
      st = SubTokenType::Ops_NotEqual;
    } else if (op == Operators.Or) {
      st = SubTokenType::Ops_Or;
    } else if (op == Operators.OrAssign) {
      st = SubTokenType::Ops_OrAssign;
    } else if (op == Operators.Subtract) {
      st = SubTokenType::Ops_Subtract;
    } else if (op == Operators.SubtractAssign) {
      st = SubTokenType::Ops_SubtractAssign;
    }

    return Token::create(TokenType::OPERATOR, st, file, op, row, col);
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
        return Token::create(TokenType::QUALIFIER, SubTokenType::Default, file,
                             s, row, col);
      }
    }

    return Token::create(TokenType::COLON, SubTokenType::Default, file, s, row,
                         col);
  }

  Token parseEscapeCharacter() {
    if (pos < source.length()) {
      char nextChar = getCurrentChar();

      switch (nextChar) {
        case 'n':
          return Token::create(TokenType::ESCAPED, SubTokenType::Default, file,
                               "\n", row, col);
        case 'r':
          return Token::create(TokenType::ESCAPED, SubTokenType::Default, file,
                               "\r", row, col);
        case 't':
          return Token::create(TokenType::ESCAPED, SubTokenType::Default, file,
                               "\t", row, col);
      }
    }

    getCurrentChar();

    return Token::create(TokenType::ESCAPED, SubTokenType::Default, file, "\\",
                         row, col);
  }

  Token parseDot(char initialChar) {
    std::string s(1, initialChar);

    if (pos < source.length()) {
      char nextChar = source[pos];
      if (nextChar == '.') {
        s += nextChar;
        getCurrentChar();
        return Token::create(TokenType::RANGE, SubTokenType::Default, file, s,
                             row, col);
      }
    }

    return Token::create(TokenType::DOT, SubTokenType::Default, file, ".", row,
                         col);
  }

  Token parseTypeName(const std::string& typeName) {
    auto st = SubTokenType::Default;

    if (typeName == TypeNames.Integer) {
      st = SubTokenType::Types_Integer;
    } else if (typeName == TypeNames.Boolean) {
      st = SubTokenType::Types_Boolean;
    } else if (typeName == TypeNames.Double) {
      st = SubTokenType::Types_Double;
    } else if (typeName == TypeNames.Hash) {
      st = SubTokenType::Types_Hash;
    } else if (typeName == TypeNames.Lambda) {
      st = SubTokenType::Types_Lambda;
    } else if (typeName == TypeNames.List) {
      st = SubTokenType::Types_List;
    } else if (typeName == TypeNames.Object) {
      st = SubTokenType::Types_Object;
    } else if (typeName == TypeNames.String) {
      st = SubTokenType::Types_String;
    } else if (typeName == TypeNames.None) {
      st = SubTokenType::Types_None;
    }

    return Token::create(TokenType::TYPENAME, st, file, typeName, row, col);
  }

  Token parseArgvBuiltin(const std::string& builtin) {
    auto st = SubTokenType::Default;

    if (builtin == ArgvBuiltins.GetArgv) {
      st = SubTokenType::Builtin_Argv_GetArgv;
    } else if (builtin == ArgvBuiltins.GetXarg) {
      st = SubTokenType::Builtin_Argv_GetXarg;
    }

    return Token::create(TokenType::IDENTIFIER, st, file, builtin, row, col);
  }

  Token parseConsoleBuiltin(const std::string& builtin) {
    auto st = SubTokenType::Default;

    if (builtin == ConsoleBuiltins.Input) {
      st = SubTokenType::Builtin_Console_Input;
    } else if (builtin == ConsoleBuiltins.Silent) {
      st = SubTokenType::Builtin_Console_Silent;
    }

    return Token::create(TokenType::IDENTIFIER, st, file, builtin, row, col);
  }

  Token parseEnvBuiltin(const std::string& builtin) {
    auto st = SubTokenType::Default;

    if (builtin == EnvBuiltins.GetEnvironmentVariable) {
      st = SubTokenType::Builtin_Env_GetEnvironmentVariable;
    } else if (builtin == EnvBuiltins.SetEnvironmentVariable) {
      st = SubTokenType::Builtin_Env_SetEnvironmentVariable;
    }

    return Token::create(TokenType::IDENTIFIER, st, file, builtin, row, col);
  }

  Token parseFileIOBuiltin(const std::string& builtin) {
    auto st = SubTokenType::Default;

    if (builtin == FileIOBuiltIns.AppendText) {
      st = SubTokenType::Builtin_FileIO_AppendText;
    } else if (builtin == FileIOBuiltIns.ChangeDirectory) {
      st = SubTokenType::Builtin_FileIO_ChangeDirectory;
    } else if (builtin == FileIOBuiltIns.CopyFile) {
      st = SubTokenType::Builtin_FileIO_CopyFile;
    } else if (builtin == FileIOBuiltIns.CopyR) {
      st = SubTokenType::Builtin_FileIO_CopyR;
    } else if (builtin == FileIOBuiltIns.CreateFile) {
      st = SubTokenType::Builtin_FileIO_CreateFile;
    } else if (builtin == FileIOBuiltIns.DeleteFile) {
      st = SubTokenType::Builtin_FileIO_DeleteFile;
    } else if (builtin == FileIOBuiltIns.FileExists) {
      st = SubTokenType::Builtin_FileIO_FileExists;
    } else if (builtin == FileIOBuiltIns.FileName) {
      st = SubTokenType::Builtin_FileIO_FileName;
    } else if (builtin == FileIOBuiltIns.FileSize) {
      st = SubTokenType::Builtin_FileIO_FileSize;
    } else if (builtin == FileIOBuiltIns.GetCurrentDirectory) {
      st = SubTokenType::Builtin_FileIO_GetCurrentDirectory;
    } else if (builtin == FileIOBuiltIns.GetFileAbsolutePath) {
      st = SubTokenType::Builtin_FileIO_GetFileAbsolutePath;
    } else if (builtin == FileIOBuiltIns.GetFileAttributes) {
      st = SubTokenType::Builtin_FileIO_GetFileAttributes;
    } else if (builtin == FileIOBuiltIns.GetFileExtension) {
      st = SubTokenType::Builtin_FileIO_GetFileExtension;
    } else if (builtin == FileIOBuiltIns.GetFilePath) {
      st = SubTokenType::Builtin_FileIO_GetFilePath;
    } else if (builtin == FileIOBuiltIns.Glob) {
      st = SubTokenType::Builtin_FileIO_Glob;
    } else if (builtin == FileIOBuiltIns.IsDirectory) {
      st = SubTokenType::Builtin_FileIO_IsDirectory;
    } else if (builtin == FileIOBuiltIns.ListDirectory) {
      st = SubTokenType::Builtin_FileIO_ListDirectory;
    } else if (builtin == FileIOBuiltIns.MakeDirectory) {
      st = SubTokenType::Builtin_FileIO_MakeDirectory;
    } else if (builtin == FileIOBuiltIns.MakeDirectoryP) {
      st = SubTokenType::Builtin_FileIO_MakeDirectoryP;
    } else if (builtin == FileIOBuiltIns.MoveFile) {
      st = SubTokenType::Builtin_FileIO_MoveFile;
    } else if (builtin == FileIOBuiltIns.ReadFile) {
      st = SubTokenType::Builtin_FileIO_ReadFile;
    } else if (builtin == FileIOBuiltIns.ReadLines) {
      st = SubTokenType::Builtin_FileIO_ReadLines;
    } else if (builtin == FileIOBuiltIns.RemoveDirectory) {
      st = SubTokenType::Builtin_FileIO_RemoveDirectory;
    } else if (builtin == FileIOBuiltIns.RemoveDirectoryF) {
      st = SubTokenType::Builtin_FileIO_RemoveDirectoryF;
    } else if (builtin == FileIOBuiltIns.TempDir) {
      st = SubTokenType::Builtin_FileIO_TempDir;
    } else if (builtin == FileIOBuiltIns.WriteLine) {
      st = SubTokenType::Builtin_FileIO_WriteLine;
    } else if (builtin == FileIOBuiltIns.WriteText) {
      st = SubTokenType::Builtin_FileIO_WriteText;
    }

    return Token::create(TokenType::IDENTIFIER, st, file, builtin, row, col);
  }

  Token parseListBuiltin(const std::string& builtin) {
    auto st = SubTokenType::Default;

    if (builtin == ListBuiltins.Map) {
      st = SubTokenType::Builtin_List_Map;
    } else if (builtin == ListBuiltins.None) {
      st = SubTokenType::Builtin_List_None;
    } else if (builtin == ListBuiltins.Reduce) {
      st = SubTokenType::Builtin_List_Reduce;
    } else if (builtin == ListBuiltins.Select) {
      st = SubTokenType::Builtin_List_Select;
    } else if (builtin == ListBuiltins.Sort) {
      st = SubTokenType::Builtin_List_Sort;
    } else if (builtin == ListBuiltins.ToH) {
      st = SubTokenType::Builtin_List_ToH;
    }

    return Token::create(TokenType::IDENTIFIER, st, file, builtin, row, col);
  }

  Token parseMathBuiltin(const std::string& builtin) {
    auto st = SubTokenType::Default;

    if (builtin == MathBuiltins.Abs) {
      st = SubTokenType::Builtin_Math_Abs;
    } else if (builtin == MathBuiltins.Acos) {
      st = SubTokenType::Builtin_Math_Acos;
    } else if (builtin == MathBuiltins.Asin) {
      st = SubTokenType::Builtin_Math_Asin;
    } else if (builtin == MathBuiltins.Atan) {
      st = SubTokenType::Builtin_Math_Atan;
    } else if (builtin == MathBuiltins.Atan2) {
      st = SubTokenType::Builtin_Math_Atan2;
    } else if (builtin == MathBuiltins.Cbrt) {
      st = SubTokenType::Builtin_Math_Cbrt;
    } else if (builtin == MathBuiltins.Ceil) {
      st = SubTokenType::Builtin_Math_Ceil;
    } else if (builtin == MathBuiltins.CopySign) {
      st = SubTokenType::Builtin_Math_CopySign;
    } else if (builtin == MathBuiltins.Cos) {
      st = SubTokenType::Builtin_Math_Cos;
    } else if (builtin == MathBuiltins.Cosh) {
      st = SubTokenType::Builtin_Math_Cosh;
    } else if (builtin == MathBuiltins.Epsilon) {
      st = SubTokenType::Builtin_Math_Epsilon;
    } else if (builtin == MathBuiltins.Erf) {
      st = SubTokenType::Builtin_Math_Erf;
    } else if (builtin == MathBuiltins.ErfC) {
      st = SubTokenType::Builtin_Math_ErfC;
    } else if (builtin == MathBuiltins.Exp) {
      st = SubTokenType::Builtin_Math_Exp;
    } else if (builtin == MathBuiltins.ExpM1) {
      st = SubTokenType::Builtin_Math_ExpM1;
    } else if (builtin == MathBuiltins.FDim) {
      st = SubTokenType::Builtin_Math_FDim;
    } else if (builtin == MathBuiltins.Floor) {
      st = SubTokenType::Builtin_Math_Floor;
    } else if (builtin == MathBuiltins.FMax) {
      st = SubTokenType::Builtin_Math_FMax;
    } else if (builtin == MathBuiltins.FMin) {
      st = SubTokenType::Builtin_Math_FMin;
    } else if (builtin == MathBuiltins.Fmod) {
      st = SubTokenType::Builtin_Math_Fmod;
    } else if (builtin == MathBuiltins.Hypot) {
      st = SubTokenType::Builtin_Math_Hypot;
    } else if (builtin == MathBuiltins.IsFinite) {
      st = SubTokenType::Builtin_Math_IsFinite;
    } else if (builtin == MathBuiltins.IsInf) {
      st = SubTokenType::Builtin_Math_IsInf;
    } else if (builtin == MathBuiltins.IsNaN) {
      st = SubTokenType::Builtin_Math_IsNaN;
    } else if (builtin == MathBuiltins.IsNormal) {
      st = SubTokenType::Builtin_Math_IsNormal;
    } else if (builtin == MathBuiltins.LGamma) {
      st = SubTokenType::Builtin_Math_LGamma;
    } else if (builtin == MathBuiltins.Log) {
      st = SubTokenType::Builtin_Math_Log;
    } else if (builtin == MathBuiltins.Log10) {
      st = SubTokenType::Builtin_Math_Log10;
    } else if (builtin == MathBuiltins.Log1P) {
      st = SubTokenType::Builtin_Math_Log1P;
    } else if (builtin == MathBuiltins.Log2) {
      st = SubTokenType::Builtin_Math_Log2;
    } else if (builtin == MathBuiltins.NextAfter) {
      st = SubTokenType::Builtin_Math_NextAfter;
    } else if (builtin == MathBuiltins.Pow) {
      st = SubTokenType::Builtin_Math_Pow;
    } else if (builtin == MathBuiltins.Random) {
      st = SubTokenType::Builtin_Math_Random;
    } else if (builtin == MathBuiltins.Remainder) {
      st = SubTokenType::Builtin_Math_Remainder;
    } else if (builtin == MathBuiltins.Round) {
      st = SubTokenType::Builtin_Math_Round;
    } else if (builtin == MathBuiltins.Sin) {
      st = SubTokenType::Builtin_Math_Sin;
    } else if (builtin == MathBuiltins.Sinh) {
      st = SubTokenType::Builtin_Math_Sinh;
    } else if (builtin == MathBuiltins.Sqrt) {
      st = SubTokenType::Builtin_Math_Sqrt;
    } else if (builtin == MathBuiltins.Tan) {
      st = SubTokenType::Builtin_Math_Tan;
    } else if (builtin == MathBuiltins.Tanh) {
      st = SubTokenType::Builtin_Math_Tanh;
    } else if (builtin == MathBuiltins.TGamma) {
      st = SubTokenType::Builtin_Math_TGamma;
    } else if (builtin == MathBuiltins.Trunc) {
      st = SubTokenType::Builtin_Math_Trunc;
    }

    return Token::create(TokenType::IDENTIFIER, st, file, builtin, row, col);
  }

  Token parseModuleBuiltin(const std::string& builtin) {
    auto st = SubTokenType::Default;

    if (builtin == ModuleBuiltins.Home) {
      st = SubTokenType::Builtin_Module_Home;
    }

    return Token::create(TokenType::IDENTIFIER, st, file, builtin, row, col);
  }

  Token parseSysBuiltin(const std::string& builtin) {
    auto st = SubTokenType::Default;

    if (builtin == SysBuiltins.EffectiveUserId) {
      st = SubTokenType::Builtin_Sys_EffectiveUserId;
    } else if (builtin == SysBuiltins.Exec) {
      st = SubTokenType::Builtin_Sys_Exec;
    } else if (builtin == SysBuiltins.ExecOut) {
      st = SubTokenType::Builtin_Sys_ExecOut;
    }

    return Token::create(TokenType::IDENTIFIER, st, file, builtin, row, col);
  }

  Token parseTimeBuiltin(const std::string& builtin) {
    auto st = SubTokenType::Default;

    if (builtin == TimeBuiltins.AMPM) {
      st = SubTokenType::Builtin_Time_AMPM;
    } else if (builtin == TimeBuiltins.Delay) {
      st = SubTokenType::Builtin_Time_Delay;
    } else if (builtin == TimeBuiltins.EpochMilliseconds) {
      st = SubTokenType::Builtin_Time_EpochMilliseconds;
    } else if (builtin == TimeBuiltins.Hour) {
      st = SubTokenType::Builtin_Time_Hour;
    } else if (builtin == TimeBuiltins.IsDST) {
      st = SubTokenType::Builtin_Time_IsDST;
    } else if (builtin == TimeBuiltins.Minute) {
      st = SubTokenType::Builtin_Time_Minute;
    } else if (builtin == TimeBuiltins.Month) {
      st = SubTokenType::Builtin_Time_Month;
    } else if (builtin == TimeBuiltins.MonthDay) {
      st = SubTokenType::Builtin_Time_MonthDay;
    } else if (builtin == TimeBuiltins.Second) {
      st = SubTokenType::Builtin_Time_Second;
    } else if (builtin == TimeBuiltins.Ticks) {
      st = SubTokenType::Builtin_Time_Ticks;
    } else if (builtin == TimeBuiltins.TicksToMilliseconds) {
      st = SubTokenType::Builtin_Time_TicksToMilliseconds;
    } else if (builtin == TimeBuiltins.WeekDay) {
      st = SubTokenType::Builtin_Time_WeekDay;
    } else if (builtin == TimeBuiltins.Year) {
      st = SubTokenType::Builtin_Time_Year;
    } else if (builtin == TimeBuiltins.YearDay) {
      st = SubTokenType::Builtin_Time_YearDay;
    }

    return Token::create(TokenType::IDENTIFIER, st, file, builtin, row, col);
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
    }

    return Token::create(TokenType::IDENTIFIER, SubTokenType::Default, file,
                         builtin, row, col);
  }

  Token parseKiwiBuiltin(const std::string& builtin) {
    auto st = SubTokenType::Default;

    if (builtin == KiwiBuiltins.BeginsWith) {
      st = SubTokenType::Builtin_Kiwi_BeginsWith;
    } else if (builtin == KiwiBuiltins.Chars) {
      st = SubTokenType::Builtin_Kiwi_Chars;
    } else if (builtin == KiwiBuiltins.Contains) {
      st = SubTokenType::Builtin_Kiwi_Contains;
    } else if (builtin == KiwiBuiltins.Downcase) {
      st = SubTokenType::Builtin_Kiwi_Downcase;
    } else if (builtin == KiwiBuiltins.EndsWith) {
      st = SubTokenType::Builtin_Kiwi_EndsWith;
    } else if (builtin == KiwiBuiltins.HasKey) {
      st = SubTokenType::Builtin_Kiwi_HasKey;
    } else if (builtin == KiwiBuiltins.IndexOf) {
      st = SubTokenType::Builtin_Kiwi_IndexOf;
    } else if (builtin == KiwiBuiltins.IsA) {
      st = SubTokenType::Builtin_Kiwi_IsA;
    } else if (builtin == KiwiBuiltins.Join) {
      st = SubTokenType::Builtin_Kiwi_Join;
    } else if (builtin == KiwiBuiltins.Keys) {
      st = SubTokenType::Builtin_Kiwi_Keys;
    } else if (builtin == KiwiBuiltins.LeftTrim) {
      st = SubTokenType::Builtin_Kiwi_LeftTrim;
    } else if (builtin == KiwiBuiltins.Replace) {
      st = SubTokenType::Builtin_Kiwi_Replace;
    } else if (builtin == KiwiBuiltins.RightTrim) {
      st = SubTokenType::Builtin_Kiwi_RightTrim;
    } else if (builtin == KiwiBuiltins.Size) {
      st = SubTokenType::Builtin_Kiwi_Size;
    } else if (builtin == KiwiBuiltins.Split) {
      st = SubTokenType::Builtin_Kiwi_Split;
    } else if (builtin == KiwiBuiltins.ToD) {
      st = SubTokenType::Builtin_Kiwi_ToD;
    } else if (builtin == KiwiBuiltins.ToI) {
      st = SubTokenType::Builtin_Kiwi_ToI;
    } else if (builtin == KiwiBuiltins.ToS) {
      st = SubTokenType::Builtin_Kiwi_ToS;
    } else if (builtin == KiwiBuiltins.Trim) {
      st = SubTokenType::Builtin_Kiwi_Trim;
    } else if (builtin == KiwiBuiltins.Type) {
      st = SubTokenType::Builtin_Kiwi_Type;
    } else if (builtin == KiwiBuiltins.Upcase) {
      st = SubTokenType::Builtin_Kiwi_Upcase;
    } else if (builtin == ListBuiltins.Map) {
      st = SubTokenType::Builtin_List_Map;
    } else if (builtin == ListBuiltins.Select) {
      st = SubTokenType::Builtin_List_Select;
    } else if (builtin == ListBuiltins.Sort) {
      st = SubTokenType::Builtin_List_Sort;
    } else if (builtin == ListBuiltins.Reduce) {
      st = SubTokenType::Builtin_List_Reduce;
    } else if (builtin == ListBuiltins.None) {
      st = SubTokenType::Builtin_List_None;
    } else if (builtin == ListBuiltins.ToH) {
      st = SubTokenType::Builtin_List_ToH;
    }

    return Token::create(TokenType::IDENTIFIER, st, file, builtin, row, col);
  }

  Token parseIdentifier(const std::string& identifier) {
    auto st = SubTokenType::Default;
    return Token::create(TokenType::IDENTIFIER, st, file, identifier, row, col);
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
    } else if (KiwiBuiltins.is_builtin_method(identifier)) {
      return parseBuiltinMethod(identifier);
    } else if (isCall && KiwiBuiltins.is_builtin(identifier)) {
      return parseKiwiBuiltin(identifier);
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
      return Token::create(TokenType::LITERAL, SubTokenType::Default, file,
                           literal, std::stod(literal), row, col);
    } else {
      std::istringstream ss(literal);
      k_int value;
      ss >> value;
      return Token::create(TokenType::LITERAL, SubTokenType::Default, file,
                           literal, value, row, col);
    }
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

    return Token::create(TokenType::STRING, SubTokenType::Default, file, str,
                         row, col);
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

  Token parseComment() {
    if (pos + 1 < source.length() && source[pos] == '#') {
      // It's a multi-line comment.
      std::string comment;
      pos++;  // Skip the "##"

      while (pos + 1 < source.length()) {
        char currentChar = getCurrentChar();
        if (currentChar == '#' && source[pos] == '#') {
          pos++;  // Skip the "##"
          break;
        } else {
          comment += currentChar;
        }
      }

      return Token::create(TokenType::COMMENT, SubTokenType::Default, file,
                           comment, row, col);
    } else {
      // It's a single-line comment
      std::string comment;

      while (pos < source.length() && source[pos] != '\n') {
        comment += getCurrentChar();
      }

      return Token::create(TokenType::COMMENT, SubTokenType::Default, file,
                           comment, row, col);
    }
  }
};

#endif