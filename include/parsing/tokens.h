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
  static Token create(const TokenType& t, const SubTokenType& st,
                      const int& fileId, const std::string& text,
                      const Value& v, const int& lineNumber,
                      const int& linePosition) {
    Token token(t, st, fileId, text, v, lineNumber, linePosition);
    return token;
  }

  static Token create(const TokenType& t, const SubTokenType& st,
                      const int& fileId, const std::string& text,
                      const int& lineNumber, const int& linePosition) {
    return create(t, st, fileId, text, text, lineNumber, linePosition);
  }

  static Token createBoolean(const int& fileId, const std::string& text,
                             const int& lineNumber, const int& linePosition) {
    bool value = text == Keywords.True;
    auto st = value ? SubTokenType::KW_True : SubTokenType::KW_False;
    return create(TokenType::LITERAL, st, fileId, text, value, lineNumber,
                  linePosition);
  }

  static Token createEmpty() {
    return create(TokenType::ENDOFFILE, SubTokenType::Default, 0, "", 0, 0);
  }

  static Token createStreamEnd() {
    return create(TokenType::STREAM_END, SubTokenType::Default, 0, "", 0, 0);
  }

  const int& getFile() const { return fileId; }

  const std::string& getText() const { return text; }

  const int& getLineNumber() const { return _lineNumber; }

  const int& getLinePosition() const { return _linePosition; }

  TokenType getType() const { return type; }

  SubTokenType getSubType() const { return subType; }

  Value& getValue() { return value; }

 private:
  TokenType type;
  SubTokenType subType;
  int fileId;
  std::string text;
  Value value;
  int _lineNumber;
  int _linePosition;

  Token(const TokenType& t, const SubTokenType& st, const int& fileId,
        const std::string& text, const Value& v, const int& lineNumber,
        const int& linePosition)
      : type(t), subType(st), fileId(fileId), text(text), value(v) {
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

  bool match(TokenType type) {
    if (current().getType() == type) {
      next();
      return true;
    }
    return false;
  }

  bool match(SubTokenType subType) {
    if (current().getSubType() == subType) {
      next();
      return true;
    }
    return false;
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