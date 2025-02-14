#ifndef KIWI_PARSING_LEXER_H
#define KIWI_PARSING_LEXER_H

#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "parsing/builtins.h"
#include "parsing/keywords.h"
#include "parsing/tokens.h"
#include "parsing/tokentype.h"
#include "tracing/error.h"
#include "tracing/fileregistry.h"

const Token lexerToken = Token::createExternal();

class Lexer {
 public:
  Lexer(const k_string& file, const k_string& source, bool skipWS = true)
      : source(source), pos(0), skipWS(skipWS), row(0), col(0) {
    fileId = FileRegistry::getInstance().registerFile(file);
    preprocessSource();
  }

  Lexer(const int& file, const k_string& source, bool skipWS = true)
      : source(source), pos(0), skipWS(skipWS), row(0), col(0) {
    fileId = file;
    preprocessSource();
  }

  // Generate token stream from tokenized string input.
  k_stream getTokenStream() {
    return std::make_shared<TokenStream>(getAllTokens());
  }

  // Tokenize string input.
  std::vector<Token> getAllTokens() {
    std::vector<Token> tokens;
    row = 0;
    col = 0;

    return tokenize(tokens);
  }

  const std::vector<Token>& tokenize(std::vector<Token>& tokens) {
    // While we have tokens to process.
    while (true) {
      // Grab the next token.
      auto token = getNextToken();

      // If we're at EOF, exit the loop.
      if (token.getType() == KTokenType::ENDOFFILE) {
        break;
      }

      // Add to token vector.
      tokens.emplace_back(token);
    }

    return tokens;
  }

  static k_string minify(const k_string& path, bool forStandardOutput = false) {
    // Read the file, if it's empty, just return an empty string.
    auto content = File::readFile(lexerToken, path);
    if (content.empty()) {
      return content;
    }

    // Tokenize and return minified output.
    std::ostringstream builder;
    Lexer lexer(path, content);
    auto stream = Lexer(path, content).getTokenStream();
    minifyTokenStream(stream, forStandardOutput, builder);

    // Optionally print to standard output stream.
    if (forStandardOutput) {
      std::cout << String::trim(builder.str()) << std::endl;
    }

    return builder.str();
  }

  static void minifyTokenStream(k_stream& stream, bool forStandardOutput,
                                std::ostringstream& builder);

 private:
  k_string source;
  size_t pos;
  bool skipWS;
  int fileId;
  int row;
  int col;

  // String Processing
  void preprocessSource();
  void skipWhitespace();
  char getCurrentChar();
  char peek();

  // Tokenization
  Token getNextToken();
  Token createToken(KTokenType type, KName name, const k_string& text);
  Token createToken(KTokenType type, KName name, const k_string& text,
                    const k_value& value);

  // Identifiers
  Token tokenizeIdentifier(const k_string& identifier);
  Token tokenizeIdentifier(char initialChar);

  // Values
  Token tokenizeTypeName(const k_string& typeName);
  Token tokenizeNumericLiteral(char currentChar);
  Token tokenizeLiteral(char initialChar);
  Token tokenizeHexLiteral();
  Token tokenizeBinaryLiteral();
  Token tokenizeOctalLiteral();
  Token tokenizeRegex();
  Token tokenizeString();

  // Comments
  Token tokenizeComment();
  Token tokenizeBlockComment();

  // Keywords
  Token tokenizeKeyword(const k_string& keyword);
  Token tokenizeConditionalKeyword(const k_string& keyword);
  Token tokenizeKeywordSpecific(const k_string& keyword);

  // Operators
  Token tokenizeOperator(const k_string& op);
  Token tokenizeUnspecifiedOperator(char initialChar);

  // Symbols
  Token tokenizeEscapeCharacter();
  Token tokenizeColon(char initialChar);
  Token tokenizeDot(char initialChar);

  // Builtins
  Token tokenizeListBuiltin(const k_string& builtin);
  Token tokenizeBuiltinMethod(const k_string& builtin);
  Token tokenizeKiwiBuiltin(const k_string& builtin);

  // Special Builtins
  Token tokenizeArgvBuiltin(const k_string& builtin);
  Token tokenizeConsoleBuiltin(const k_string& builtin);
  Token tokenizeEncoderBuiltin(const k_string& builtin);
  Token tokenizeFFIBuiltin(const k_string& builtin);
  Token tokenizeSignalBuiltin(const k_string& builtin);
  Token tokenizeSocketBuiltin(const k_string& builtin);
  Token tokenizeReflectorBuiltin(const k_string& builtin);
  Token tokenizeSerializerBuiltin(const k_string& builtin);
  Token tokenizeEnvBuiltin(const k_string& builtin);
  Token tokenizeFileIOBuiltin(const k_string& builtin);
  Token tokenizeTaskBuiltin(const k_string& builtin);
  Token tokenizeMathBuiltin(const k_string& builtin);
  Token tokenizePackageBuiltin(const k_string& builtin);
  Token tokenizeSysBuiltin(const k_string& builtin);
  Token tokenizeWebClientBuiltin(const k_string& builtin);
  Token tokenizeWebServerBuiltin(const k_string& builtin);
  Token tokenizeLoggingBuiltin(const k_string& builtin);
  Token tokenizeTimeBuiltin(const k_string& builtin);

  // String Manipulation
  k_string tokenizeInterpolatedExpression();
};

/// =========================================================== ///
/// Lexer Implementation                                        ///
/// =========================================================== ///

