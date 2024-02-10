#ifndef KIWI_PARSING_TOKENS_H
#define KIWI_PARSING_TOKENS_H

#include <string>
#include "parsing/keywords.h"
#include "typing/serializer.h"
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
  OPEN_BRACE,
  CLOSE_BRACE,
  CONDITIONAL,
  ENDOFFRAME,
  QUALIFIER,
  RANGE,
  COLON,
  DOT,
  TYPENAME,
  LAMBDA,
  DECLVAR,
  ENDOFFILE
};

class Token {
 public:
  static Token create(TokenType t, std::string file, std::string text,
                      const Value& v, const int& lineNumber,
                      const int& linePosition) {
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
    if (!std::holds_alternative<std::string>(value)) {
      throw new std::runtime_error("Value type is not a `String`.");
    }

    return std::get<std::string>(value);
  }

  int toInteger() {
    if (!std::holds_alternative<int>(value)) {
      throw new std::runtime_error("Value type is not an `Integer`.");
    }

    return std::get<int>(value);
  }

  bool toBoolean() {
    if (!std::holds_alternative<bool>(value)) {
      throw new std::runtime_error("Value type is not a `Boolean`.");
    }

    return std::get<bool>(value);
  }

  double toDouble() {
    if (!std::holds_alternative<double>(value)) {
      throw new std::runtime_error("Value type is not a `Double`.");
    }

    return std::get<double>(value);
  }

  std::string getText() const { return text; }

  const int& getLineNumber() const { return _lineNumber; }

  const int& getLinePosition() const { return _linePosition; }

  TokenType getType() const { return type; }

  Value getValue() { return value; }

  ValueType getValueType() { return valueType; }

 private:
  TokenType type;
  std::string file;
  std::string text;
  Value value;
  ValueType valueType;
  int _lineNumber;
  int _linePosition;
  std::string owner;

  Token(TokenType t, std::string file, std::string text, const Value& v,
        const int& lineNumber, const int& linePosition)
      : type(t), file(file), text(text), value(v) {
    valueType = Serializer::get_value_type(v);
    _lineNumber = lineNumber;
    _linePosition = linePosition;
    owner = "";
  }
};

#endif