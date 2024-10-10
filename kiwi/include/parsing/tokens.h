#ifndef KIWI_PARSING_TOKENS_H
#define KIWI_PARSING_TOKENS_H

#include <memory>
#include <string>
#include "parsing/keywords.h"
#include "parsing/tokentype.h"
#include "typing/serializer.h"
#include "typing/value.h"

k_string get_token_type_string(KTokenType type) {
  switch (type) {
    case KTokenType::CLOSE_BRACE:
      return "CLOSE_BRACE";
    case KTokenType::CLOSE_BRACKET:
      return "CLOSE_BRACKET";
    case KTokenType::CLOSE_PAREN:
      return "CLOSE_PAREN";
    case KTokenType::COLON:
      return "COLON";
    case KTokenType::COMMA:
      return "COMMA";
    case KTokenType::COMMENT:
      return "COMMENT";
    case KTokenType::CONDITIONAL:
      return "CONDITIONAL";
    case KTokenType::DOT:
      return "DOT";
    case KTokenType::ENDOFFILE:
      return "ENDOFFILE";
    case KTokenType::ESCAPED:
      return "ESCAPED";
    case KTokenType::IDENTIFIER:
      return "IDENTIFIER";
    case KTokenType::KEYWORD:
      return "KEYWORD";
    case KTokenType::LAMBDA:
      return "LAMBDA";
    case KTokenType::LITERAL:
      return "LITERAL";
    case KTokenType::NEWLINE:
      return "NEWLINE";
    case KTokenType::OPEN_BRACE:
      return "OPEN_BRACE";
    case KTokenType::OPEN_BRACKET:
      return "OPEN_BRACKET";
    case KTokenType::OPEN_PAREN:
      return "OPEN_PAREN";
    case KTokenType::OPERATOR:
      return "OPERATOR";
    case KTokenType::QUALIFIER:
      return "QUALIFIER";
    case KTokenType::QUESTION:
      return "QUESTION";
    case KTokenType::RANGE:
      return "RANGE";
    case KTokenType::STREAM_END:
      return "STREAM_END";
    case KTokenType::STRING:
      return "STRING";
    case KTokenType::TYPENAME:
      return "TYPENAME";
  }

  return "UNKNOWN";
}

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

  static Token createNull(const int& fileId, const k_string& text,
                          const int& lineNumber, const int& linePosition) {
    k_null value;
    return create(KTokenType::LITERAL, KName::KW_Null, fileId, text, value,
                  lineNumber, linePosition);
  }

  static Token createEmpty() {
    return create(KTokenType::ENDOFFILE, KName::Default, 0, "", 0, 0);
  }

  static Token createStreamEnd() {
    return create(KTokenType::STREAM_END, KName::Default, 0, "", 0, 0);
  }

  const int& getFile() const { return fileId; }

  const k_string& getText() const { return text; }

  const k_string& getOriginalText() const { return text; }

  void setText(const k_string& newText) { text = newText; }

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
  k_string originalText;
  k_value value;
  int _lineNumber;
  int _linePosition;

  Token(const KTokenType& t, const KName& st, const int& fileId,
        const k_string& text, const k_value& v, const int& lineNumber,
        const int& linePosition)
      : type(t),
        subType(st),
        fileId(fileId),
        text(text),
        originalText(text),
        value(v) {
    _lineNumber = lineNumber;
    _linePosition = linePosition;
  }
};

class TokenStream {
 public:
  TokenStream(const std::vector<Token>& tokens) : tokens(tokens) {}
  ~TokenStream() { tokens.clear(); }

  Token at(size_t pos) {
    if (pos >= tokens.size()) {
      return Token::createStreamEnd();
    }
    return tokens.at(pos);
  }

  Token current() {
    if (position >= tokens.size()) {
      return Token::createStreamEnd();
    }
    return tokens.at(position);
  }

  Token previous() { return tokens.at(position - 1); }

  void rewind() {
    if (static_cast<int>(position) - 1 < 0) {
      return;  // TODO: is it okay to swallow this?
    }

    --position;
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
      if (current().getType() == KTokenType::COMMENT) {
        next();
      }
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
  size_t size() const { return tokens.size(); }
};

using k_stream = std::shared_ptr<TokenStream>;

#endif