void Lexer::minifyTokenStream(k_stream& stream, bool forStandardOutput,
                              std::ostringstream& builder) {
  bool addSpace = true;
  while (stream->canRead()) {
    auto token = stream->current();
    switch (token.getType()) {
      case KTokenType::COMMENT:
        stream->next();
        continue;

      case KTokenType::KEYWORD:
      case KTokenType::IDENTIFIER:
      case KTokenType::CONDITIONAL:
      case KTokenType::LITERAL:
        if (!forStandardOutput && addSpace) {
          builder << ' ';
        }

        builder << token.getText();

        if (forStandardOutput) {
          builder << std::endl;
        }

        addSpace = true;
        break;

      case KTokenType::STRING:
        if (!forStandardOutput && addSpace) {
          builder << ' ';
        }

        builder << '"' << token.getText() << '"';

        if (forStandardOutput) {
          builder << std::endl;
        }

        addSpace = true;
        break;

      default:
        addSpace = false;
        builder << token.getText();

        if (forStandardOutput) {
          builder << std::endl;
        }
        break;
    }

    stream->next();
  }
}

/// =========================================================== ///
/// String Processing                                           ///
/// =========================================================== ///

void Lexer::preprocessSource() {
  std::regex re(R"(\$\{([^}]+)\})");
  k_string output;

  std::regex_replace(std::back_inserter(output), source.begin(), source.end(),
                     re, R"(" + ($1).to_string() + ")");
  source = output;
}

void Lexer::skipWhitespace() {
  if (!skipWS) {
    return;
  }

  while (pos < source.length() && isspace(source[pos])) {
    getCurrentChar();
  }
}

char Lexer::getCurrentChar() {
  char c = source[pos++];
  if (c == '\n') {
    row++;
    col = 0;
  } else {
    col++;
  }
  return c;
}

char Lexer::peek() {
  if (pos + 1 < source.length()) {
    return source[pos + 1];
  }
  return '\0';
}

/// =========================================================== ///
/// Tokenization                                                ///
/// =========================================================== ///

Token Lexer::createToken(KTokenType type, KName name, const k_string& text) {
  return Token::create(type, name, fileId, text, row, col);
}

Token Lexer::createToken(KTokenType type, KName name, const k_string& text,
                         const k_value& value) {
  return Token::create(type, name, fileId, text, value, row, col);
}

Token Lexer::getNextToken() {
  skipWhitespace();

  if (pos >= source.length()) {
    return createToken(KTokenType::ENDOFFILE, KName::Default, "");
  }

  char currentChar = getCurrentChar();

  if (isalpha(currentChar) || currentChar == '_') {
    return tokenizeIdentifier(currentChar);
  } else if (isdigit(currentChar)) {
    return tokenizeNumericLiteral(currentChar);
  } else if (currentChar == '"') {
    return tokenizeString();
  } else if (currentChar == '\'') {
    return tokenizeRegex();
  } else if (currentChar == '#') {
    return tokenizeComment();
  } else if (currentChar == '/' &&
             (pos < source.length() && source[pos] == '#')) {
    return tokenizeBlockComment();
  } else if (currentChar == '@') {
    return createToken(KTokenType::KEYWORD, KName::KW_This, "@");
  } else if (currentChar == '$') {
    return createToken(KTokenType::OPERATOR, KName::Default, "$");
  } else if (currentChar == '\n') {
    return createToken(KTokenType::NEWLINE, KName::Default, "\n");
  } else if (currentChar == '(') {
    return createToken(KTokenType::OPEN_PAREN, KName::Default, "(");
  } else if (currentChar == ')') {
    return createToken(KTokenType::CLOSE_PAREN, KName::Default, ")");
  } else if (currentChar == '[') {
    return createToken(KTokenType::OPEN_BRACKET, KName::Default, "[");
  } else if (currentChar == ']') {
    return createToken(KTokenType::CLOSE_BRACKET, KName::Default, "]");
  } else if (currentChar == '{') {
    return createToken(KTokenType::OPEN_BRACE, KName::Default, "{");
  } else if (currentChar == '}') {
    return createToken(KTokenType::CLOSE_BRACE, KName::Default, "}");
  } else if (currentChar == ',') {
    return createToken(KTokenType::COMMA, KName::Default, ",");
  } else if (currentChar == '?') {
    return createToken(KTokenType::QUESTION, KName::Default, "?");
  } else if (currentChar == '.') {
    return tokenizeDot(currentChar);
  } else if (currentChar == '\\') {
    return tokenizeEscapeCharacter();
  } else if (currentChar == ':') {
    return tokenizeColon(currentChar);
  } else {
    return tokenizeUnspecifiedOperator(currentChar);
  }
}

/// =========================================================== ///
/// Identifiers                                                 ///
/// =========================================================== ///

Token Lexer::tokenizeIdentifier(const k_string& identifier) {
  auto st = KName::Default;

  return createToken(KTokenType::IDENTIFIER, st, identifier);
}

Token Lexer::tokenizeIdentifier(char initialChar) {
  k_string identifier(1, initialChar);
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
    return tokenizeKeyword(identifier);
  } else if (TypeNames.is_typename(identifier)) {
    return tokenizeTypeName(identifier);
  } else if (KiwiBuiltins.is_builtin_method(identifier)) {
    return tokenizeBuiltinMethod(identifier);
  } else if (isCall && KiwiBuiltins.is_builtin(identifier)) {
    return tokenizeKiwiBuiltin(identifier);
  }

  return tokenizeIdentifier(identifier);
}

/// =========================================================== ///
/// Values                                                      ///
/// =========================================================== ///

