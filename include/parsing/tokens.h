#ifndef KIWI_PARSING_TOKENS_H
#define KIWI_PARSING_TOKENS_H

#include <memory>
#include <string>
#include "k_int.h"
#include "parsing/keywords.h"
#include "parsing/tokentype.h"
#include "typing/serializer.h"
#include "typing/valuetype.h"

class Token {
 public:
  static Token create(TokenType t, SubTokenType st, const std::string& file,
                      const std::string& text, const Value& v,
                      const int& lineNumber, const int& linePosition) {
    Token token(t, st, file, text, v, lineNumber, linePosition);
    return token;
  }

  static Token create(TokenType t, SubTokenType st, const std::string& file,
                      const std::string& text, const int& lineNumber,
                      const int& linePosition) {
    return create(t, st, file, text, text, lineNumber, linePosition);
  }

  static Token createBoolean(const std::string& file, const std::string& text,
                             const int& lineNumber, const int& linePosition) {
    bool value = text == Keywords.True;
    auto st = value ? SubTokenType::KW_True : SubTokenType::KW_False;
    return create(TokenType::LITERAL, st, file, text, value, lineNumber,
                  linePosition);
  }

  static Token createEmpty() {
    return create(TokenType::ENDOFFILE, SubTokenType::Default, "", "", 0, 0);
  }

  static Token createStreamEnd() {
    return create(TokenType::STREAM_END, SubTokenType::Default, "", "", 0, 0);
  }

  std::string getFile() const { return file; }

  std::string getText() const { return text; }

  const int& getLineNumber() const { return _lineNumber; }

  const int& getLinePosition() const { return _linePosition; }

  TokenType getType() const { return type; }

  SubTokenType getSubType() const { return subType; }

  Value& getValue() { return value; }

 private:
  TokenType type;
  SubTokenType subType;
  std::string file;
  std::string text;
  Value value;
  ValueType valueType;
  int _lineNumber;
  int _linePosition;

  Token(TokenType t, SubTokenType st, std::string file, std::string text,
        const Value& v, const int& lineNumber, const int& linePosition)
      : type(t), subType(st), file(file), text(text), value(v) {
    valueType = Serializer::get_value_type(v);
    _lineNumber = lineNumber;
    _linePosition = linePosition;
  }
};

class TokenStream {
 public:
  TokenStream(const std::vector<Token>& tokens) : tokens(tokens) {}
  ~TokenStream() { tokens.clear(); }

  Token current() {
    if (position >= tokens.size()) {
      return Token::createStreamEnd();
    }
    return tokens.at(position);
  }

  void next() {
    if (position < tokens.size()) {
      position++;
    }
  }

  Token peek() {
    if (position + 1 < tokens.size()) {
      return tokens.at(position + 1);
    } else {
      return Token::createStreamEnd();
    }
  }

  bool empty() const { return tokens.empty(); }
  bool canRead() const { return position < tokens.size(); }

  std::vector<Token> tokens;
  size_t position = 0;
};

#endif