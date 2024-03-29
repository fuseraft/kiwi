#ifndef ASTRAL_PARSING_TOKENS_H
#define ASTRAL_PARSING_TOKENS_H

#include <memory>
#include <string>
#include "parsing/keywords.h"
#include "parsing/tokentype.h"
#include "typing/serializer.h"
#include "typing/value.h"

class Token {
 public:
  static Token create(const KTokenType& t, const KName& st, const int& fileId,
                      const k_string& text, const k_value& v,
                      const int& lineNumber, const int& linePosition) {
    Token token(t, st, fileId, text, v, lineNumber, linePosition);
    return token;
  }

  static Token create(const KTokenType& t, const KName& st, const int& fileId,
                      const k_string& text, const int& lineNumber,
                      const int& linePosition) {
    return create(t, st, fileId, text, text, lineNumber, linePosition);
  }

  static Token createBoolean(const int& fileId, const k_string& text,
                             const int& lineNumber, const int& linePosition) {
    bool value = text == Keywords.True;
    auto st = value ? KName::KW_True : KName::KW_False;
    return create(KTokenType::LITERAL, st, fileId, text, value, lineNumber,
                  linePosition);
  }

  static Token createEmpty() {
    return create(KTokenType::ENDOFFILE, KName::Default, 0, "", 0, 0);
  }

  static Token createStreamEnd() {
    return create(KTokenType::STREAM_END, KName::Default, 0, "", 0, 0);
  }

  const int& getFile() const { return fileId; }

  const k_string& getText() const { return text; }

  const int& getLineNumber() const { return _lineNumber; }

  const int& getLinePosition() const { return _linePosition; }

  KTokenType getType() const { return type; }

  KName getSubType() const { return subType; }

  k_value& getValue() { return value; }

 private:
  KTokenType type;
  KName subType;
  int fileId;
  k_string text;
  k_value value;
  int _lineNumber;
  int _linePosition;

  Token(const KTokenType& t, const KName& st, const int& fileId,
        const k_string& text, const k_value& v, const int& lineNumber,
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

  bool match(KTokenType type) {
    if (current().getType() == type) {
      next();
      return true;
    }
    return false;
  }

  bool matchsub(KName subType) {
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