Token Lexer::tokenizeTypeName(const k_string& typeName) {
  auto st = KName::Default;

  if (typeName == TypeNames.Integer || typeName == TypeNames.LowInteger) {
    st = KName::Types_Integer;
  } else if (typeName == TypeNames.Boolean ||
             typeName == TypeNames.LowBoolean) {
    st = KName::Types_Boolean;
  } else if (typeName == TypeNames.Float || typeName == TypeNames.LowFloat) {
    st = KName::Types_Float;
  } else if (typeName == TypeNames.Hashmap || typeName == TypeNames.LowHash) {
    st = KName::Types_Hash;
  } else if (typeName == TypeNames.Lambda || typeName == TypeNames.LowLambda) {
    st = KName::Types_Lambda;
  } else if (typeName == TypeNames.List || typeName == TypeNames.LowList) {
    st = KName::Types_List;
  } else if (typeName == TypeNames.Object || typeName == TypeNames.LowObject) {
    st = KName::Types_Object;
  } else if (typeName == TypeNames.String || typeName == TypeNames.LowString) {
    st = KName::Types_String;
  } else if (typeName == TypeNames.None || typeName == TypeNames.LowNone) {
    st = KName::Types_None;
  } else if (typeName == TypeNames.Any || typeName == TypeNames.LowAny) {
    st = KName::Types_Any;
  } else if (typeName == TypeNames.Pointer ||
             typeName == TypeNames.LowPointer) {
    st = KName::Types_Pointer;
  }

  return createToken(KTokenType::TYPENAME, st, typeName);
}

Token Lexer::tokenizeNumericLiteral(char currentChar) {
  if (currentChar == '0' && (pos < source.length() && source[pos] == 'x')) {
    return tokenizeHexLiteral();
  } else if (currentChar == '0' &&
             (pos < source.length() && source[pos] == 'b')) {
    return tokenizeBinaryLiteral();
  } else if (currentChar == '0' &&
             (pos < source.length() && source[pos] == 'o')) {
    return tokenizeOctalLiteral();
  }
  return tokenizeLiteral(currentChar);
}

Token Lexer::tokenizeLiteral(char initialChar) {
  k_string literal(1, initialChar);
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

  if (literal.find('.') != k_string::npos) {
    return createToken(KTokenType::LITERAL, KName::Default, literal,
                       std::stod(literal));
  } else {
    std::istringstream ss(literal);
    k_int value;
    ss >> value;
    return createToken(KTokenType::LITERAL, KName::Default, literal, value);
  }
}

Token Lexer::tokenizeHexLiteral() {
  k_string hexLiteral;
  getCurrentChar();  // Move past 'x'

  while (pos < source.length() && isxdigit(source[pos])) {
    hexLiteral += getCurrentChar();
  }

  std::istringstream ss(hexLiteral);
  k_int value;
  ss >> std::hex >> value;

  return createToken(KTokenType::LITERAL, KName::Default, hexLiteral, value);
}

Token Lexer::tokenizeBinaryLiteral() {
  k_string binaryLiteral;
  getCurrentChar();  // Move past 'b'

  while (pos < source.length() && (source[pos] == '0' || source[pos] == '1')) {
    binaryLiteral += getCurrentChar();
  }

  if (binaryLiteral.empty()) {
    const auto& errorToken = createToken(KTokenType::LITERAL, KName::Default,
                                         binaryLiteral, static_cast<k_int>(0));
    throw SyntaxError(errorToken, "Invalid binary literal.");
  }

  k_int value = std::stoi(binaryLiteral, nullptr, 2);

  return createToken(KTokenType::LITERAL, KName::Default, binaryLiteral, value);
}

Token Lexer::tokenizeOctalLiteral() {
  k_string octalLiteral;
  getCurrentChar();  // Move past 'o'

  while (pos < source.length() && source[pos] >= '0' && source[pos] <= '7') {
    octalLiteral += getCurrentChar();
  }

  if (octalLiteral.empty()) {
    const auto& errorToken = createToken(KTokenType::LITERAL, KName::Default,
                                         octalLiteral, static_cast<k_int>(0));
    throw SyntaxError(errorToken, "Invalid octal literal.");
  }

  k_int value = std::stoi(octalLiteral, nullptr, 8);

  return createToken(KTokenType::LITERAL, KName::Default, octalLiteral, value);
}

Token Lexer::tokenizeRegex() {
  k_string regexPattern;
  bool escape = false;

  while (pos < source.length()) {
    char currentChar = getCurrentChar();

    if (escape) {
      switch (currentChar) {
        case 'n':
          regexPattern += '\n';
          break;
        case 't':
          regexPattern += '\t';
          break;
        case 'r':
          // Handle both '\r' and '\r\n' for Windows line endings
          if (pos + 1 < source.length() && source[pos + 1] == '\n') {
            pos++;  // Skip the '\n' after '\r'
          }
          regexPattern += '\r';
          break;
        case '\\':
          regexPattern += '\\';
          break;
        case '/':
          regexPattern += '/';
          break;
        default:
          regexPattern += "\\" + k_string(1, currentChar);
      }
      escape = false;
    } else if (currentChar == '\\') {
      escape = true;
    } else if (currentChar == '\'') {
      if (!escape) {
        break;
      }
      regexPattern += currentChar;
    } else if (currentChar == '\r') {
      // Handle stray '\r' for Windows line endings
      if (pos + 1 < source.length() && source[pos + 1] == '\n') {
        pos++;  // Skip the '\n' after '\r'
      }
      regexPattern += '\n';
    } else {
      regexPattern += currentChar;
    }
  }

  return createToken(KTokenType::STRING, KName::Regex, regexPattern);
}

Token Lexer::tokenizeString() {
  k_string str;
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
          str += '\\';  // Retain the escape character
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
      k_string interpolationExpression = tokenizeInterpolatedExpression();
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

  return createToken(KTokenType::STRING, KName::Default, str);
}

/// =========================================================== ///
/// Comments                                                    ///
/// =========================================================== ///

Token Lexer::tokenizeComment() {
  k_string comment;

  while (pos < source.length() && source[pos] != '\n') {
    comment += getCurrentChar();
  }

  return createToken(KTokenType::COMMENT, KName::Default, comment);
}

