#ifndef KIWI_PARSING_LEXER_H
#define KIWI_PARSING_LEXER_H

#include "tokens.h"
#include "keywords.h"
#include <sstream>
#include <string>
#include <vector>

class Lexer {
 public:
  Lexer(const std::string& file, const std::string& source,
        bool skipWhitespace = true)
      : file(file),
        source(source),
        currentPosition(0),
        _skipWhitespace(skipWhitespace) {
    std::istringstream stream(source);
    std::string line;

    while (std::getline(stream, line)) {
      lines.push_back(line);
    }
  }

  std::vector<Token> getAllTokens() {
    std::vector<Token> tokens;
    lineNumber = 0;
    linePosition = 0;

    while (true) {
      Token token = _getNextToken();

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
  size_t currentPosition;
  bool _skipWhitespace;
  int lineNumber;
  int linePosition;
  std::vector<std::string> lines;

  char getCurrentChar() {
    char c = source[currentPosition++];
    if (c == '\n') {
      lineNumber++;
      linePosition = 0;
    } else {
      linePosition++;
    }
    return c;
  }

  Token _getNextToken() {
    skipWhitespace();

    if (currentPosition >= source.length()) {
      return Token::create(TokenType::ENDOFFILE, file, "", 0, lineNumber,
                           linePosition);
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
      return Token::create(TokenType::DECLVAR, file, "@", lineNumber,
                           linePosition);
    } else if (currentChar == '$') {
      return Token::create(TokenType::OPERATOR, file, "$", lineNumber,
                           linePosition);
    } else if (currentChar == '\n') {
      return Token::create(TokenType::NEWLINE, file, "\n", lineNumber,
                           linePosition);
    } else if (currentChar == '(') {
      return Token::create(TokenType::OPEN_PAREN, file, "(", lineNumber,
                           linePosition);
    } else if (currentChar == ')') {
      return Token::create(TokenType::CLOSE_PAREN, file, ")", lineNumber,
                           linePosition);
    } else if (currentChar == '[') {
      return Token::create(TokenType::OPEN_BRACKET, file, "[", lineNumber,
                           linePosition);
    } else if (currentChar == ']') {
      return Token::create(TokenType::CLOSE_BRACKET, file, "]", lineNumber,
                           linePosition);
    } else if (currentChar == '{') {
      return Token::create(TokenType::OPEN_BRACE, file, "{", lineNumber,
                           linePosition);
    } else if (currentChar == '}') {
      return Token::create(TokenType::CLOSE_BRACE, file, "}", lineNumber,
                           linePosition);
    } else if (currentChar == ',') {
      return Token::create(TokenType::COMMA, file, ",", lineNumber,
                           linePosition);
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
    if (!_skipWhitespace) {
      return;
    }

    while (currentPosition < source.length() &&
           isspace(source[currentPosition])) {
      getCurrentChar();
    }
  }

  Token parseKeyword(std::string& keyword) {
    TokenType tokenType = TokenType::KEYWORD;

    if (Keywords.is_conditional_keyword(keyword)) {
      tokenType = TokenType::CONDITIONAL;
    } else if (keyword == Keywords.Lambda) {
      tokenType = TokenType::LAMBDA;
    } else if (Keywords.is_boolean(keyword)) {
      tokenType = TokenType::LITERAL;
      return Token::createBoolean(file, keyword, lineNumber, linePosition);
    }

    return Token::create(tokenType, file, keyword, lineNumber, linePosition);
  }

  Token parseUnspecified(char initialChar) {
    std::string s(1, initialChar);

    if (currentPosition < source.length()) {
      char nextChar = source[currentPosition];
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

        nextChar = source[currentPosition];
        if (nextChar == '=' && Operators.is_large_operator(s)) {
          s += nextChar;
          getCurrentChar();
        }
      }
    }

    return Token::create(TokenType::OPERATOR, file, s, lineNumber,
                         linePosition);
  }

  Token parseColon(char initialChar) {
    std::string s(1, initialChar);

    if (currentPosition < source.length()) {
      char nextChar = source[currentPosition];
      if (nextChar == ':') {
        s += nextChar;
        getCurrentChar();
        return Token::create(TokenType::QUALIFIER, file, s, lineNumber,
                             linePosition);
      }
    }

    return Token::create(TokenType::COLON, file, s, lineNumber, linePosition);
  }

  Token parseEscapeCharacter() {
    if (currentPosition < source.length()) {
      char nextChar = getCurrentChar();

      switch (nextChar) {
        case 't':
          return Token::create(TokenType::ESCAPED, file, "\t", lineNumber,
                               linePosition);
        case 'n':
          return Token::create(TokenType::ESCAPED, file, "\n", lineNumber,
                               linePosition);
      }
    }

    getCurrentChar();

    return Token::create(TokenType::IDENTIFIER, file, "\\", lineNumber,
                         linePosition);
  }

  Token parseDot(char initialChar) {
    std::string s(1, initialChar);

    if (currentPosition < source.length()) {
      char nextChar = source[currentPosition];
      if (nextChar == '.') {
        s += nextChar;
        getCurrentChar();
        return Token::create(TokenType::RANGE, file, s, lineNumber,
                             linePosition);
      }
    }

    return Token::create(TokenType::DOT, file, ".", lineNumber, linePosition);
  }

  Token parseIdentifier(char initialChar) {
    std::string identifier(1, initialChar);

    while (
        currentPosition < source.length() &&
        (isalnum(source[currentPosition]) || source[currentPosition] == '_')) {
      identifier += getCurrentChar();
    }

    if (Keywords.is_keyword(identifier)) {
      return parseKeyword(identifier);
    } else if (TypeNames.is_typename(identifier)) {
      return Token::create(TokenType::TYPENAME, file, identifier, lineNumber,
                           linePosition);
    }

    return Token::create(TokenType::IDENTIFIER, file, identifier, lineNumber,
                         linePosition);
  }

  Token parseLiteral(char initialChar) {
    std::string literal(1, initialChar);
    char lastChar = initialChar;

    while (
        currentPosition < source.length() &&
        (isdigit(source[currentPosition]) || source[currentPosition] == '.')) {
      if (source[currentPosition] == '.' && lastChar == '.') {
        literal.pop_back();
        --currentPosition;
        break;
      }

      lastChar = source[currentPosition];
      literal += getCurrentChar();
    }

    if (literal.find('.') != std::string::npos) {
      return Token::create(TokenType::LITERAL, file, literal,
                           std::stod(literal), lineNumber, linePosition);
    } else {
      return Token::create(TokenType::LITERAL, file, literal,
                           std::stoi(literal), lineNumber, linePosition);
    }
  }

  Token parseString() {
    std::string str;
    bool escape = false;

    while (currentPosition < source.length()) {
      char currentChar = source[currentPosition];

      if (escape) {
        switch (currentChar) {
          case 'n':
            str += '\n';
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
        getCurrentChar();
        break;  // End of string
      } else {
        str += currentChar;
      }

      getCurrentChar();  // Move to next character
    }

    // Handle case where string ends with a backslash
    if (escape) {
      str += '\\';
    }

    return Token::create(TokenType::STRING, file, str, lineNumber,
                         linePosition);
  }

  Token parseComment() {
    if (currentPosition + 1 < source.length() &&
        source[currentPosition] == '#') {
      // It's a multi-line comment.
      std::string comment;
      currentPosition++;  // Skip the "##"

      while (currentPosition + 1 < source.length()) {
        char currentChar = getCurrentChar();
        if (currentChar == '#' && source[currentPosition] == '#') {
          currentPosition++;  // Skip the "##"
          break;
        } else {
          comment += currentChar;
        }
      }

      return Token::create(TokenType::COMMENT, file, comment, lineNumber,
                           linePosition);
    } else {
      // It's a single-line comment
      std::string comment;

      while (currentPosition < source.length() &&
             source[currentPosition] != '\n') {
        comment += getCurrentChar();
      }

      return Token::create(TokenType::COMMENT, file, comment, lineNumber,
                           linePosition);
    }
  }
};

#endif