#ifndef KIWI_PARSING_LEXER_H
#define KIWI_PARSING_LEXER_H

#include <memory>
#include <sstream>
#include <string>
#include <vector>
#include "parsing/tokens.h"
#include "parsing/keywords.h"

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
      return Token::create(TokenType::ENDOFFILE, file, "", 0, row, col);
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
      return Token::create(TokenType::DECLVAR, file, "@", row, col);
    } else if (currentChar == '$') {
      return Token::create(TokenType::OPERATOR, file, "$", row, col);
    } else if (currentChar == '\n') {
      return Token::create(TokenType::NEWLINE, file, "\n", row, col);
    } else if (currentChar == '(') {
      return Token::create(TokenType::OPEN_PAREN, file, "(", row, col);
    } else if (currentChar == ')') {
      return Token::create(TokenType::CLOSE_PAREN, file, ")", row, col);
    } else if (currentChar == '[') {
      return Token::create(TokenType::OPEN_BRACKET, file, "[", row, col);
    } else if (currentChar == ']') {
      return Token::create(TokenType::CLOSE_BRACKET, file, "]", row, col);
    } else if (currentChar == '{') {
      return Token::create(TokenType::OPEN_BRACE, file, "{", row, col);
    } else if (currentChar == '}') {
      return Token::create(TokenType::CLOSE_BRACE, file, "}", row, col);
    } else if (currentChar == ',') {
      return Token::create(TokenType::COMMA, file, ",", row, col);
    } else if (currentChar == '?') {
      return Token::create(TokenType::QUESTION, file, "?", row, col);
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

  Token parseKeyword(std::string& keyword) {
    auto tokenType = TokenType::KEYWORD;

    if (Keywords.is_conditional_keyword(keyword)) {
      tokenType = TokenType::CONDITIONAL;
    } else if (keyword == Keywords.Lambda) {
      tokenType = TokenType::LAMBDA;
    } else if (Keywords.is_boolean(keyword)) {
      tokenType = TokenType::LITERAL;
      return Token::createBoolean(file, keyword, row, col);
    }

    return Token::create(tokenType, file, keyword, row, col);
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

    return Token::create(TokenType::OPERATOR, file, s, row, col);
  }

  Token parseColon(char initialChar) {
    std::string s(1, initialChar);

    if (pos < source.length()) {
      char nextChar = source[pos];
      if (nextChar == ':') {
        s += nextChar;
        getCurrentChar();
        return Token::create(TokenType::QUALIFIER, file, s, row, col);
      }
    }

    return Token::create(TokenType::COLON, file, s, row, col);
  }

  Token parseEscapeCharacter() {
    if (pos < source.length()) {
      char nextChar = getCurrentChar();

      switch (nextChar) {
        case 'n':
          return Token::create(TokenType::ESCAPED, file, "\n", row, col);
        case 'r':
          return Token::create(TokenType::ESCAPED, file, "\r", row, col);
        case 't':
          return Token::create(TokenType::ESCAPED, file, "\t", row, col);
      }
    }

    getCurrentChar();

    return Token::create(TokenType::IDENTIFIER, file, "\\", row, col);
  }

  Token parseDot(char initialChar) {
    std::string s(1, initialChar);

    if (pos < source.length()) {
      char nextChar = source[pos];
      if (nextChar == '.') {
        s += nextChar;
        getCurrentChar();
        return Token::create(TokenType::RANGE, file, s, row, col);
      }
    }

    return Token::create(TokenType::DOT, file, ".", row, col);
  }

  Token parseIdentifier(char initialChar) {
    std::string identifier(1, initialChar);

    while (pos < source.length() &&
           (isalnum(source[pos]) || source[pos] == '_')) {
      identifier += getCurrentChar();
    }

    if (Keywords.is_keyword(identifier)) {
      return parseKeyword(identifier);
    } else if (TypeNames.is_typename(identifier)) {
      return Token::create(TokenType::TYPENAME, file, identifier, row, col);
    }

    return Token::create(TokenType::IDENTIFIER, file, identifier, row, col);
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
      return Token::create(TokenType::LITERAL, file, literal,
                           std::stod(literal), row, col);
    } else {
      return Token::create(TokenType::LITERAL, file, literal,
                           std::stoi(literal), row, col);
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

    return Token::create(TokenType::STRING, file, str, row, col);
  }

  std::string parseInterpolatedExpression() {
    std::string expression;
    int braceCount = 1;

    while (pos < source.length() && braceCount > 0) {
      char currentChar = getCurrentChar();

      if (currentChar == '}' && braceCount == 1) {
        braceCount--;
        break;
      } else if (currentChar == '{') {
        braceCount++;
      } else if (currentChar == '}') {
        braceCount--;
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

      return Token::create(TokenType::COMMENT, file, comment, row, col);
    } else {
      // It's a single-line comment
      std::string comment;

      while (pos < source.length() && source[pos] != '\n') {
        comment += getCurrentChar();
      }

      return Token::create(TokenType::COMMENT, file, comment, row, col);
    }
  }
};

#endif