Token Lexer::tokenizeBlockComment() {
  k_string comment;
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

  return createToken(KTokenType::COMMENT, KName::Default, comment);
}

/// =========================================================== ///
/// Symbols                                                     ///
/// =========================================================== ///

Token Lexer::tokenizeEscapeCharacter() {
  if (pos < source.length()) {
    char nextChar = getCurrentChar();

    switch (nextChar) {
      case 'n':
        return createToken(KTokenType::ESCAPED, KName::Default, "\n");
      case 'r':
        return createToken(KTokenType::ESCAPED, KName::Default, "\r");
      case 't':
        return createToken(KTokenType::ESCAPED, KName::Default, "\t");
      case '"':
        return createToken(KTokenType::ESCAPED, KName::Default, "\"");
      case 'b':
        return createToken(KTokenType::ESCAPED, KName::Default, "\b");
      case 'f':
        return createToken(KTokenType::ESCAPED, KName::Default, "\f");
      case '\\':
        return createToken(KTokenType::ESCAPED, KName::Default, "\\");
    }
  }

  getCurrentChar();

  return createToken(KTokenType::ESCAPED, KName::Default, "\\");
}

Token Lexer::tokenizeColon(char initialChar) {
  k_string s(1, initialChar);

  if (pos < source.length()) {
    char nextChar = source[pos];
    if (nextChar == ':') {
      s += nextChar;
      getCurrentChar();
      return createToken(KTokenType::QUALIFIER, KName::Default, s);
    }
  }

  return createToken(KTokenType::COLON, KName::Default, s);
}

Token Lexer::tokenizeDot(char initialChar) {
  k_string s(1, initialChar);

  if (pos < source.length()) {
    char nextChar = source[pos];
    if (nextChar == '.') {
      s += nextChar;
      getCurrentChar();
      return createToken(KTokenType::RANGE, KName::Default, s);
    }
  }

  return createToken(KTokenType::DOT, KName::Default, ".");
}

/// =========================================================== ///
/// Operators                                                   ///
/// =========================================================== ///

Token Lexer::tokenizeOperator(const k_string& op) {
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
  } else if (op == Operators.BitwiseUnsignedRightShift) {
    st = KName::Ops_BitwiseUnsignedRightShift;
  } else if (op == Operators.BitwiseUnsignedRightShiftAssign) {
    st = KName::Ops_BitwiseUnsignedRightShiftAssign;
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

  return createToken(KTokenType::OPERATOR, st, op);
}

Token Lexer::tokenizeUnspecifiedOperator(char initialChar) {
  k_string s(1, initialChar);

  if (pos < source.length()) {
    char nextChar = source[pos];
    bool isArithmeticOpChar =
        Operators.is_arithmetic_operator_char(initialChar);
    bool isBooleanOpChar = Operators.is_boolean_operator_char(initialChar);
    bool isArithmeticOp =
        (nextChar == '=' && (isArithmeticOpChar || isBooleanOpChar)) ||
        (initialChar == '*' && nextChar == '*');
    bool isBooleanOp = (nextChar == '|' || nextChar == '&') && isBooleanOpChar;
    bool isBitwiseOp =
        (Operators.is_bitwise_operator_char(initialChar) && nextChar == '=') ||
        (initialChar == '<' && nextChar == '<') ||
        (initialChar == '>' && nextChar == '>');

    if (isArithmeticOp || isBooleanOp || isBitwiseOp) {
      s += nextChar;
      getCurrentChar();

      nextChar = source[pos];
      if ((nextChar == '=' && Operators.is_large_operator(s)) ||
          (nextChar == '>' && s == Operators.BitwiseRightShift)) {
        s += nextChar;
        getCurrentChar();

        nextChar = source[pos];
        if (nextChar == '=' && s == Operators.BitwiseUnsignedRightShift) {
          s += nextChar;
          getCurrentChar();
        }
      }
    }
  }

  return tokenizeOperator(s);
}

/// =========================================================== ///
/// String manipulation                                         ///
/// =========================================================== ///

