#ifndef TOKENS_H
#define TOKENS_H

#include <string>
#include <variant>
#include "../parsing/keywords.h"
#include "../typing/value_type.h"

enum TokenType {
  IDENTIFIER,
  COMMENT,
  KEYWORD,
  OPERATOR,
  LITERAL,
  STRING,
  NEWLINE,
  ESCAPED,
  OPEN_PAREN,
  CLOSE_PAREN,
  CONDITIONAL,
  NOOP,
  ENDOFFILE
};

std::string get_token_type_string(TokenType tokenType) {
  switch (tokenType) {
    case TokenType::IDENTIFIER:
      return "IDENTIFIER";
    case TokenType::COMMENT:
      return "COMMENT";
    case TokenType::KEYWORD:
      return "KEYWORD";
    case TokenType::OPERATOR:
      return "OPERATOR";
    case TokenType::LITERAL:
      return "LITERAL";
    case TokenType::STRING:
      return "STRING";
    case TokenType::NEWLINE:
      return "NEWLINE";
    case TokenType::ESCAPED:
      return "ESCAPED";
    case TokenType::OPEN_PAREN:
      return "OPEN_PAREN";
    case TokenType::CLOSE_PAREN:
      return "CLOSE_PAREN";
    case TokenType::CONDITIONAL:
      return "CONDITIONAL";
    case TokenType::NOOP:
      return "NOOP";
    case TokenType::ENDOFFILE:
      return "ENDOFFILE";
  }

  std::ostringstream error;
  error << "Cannot determine token type `" << static_cast<int>(tokenType)
        << "`.";
  throw std::runtime_error(error.str());
}

class Token {
 public:
  static Token create(TokenType t, std::string file, std::string text,
                      const std::variant<int, double, bool, std::string>& v,
                      const int& lineNumber, const int& linePosition) {
    Token token(t, file, text, v, lineNumber, linePosition);
    return token;
  }

  static Token create(TokenType t, std::string file, std::string text, const int& lineNumber,
                      const int& linePosition) {
    return create(t, file, text, text, lineNumber, linePosition);
  }

  static Token createBoolean(const std::string& file, std::string text, const int& lineNumber,
                             const int& linePosition) {
    bool value = text == Keywords.True;
    return create(TokenType::LITERAL, file, text, value, lineNumber, linePosition);
  }

  static Token createNoOp() { return create(TokenType::NOOP, "", "", 0, 0); }

  static Token createEmpty() { return create(TokenType::ENDOFFILE, "", "", 0, 0); }

  std::string info() {
    std::ostringstream info;
    info << "type: " << get_token_type_string(type) << ", "
         << "text: " << text;
    return info.str();
  }

  std::string getFile() const {
    return file;
  }

  std::string toString() {
    if (valueType != ValueType::String) {
      throw new std::runtime_error("Value type is not a `String`.");
    }

    return std::get<std::string>(value);
  }

  int toInteger() {
    if (valueType != ValueType::Integer) {
      throw new std::runtime_error("Value type is not an `Integer`.");
    }

    return std::get<int>(value);
  }

  bool toBoolean() {
    if (valueType != ValueType::Boolean) {
      throw new std::runtime_error("Value type is not a `Boolean`.");
    }

    return std::get<bool>(value);
  }

  double toDouble() {
    if (valueType != ValueType::Double) {
      throw new std::runtime_error("Value type is not a `Double`.");
    }

    return std::get<double>(value);
  }

  std::string getText() const { return text; }

  const int& getLineNumber() const { return _lineNumber; }

  const int& getLinePosition() const { return _linePosition; }

  TokenType getType() { return type; }

  std::variant<int, double, bool, std::string> getValue() { return value; }

  ValueType getValueType() { return valueType; }

 private:
  TokenType type;
  std::string file;
  std::string text;
  std::variant<int, double, bool, std::string> value;
  ValueType valueType;
  int _lineNumber;
  int _linePosition;

  Token(TokenType t, std::string file, std::string text,
        const std::variant<int, double, bool, std::string>& v,
        const int& lineNumber, const int& linePosition)
      : type(t), file(file), text(text), value(v) {
    valueType = get_value_type(v);
    _lineNumber = lineNumber;
    _linePosition = linePosition;
  }
};

#endif