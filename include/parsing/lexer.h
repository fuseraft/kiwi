#ifndef KIWI_PARSING_LEXER_H
#define KIWI_PARSING_LEXER_H

#include "logging/logger.h"
#include "tokens.h"
#include "keywords.h"
#include <sstream>
#include <string>
#include <vector>

class Lexer {
 public:
  Lexer(Logger& logger, const std::string& file, const std::string& source,
        bool skipWhitespace = true)
      : logger(logger),
        file(file),
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

  std::vector<std::string> getLines() { return lines; }

  std::string getFile() { return file; }

 private:
  Logger& logger;
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
      return Token::create(TokenType::KEYWORD, file, "@", lineNumber,
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
    } else if (currentChar == ',') {
      return Token::create(TokenType::COMMA, file, ",", lineNumber,
                           linePosition);
    } else if (currentChar == '\\') {
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
    } else if (currentChar == ':') {
      std::string s;
      s = currentChar;

      if (currentPosition < source.length()) {
        char nextChar = source[currentPosition];
        if (nextChar == ':') {
          s += nextChar;
          getCurrentChar();
          return Token::create(TokenType::QUALIFIER, file, s, lineNumber,
                               linePosition);
        }
      }

      return Token::create(TokenType::OPERATOR, file, s, lineNumber,
                           linePosition);
    } else {
      std::string s;
      s = currentChar;

      if (currentPosition < source.length()) {
        char nextChar = source[currentPosition];
        bool isArithmeticOpChar =
            Operators.is_arithmetic_operator_char(currentChar);
        bool isBooleanOpChar = Operators.is_boolean_operator_char(currentChar);
        bool isArithmeticOp =
            (nextChar == '=' && (isArithmeticOpChar || isBooleanOpChar)) ||
            (currentChar == '*' && nextChar == '*');
        bool isBooleanOp =
            (nextChar == '|' || nextChar == '&') && isBooleanOpChar;
        bool isBitwiseOp = (Operators.is_bitwise_operator_char(currentChar) &&
                            nextChar == '=') ||
                           (currentChar == '<' && nextChar == '<') ||
                           (currentChar == '>' && nextChar == '>');

        bool appendNextChar = isArithmeticOp || isBooleanOp || isBitwiseOp;

        if (appendNextChar) {
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

  Token parseKeyword(std::string& identifier) {
    TokenType tokenType;

    if (Keywords.is_conditional_keyword(identifier)) {
      tokenType = TokenType::CONDITIONAL;
    } else if (Keywords.is_boolean(identifier)) {
      tokenType = TokenType::LITERAL;
      return Token::createBoolean(file, identifier, lineNumber, linePosition);
    } else {
      tokenType = TokenType::KEYWORD;
    }

    return Token::create(tokenType, file, identifier, lineNumber, linePosition);
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
    }

    return Token::create(TokenType::IDENTIFIER, file, identifier, lineNumber,
                         linePosition);
  }

  Token parseLiteral(char initialChar) {
    std::string literal(1, initialChar);

    while (
        currentPosition < source.length() &&
        (isdigit(source[currentPosition]) || source[currentPosition] == '.')) {
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

    while (currentPosition < source.length() &&
           source[currentPosition] != '"') {
      str += getCurrentChar();
    }

    getCurrentChar();  // skip closing quote
    return Token::create(TokenType::STRING, file, str, lineNumber,
                         linePosition);
  }

  Token parseComment() {
    std::string comment;

    while (currentPosition < source.length() &&
           source[currentPosition] != '\n') {
      comment += getCurrentChar();
    }

    return Token::create(TokenType::COMMENT, file, comment, lineNumber,
                         linePosition);
  }
};

#endif