k_string Lexer::tokenizeInterpolatedExpression() {
  k_string expression;
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

/// =========================================================== ///
/// Keywords                                                    ///
/// =========================================================== ///

Token Lexer::tokenizeKeyword(const k_string& keyword) {
  if (Keywords.is_conditional_keyword(keyword)) {
    return tokenizeConditionalKeyword(keyword);
  } else if (keyword == Keywords.With) {
    return createToken(KTokenType::LAMBDA, KName::KW_Lambda, keyword);
  } else if (Keywords.is_boolean(keyword)) {
    return Token::createBoolean(fileId, keyword, row, col);
  } else if (Keywords.is_null(keyword)) {
    return Token::createNull(fileId, keyword, row, col);
  }

  return tokenizeKeywordSpecific(keyword);
}

Token Lexer::tokenizeConditionalKeyword(const k_string& keyword) {
  auto st = KName::Default;

  if (keyword == Keywords.If) {
    st = KName::KW_If;
  } else if (keyword == Keywords.ElseIf) {
    st = KName::KW_ElseIf;
  } else if (keyword == Keywords.Else) {
    st = KName::KW_Else;
  } else if (keyword == Keywords.End) {
    st = KName::KW_End;
  } else if (keyword == Keywords.Case) {
    st = KName::KW_Case;
  }

  return createToken(KTokenType::CONDITIONAL, st, keyword);
}

Token Lexer::tokenizeKeywordSpecific(const k_string& keyword) {
  auto st = KName::Default;

  if (keyword == Keywords.Abstract) {
    st = KName::KW_Abstract;
  } else if (keyword == Keywords.As) {
    st = KName::KW_As;
  } else if (keyword == Keywords.Break) {
    st = KName::KW_Break;
  } else if (keyword == Keywords.Catch) {
    st = KName::KW_Catch;
  } else if (keyword == Keywords.Const) {
    st = KName::KW_Const;
  } else if (keyword == Keywords.Do) {
    st = KName::KW_Do;
  } else if (keyword == Keywords.EPrint) {
    st = KName::KW_EPrint;
  } else if (keyword == Keywords.EPrintLn) {
    st = KName::KW_EPrintLn;
  } else if (keyword == Keywords.Exit) {
    st = KName::KW_Exit;
  } else if (keyword == Keywords.Export) {
    st = KName::KW_Export;
  } else if (keyword == Keywords.False) {
    st = KName::KW_False;
  } else if (keyword == Keywords.Finally) {
    st = KName::KW_Finally;
  } else if (keyword == Keywords.For) {
    st = KName::KW_For;
  } else if (keyword == Keywords.Spawn) {
    st = KName::KW_Spawn;
  } else if (keyword == Keywords.Import) {
    st = KName::KW_Import;
  } else if (keyword == Keywords.In) {
    st = KName::KW_In;
  } else if (keyword == Keywords.Interface) {
    st = KName::KW_Interface;
  } else if (keyword == Keywords.Method || keyword == Keywords.Function) {
    st = KName::KW_Method;
  } else if (keyword == Keywords.Package) {
    st = KName::KW_Package;
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
  } else if (keyword == Keywords.PrintXy) {
    st = KName::KW_PrintXy;
  } else if (keyword == Keywords.Private) {
    st = KName::KW_Private;
  } else if (keyword == Keywords.Repeat) {
    st = KName::KW_Repeat;
  } else if (keyword == Keywords.Return) {
    st = KName::KW_Return;
  } else if (keyword == Keywords.Static) {
    st = KName::KW_Static;
  } else if (keyword == Keywords.Struct) {
    st = KName::KW_Struct;
  } else if (keyword == Keywords.This) {
    st = KName::KW_This;
  } else if (keyword == Keywords.Throw) {
    st = KName::KW_Throw;
  } else if (keyword == Keywords.Try) {
    st = KName::KW_Try;
  } else if (keyword == Keywords.Var) {
    st = KName::KW_Var;
  } else if (keyword == Keywords.When) {
    st = KName::KW_When;
  } else if (keyword == Keywords.While) {
    st = KName::KW_While;
  }

  return createToken(KTokenType::KEYWORD, st, keyword);
}

/// =========================================================== ///
/// Value Builtins                                              ///
/// =========================================================== ///

Token Lexer::tokenizeKiwiBuiltin(const k_string& builtin) {
  auto st = KName::Default;

  if (builtin == KiwiBuiltins.BeginsWith) {
    st = KName::Builtin_Kiwi_BeginsWith;
  } else if (builtin == KiwiBuiltins.Chars) {
    st = KName::Builtin_Kiwi_Chars;
  } else if (builtin == KiwiBuiltins.Contains) {
    st = KName::Builtin_Kiwi_Contains;
  } else if (builtin == KiwiBuiltins.Lowercase) {
    st = KName::Builtin_Kiwi_Lowercase;
  } else if (builtin == KiwiBuiltins.Empty) {
    st = KName::Builtin_Kiwi_Empty;
  } else if (builtin == KiwiBuiltins.EndsWith) {
    st = KName::Builtin_Kiwi_EndsWith;
  } else if (builtin == KiwiBuiltins.HasKey) {
    st = KName::Builtin_Kiwi_HasKey;
  } else if (builtin == KiwiBuiltins.IndexOf) {
    st = KName::Builtin_Kiwi_IndexOf;
  } else if (builtin == KiwiBuiltins.IsA) {
    st = KName::Builtin_Kiwi_IsA;
  } else if (builtin == KiwiBuiltins.Join) {
    st = KName::Builtin_Kiwi_Join;
  } else if (builtin == KiwiBuiltins.Keys) {
    st = KName::Builtin_Kiwi_Keys;
  } else if (builtin == KiwiBuiltins.Merge) {
    st = KName::Builtin_Kiwi_Merge;
  } else if (builtin == KiwiBuiltins.Values) {
    st = KName::Builtin_Kiwi_Values;
  } else if (builtin == KiwiBuiltins.LastIndexOf) {
    st = KName::Builtin_Kiwi_LastIndexOf;
  } else if (builtin == KiwiBuiltins.LeftTrim) {
    st = KName::Builtin_Kiwi_LeftTrim;
  } else if (builtin == KiwiBuiltins.Members) {
    st = KName::Builtin_Kiwi_Members;
  } else if (builtin == KiwiBuiltins.Replace) {
    st = KName::Builtin_Kiwi_Replace;
  } else if (builtin == KiwiBuiltins.RReplace) {
    st = KName::Builtin_Kiwi_RReplace;
  } else if (builtin == KiwiBuiltins.Reverse) {
    st = KName::Builtin_Kiwi_Reverse;
  } else if (builtin == KiwiBuiltins.RightTrim) {
    st = KName::Builtin_Kiwi_RightTrim;
  } else if (builtin == KiwiBuiltins.Size) {
    st = KName::Builtin_Kiwi_Size;
  } else if (builtin == KiwiBuiltins.RSplit) {
    st = KName::Builtin_Kiwi_RSplit;
  } else if (builtin == KiwiBuiltins.Split) {
    st = KName::Builtin_Kiwi_Split;
  } else if (builtin == KiwiBuiltins.Substring) {
    st = KName::Builtin_Kiwi_Substring;
  } else if (builtin == KiwiBuiltins.ToD) {
    st = KName::Builtin_Kiwi_ToD;
  } else if (builtin == KiwiBuiltins.ToI) {
    st = KName::Builtin_Kiwi_ToI;
  } else if (builtin == KiwiBuiltins.ToS) {
    st = KName::Builtin_Kiwi_ToS;
  } else if (builtin == KiwiBuiltins.ToBytes) {
    st = KName::Builtin_Kiwi_ToBytes;
  } else if (builtin == KiwiBuiltins.ToHex) {
    st = KName::Builtin_Kiwi_ToHex;
  } else if (builtin == KiwiBuiltins.Trim) {
    st = KName::Builtin_Kiwi_Trim;
  } else if (builtin == KiwiBuiltins.Type) {
    st = KName::Builtin_Kiwi_Type;
  } else if (builtin == KiwiBuiltins.Uppercase) {
    st = KName::Builtin_Kiwi_Uppercase;
  } else if (builtin == KiwiBuiltins.Append || builtin == KiwiBuiltins.Push) {
    st = KName::Builtin_Kiwi_Push;
  } else if (builtin == KiwiBuiltins.Pop) {
    st = KName::Builtin_Kiwi_Pop;
  } else if (builtin == KiwiBuiltins.Dequeue) {
    st = KName::Builtin_Kiwi_Dequeue;
  } else if (builtin == KiwiBuiltins.Enqueue) {
    st = KName::Builtin_Kiwi_Enqueue;
  } else if (builtin == KiwiBuiltins.Shift) {
    st = KName::Builtin_Kiwi_Shift;
  } else if (builtin == KiwiBuiltins.Unshift) {
    st = KName::Builtin_Kiwi_Unshift;
  } else if (builtin == KiwiBuiltins.Remove) {
    st = KName::Builtin_Kiwi_Remove;
  } else if (builtin == KiwiBuiltins.RemoveAt) {
    st = KName::Builtin_Kiwi_RemoveAt;
  } else if (builtin == KiwiBuiltins.Rotate) {
    st = KName::Builtin_Kiwi_Rotate;
  } else if (builtin == KiwiBuiltins.Insert) {
    st = KName::Builtin_Kiwi_Insert;
  } else if (builtin == KiwiBuiltins.Slice) {
    st = KName::Builtin_Kiwi_Slice;
  } else if (builtin == KiwiBuiltins.Concat) {
    st = KName::Builtin_Kiwi_Concat;
  } else if (builtin == KiwiBuiltins.Unique) {
    st = KName::Builtin_Kiwi_Unique;
  } else if (builtin == KiwiBuiltins.Count) {
    st = KName::Builtin_Kiwi_Count;
  } else if (builtin == KiwiBuiltins.Flatten) {
    st = KName::Builtin_Kiwi_Flatten;
  } else if (builtin == KiwiBuiltins.Zip) {
    st = KName::Builtin_Kiwi_Zip;
  } else if (builtin == KiwiBuiltins.Clear) {
    st = KName::Builtin_Kiwi_Clear;
  } else if (builtin == KiwiBuiltins.Clone) {
    st = KName::Builtin_Kiwi_Clone;
  } else if (builtin == KiwiBuiltins.Pretty) {
    st = KName::Builtin_Kiwi_Pretty;
  } else if (builtin == KiwiBuiltins.Find) {
    st = KName::Builtin_Kiwi_Find;
  } else if (builtin == KiwiBuiltins.Match) {
    st = KName::Builtin_Kiwi_Match;
  } else if (builtin == KiwiBuiltins.Matches) {
    st = KName::Builtin_Kiwi_Matches;
  } else if (builtin == KiwiBuiltins.MatchesAll) {
    st = KName::Builtin_Kiwi_MatchesAll;
  } else if (builtin == KiwiBuiltins.Scan) {
    st = KName::Builtin_Kiwi_Scan;
  } else if (builtin == KiwiBuiltins.Get) {
    st = KName::Builtin_Kiwi_Get;
  } else if (builtin == KiwiBuiltins.Set) {
    st = KName::Builtin_Kiwi_Set;
  } else if (builtin == KiwiBuiltins.Swap) {
    st = KName::Builtin_Kiwi_Swap;
  } else if (builtin == KiwiBuiltins.First) {
    st = KName::Builtin_Kiwi_First;
  } else if (builtin == KiwiBuiltins.Last) {
    st = KName::Builtin_Kiwi_Last;
  } else if (builtin == KiwiBuiltins.Truthy) {
    st = KName::Builtin_Kiwi_Truthy;
  } else if (builtin == KiwiBuiltins.Lines) {
    st = KName::Builtin_Kiwi_Lines;
  } else if (builtin == KiwiBuiltins.Tokens) {
    st = KName::Builtin_Kiwi_Tokens;
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
  } else if (builtin == ListBuiltins.Each) {
    st = KName::Builtin_List_Each;
  } else if (builtin == ListBuiltins.All) {
    st = KName::Builtin_List_All;
  }

  return createToken(KTokenType::IDENTIFIER, st, builtin);
}

Token Lexer::tokenizeListBuiltin(const k_string& builtin) {
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
  } else if (builtin == ListBuiltins.All) {
    st = KName::Builtin_List_All;
  }

  return createToken(KTokenType::IDENTIFIER, st, builtin);
}

