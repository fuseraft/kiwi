#ifndef KIWI_PARSING_TOKENS_H
#define KIWI_PARSING_TOKENS_H

#include <string>
#include <variant>
#include "parsing/keywords.h"
#include "typing/valuetype.h"

enum TokenType {
  IDENTIFIER,
  COMMENT,
  COMMA,
  KEYWORD,
  OPERATOR,
  LITERAL,
  STRING,
  NEWLINE,
  ESCAPED,
  OPEN_PAREN,
  CLOSE_PAREN,
  OPEN_BRACKET,
  CLOSE_BRACKET,
  CONDITIONAL,
  NOOP,
  ENDOFFRAME,
  QUALIFIER,
  ENDOFFILE
};

class Token {
 public:
  static Token create(TokenType t, std::string file, std::string text,
                      const std::variant<int, double, bool, std::string,
                                         std::shared_ptr<List>>& v,
                      const int& lineNumber, const int& linePosition) {
    Token token(t, file, text, v, lineNumber, linePosition);
    return token;
  }

  static Token create(TokenType t, std::string file, std::string text,
                      const int& lineNumber, const int& linePosition) {
    return create(t, file, text, text, lineNumber, linePosition);
  }

  static Token createBoolean(const std::string& file, std::string text,
                             const int& lineNumber, const int& linePosition) {
    bool value = text == Keywords.True;
    return create(TokenType::LITERAL, file, text, value, lineNumber,
                  linePosition);
  }

  static Token createNoOp() { return create(TokenType::NOOP, "", "", 0, 0); }

  static Token createEmpty() {
    return create(TokenType::ENDOFFILE, "", "", 0, 0);
  }

  static Token createEndOfFrame() {
    return create(TokenType::ENDOFFRAME, "", "", 0, 0);
  }

  void setOwner(std::string value) { owner = value; }
  std::string getOwner() { return owner; }

  std::string getFile() const { return file; }

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

  TokenType getType() const { return type; }

  std::variant<int, double, bool, std::string, std::shared_ptr<List>>
  getValue() {
    return value;
  }

  ValueType getValueType() { return valueType; }

 private:
  TokenType type;
  std::string file;
  std::string text;
  std::variant<int, double, bool, std::string, std::shared_ptr<List>> value;
  ValueType valueType;
  int _lineNumber;
  int _linePosition;
  std::string owner;

  Token(TokenType t, std::string file, std::string text,
        const std::variant<int, double, bool, std::string,
                           std::shared_ptr<List>>& v,
        const int& lineNumber, const int& linePosition)
      : type(t), file(file), text(text), value(v) {
    valueType = get_value_type(v);
    _lineNumber = lineNumber;
    _linePosition = linePosition;
    owner = "";
  }
};

#endif