/// =========================================================== ///
/// Builtins                                                    ///
/// =========================================================== ///

Token Lexer::tokenizeBuiltinMethod(const k_string& builtin) {
  if (ArgvBuiltins.is_builtin(builtin)) {
    return tokenizeArgvBuiltin(builtin);
  } else if (ConsoleBuiltins.is_builtin(builtin)) {
    return tokenizeConsoleBuiltin(builtin);
  } else if (EnvBuiltins.is_builtin(builtin)) {
    return tokenizeEnvBuiltin(builtin);
  } else if (FileIOBuiltIns.is_builtin(builtin)) {
    return tokenizeFileIOBuiltin(builtin);
  } else if (LoggingBuiltins.is_builtin(builtin)) {
    return tokenizeLoggingBuiltin(builtin);
  } else if (ListBuiltins.is_builtin(builtin)) {
    return tokenizeListBuiltin(builtin);
  } else if (MathBuiltins.is_builtin(builtin)) {
    return tokenizeMathBuiltin(builtin);
  } else if (TaskBuiltins.is_builtin(builtin)) {
    return tokenizeTaskBuiltin(builtin);
  } else if (PackageBuiltins.is_builtin(builtin)) {
    return tokenizePackageBuiltin(builtin);
  } else if (SysBuiltins.is_builtin(builtin)) {
    return tokenizeSysBuiltin(builtin);
  } else if (TimeBuiltins.is_builtin(builtin)) {
    return tokenizeTimeBuiltin(builtin);
  } else if (WebServerBuiltins.is_builtin(builtin)) {
    return tokenizeWebServerBuiltin(builtin);
  } else if (HttpBuiltins.is_builtin(builtin)) {
    return tokenizeWebClientBuiltin(builtin);
  } else if (EncoderBuiltins.is_builtin(builtin)) {
    return tokenizeEncoderBuiltin(builtin);
  } else if (SerializerBuiltins.is_builtin(builtin)) {
    return tokenizeSerializerBuiltin(builtin);
  } else if (ReflectorBuiltins.is_builtin(builtin)) {
    return tokenizeReflectorBuiltin(builtin);
  } else if (FFIBuiltins.is_builtin(builtin)) {
    return tokenizeFFIBuiltin(builtin);
  } else if (SocketBuiltins.is_builtin(builtin)) {
    return tokenizeSocketBuiltin(builtin);
  } else if (SignalBuiltins.is_builtin(builtin)) {
    return tokenizeSignalBuiltin(builtin);
  }

  return createToken(KTokenType::IDENTIFIER, KName::Default, builtin);
}

/// =========================================================== ///
/// Special Builtins                                            ///
/// =========================================================== ///

Token Lexer::tokenizeArgvBuiltin(const k_string& builtin) {
  auto st = KName::Default;

  if (builtin == ArgvBuiltins.GetArgv) {
    st = KName::Builtin_Argv_GetArgv;
  } else if (builtin == ArgvBuiltins.GetXarg) {
    st = KName::Builtin_Argv_GetXarg;
  }

  return createToken(KTokenType::IDENTIFIER, st, builtin);
}

Token Lexer::tokenizeConsoleBuiltin(const k_string& builtin) {
  auto st = KName::Default;

  if (builtin == ConsoleBuiltins.Input) {
    st = KName::Builtin_Console_Input;
  }

  return createToken(KTokenType::IDENTIFIER, st, builtin);
}

Token Lexer::tokenizeEncoderBuiltin(const k_string& builtin) {
  auto st = KName::Default;

  if (builtin == EncoderBuiltins.Base64Decode) {
    st = KName::Builtin_Encoder_Base64Decode;
  } else if (builtin == EncoderBuiltins.Base64Encode) {
    st = KName::Builtin_Encoder_Base64Encode;
  } else if (builtin == EncoderBuiltins.UrlDecode) {
    st = KName::Builtin_Encoder_UrlDecode;
  } else if (builtin == EncoderBuiltins.UrlEncode) {
    st = KName::Builtin_Encoder_UrlEncode;
  }

  return createToken(KTokenType::IDENTIFIER, st, builtin);
}

Token Lexer::tokenizeFFIBuiltin(const k_string& builtin) {
  auto st = KName::Default;

  if (builtin == FFIBuiltins.Attach) {
    st = KName::Builtin_FFI_Attach;
  } else if (builtin == FFIBuiltins.Invoke) {
    st = KName::Builtin_FFI_Invoke;
  } else if (builtin == FFIBuiltins.Load) {
    st = KName::Builtin_FFI_Load;
  } else if (builtin == FFIBuiltins.Unload) {
    st = KName::Builtin_FFI_Unload;
  }

  return createToken(KTokenType::IDENTIFIER, st, builtin);
}

Token Lexer::tokenizeSignalBuiltin(const k_string& builtin) {
  auto st = KName::Default;

  if (builtin == SignalBuiltins.Raise) {
    st = KName::Builtin_Signal_Raise;
  } else if (builtin == SignalBuiltins.Send) {
    st = KName::Builtin_Signal_Send;
  } else if (builtin == SignalBuiltins.Trap) {
    st = KName::Builtin_Signal_Trap;
  }

  return createToken(KTokenType::IDENTIFIER, st, builtin);
}

Token Lexer::tokenizeSocketBuiltin(const k_string& builtin) {
  auto st = KName::Default;

  if (builtin == SocketBuiltins.Accept) {
    st = KName::Builtin_Socket_Accept;
  } else if (builtin == SocketBuiltins.Bind) {
    st = KName::Builtin_Socket_Bind;
  } else if (builtin == SocketBuiltins.Close) {
    st = KName::Builtin_Socket_Close;
  } else if (builtin == SocketBuiltins.Connect) {
    st = KName::Builtin_Socket_Connect;
  } else if (builtin == SocketBuiltins.Create) {
    st = KName::Builtin_Socket_Create;
  } else if (builtin == SocketBuiltins.Listen) {
    st = KName::Builtin_Socket_Listen;
  } else if (builtin == SocketBuiltins.Receive) {
    st = KName::Builtin_Socket_Receive;
  } else if (builtin == SocketBuiltins.Send) {
    st = KName::Builtin_Socket_Send;
  } else if (builtin == SocketBuiltins.SendRaw) {
    st = KName::Builtin_Socket_SendRaw;
  } else if (builtin == SocketBuiltins.Shutdown) {
    st = KName::Builtin_Socket_Shutdown;
  } else if (builtin == SocketBuiltins.IsIPAddr) {
    st = KName::Builtin_Net_IsIPAddr;
  } else if (builtin == SocketBuiltins.ResolveHost) {
    st = KName::Builtin_Net_ResolveHost;
  }

  return createToken(KTokenType::IDENTIFIER, st, builtin);
}

Token Lexer::tokenizeReflectorBuiltin(const k_string& builtin) {
  auto st = KName::Default;

  if (builtin == ReflectorBuiltins.RFFlags) {
    st = KName::Builtin_Reflector_RFFlags;
  } else if (builtin == ReflectorBuiltins.RInspect) {
    st = KName::Builtin_Reflector_RInspect;
  } else if (builtin == ReflectorBuiltins.RList) {
    st = KName::Builtin_Reflector_RList;
  } else if (builtin == ReflectorBuiltins.RObject) {
    st = KName::Builtin_Reflector_RObject;
  } else if (builtin == ReflectorBuiltins.RRetVal) {
    st = KName::Builtin_Reflector_RRetVal;
  } else if (builtin == ReflectorBuiltins.RStack) {
    st = KName::Builtin_Reflector_RStack;
  }

  return createToken(KTokenType::IDENTIFIER, st, builtin);
}

Token Lexer::tokenizeSerializerBuiltin(const k_string& builtin) {
  auto st = KName::Default;

  if (builtin == SerializerBuiltins.Deserialize) {
    st = KName::Builtin_Serializer_Deserialize;
  } else if (builtin == SerializerBuiltins.Serialize) {
    st = KName::Builtin_Serializer_Serialize;
  }

  return createToken(KTokenType::IDENTIFIER, st, builtin);
}

Token Lexer::tokenizeEnvBuiltin(const k_string& builtin) {
  auto st = KName::Default;

  if (builtin == EnvBuiltins.GetEnvironmentVariable) {
    st = KName::Builtin_Env_GetEnvironmentVariable;
  } else if (builtin == EnvBuiltins.SetEnvironmentVariable) {
    st = KName::Builtin_Env_SetEnvironmentVariable;
  } else if (builtin == EnvBuiltins.UnsetEnvironmentVariable) {
    st = KName::Builtin_Env_UnsetEnvironmentVariable;
  } else if (builtin == EnvBuiltins.Kiwi) {
    st = KName::Builtin_Env_Kiwi;
  } else if (builtin == EnvBuiltins.KiwiLib) {
    st = KName::Builtin_Env_KiwiLib;
  }

  return createToken(KTokenType::IDENTIFIER, st, builtin);
}

Token Lexer::tokenizeFileIOBuiltin(const k_string& builtin) {
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

  return createToken(KTokenType::IDENTIFIER, st, builtin);
}

Token Lexer::tokenizeTaskBuiltin(const k_string& builtin) {
  auto st = KName::Default;

  if (builtin == TaskBuiltins.TaskBusy) {
    st = KName::Builtin_Task_Busy;
  } else if (builtin == TaskBuiltins.TaskList) {
    st = KName::Builtin_Task_List;
  } else if (builtin == TaskBuiltins.TaskResult) {
    st = KName::Builtin_Task_Result;
  } else if (builtin == TaskBuiltins.TaskSleep) {
    st = KName::Builtin_Task_Sleep;
  } else if (builtin == TaskBuiltins.TaskStatus) {
    st = KName::Builtin_Task_Status;
  }

  return createToken(KTokenType::IDENTIFIER, st, builtin);
}

Token Lexer::tokenizeMathBuiltin(const k_string& builtin) {
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
  } else if (builtin == MathBuiltins.RotateLeft) {
    st = KName::Builtin_Math_RotateLeft;
  } else if (builtin == MathBuiltins.RotateRight) {
    st = KName::Builtin_Math_RotateRight;
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

  return createToken(KTokenType::IDENTIFIER, st, builtin);
}

Token Lexer::tokenizePackageBuiltin(const k_string& builtin) {
  auto st = KName::Default;

  if (builtin == PackageBuiltins.Home) {
    st = KName::Builtin_Package_Home;
  }

  return createToken(KTokenType::IDENTIFIER, st, builtin);
}

Token Lexer::tokenizeSysBuiltin(const k_string& builtin) {
  auto st = KName::Default;

  if (builtin == SysBuiltins.EffectiveUserId) {
    st = KName::Builtin_Sys_EffectiveUserId;
  } else if (builtin == SysBuiltins.Exec) {
    st = KName::Builtin_Sys_Exec;
  } else if (builtin == SysBuiltins.ExecOut) {
    st = KName::Builtin_Sys_ExecOut;
  }

  return createToken(KTokenType::IDENTIFIER, st, builtin);
}

Token Lexer::tokenizeWebClientBuiltin(const k_string& builtin) {
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

  return createToken(KTokenType::IDENTIFIER, st, builtin);
}

Token Lexer::tokenizeWebServerBuiltin(const k_string& builtin) {
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

  return createToken(KTokenType::IDENTIFIER, st, builtin);
}

Token Lexer::tokenizeLoggingBuiltin(const k_string& builtin) {
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

  return createToken(KTokenType::IDENTIFIER, st, builtin);
}

Token Lexer::tokenizeTimeBuiltin(const k_string& builtin) {
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
  } else if (builtin == TimeBuiltins.FormatDateTime) {
    st = KName::Builtin_Time_FormatDateTime;
  } else if (builtin == TimeBuiltins.WeekDay) {
    st = KName::Builtin_Time_WeekDay;
  } else if (builtin == TimeBuiltins.Year) {
    st = KName::Builtin_Time_Year;
  } else if (builtin == TimeBuiltins.YearDay) {
    st = KName::Builtin_Time_YearDay;
  }

  return createToken(KTokenType::IDENTIFIER, st, builtin);
}

#endif
