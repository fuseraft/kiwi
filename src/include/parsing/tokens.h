#ifndef TOKENS_H
#define TOKENS_H

#include <string>
#include <variant>
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
    ENDOFFILE
};

std::string get_token_type_string(TokenType tt) {
    switch (tt) {
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
        case TokenType::ENDOFFILE:
            return "ENDOFFILE";
    }

    std::ostringstream error;
    error << "Cannot determine token type `" << static_cast<int>(tt) << "`.";
    throw std::runtime_error(error.str());
}

struct Token {
    TokenType                                    type;
    std::string                                  text;
    std::variant<int, double, bool, std::string> value;
    ValueType                                    value_type;

    Token(
        TokenType t, std::string text,
        const std::variant<int, double, bool, std::string> &v)
        : type(t), text(text), value(v) {
        value_type = get_value_type(v);
    }

    std::string info() {
        std::ostringstream info;
        info << "type: " << get_token_type_string(type) << ", "
             << "text: " << text;
        return info.str();
    }

    std::string toString() {
        if (value_type != ValueType::String) {
            throw new std::runtime_error("Value type is not a `String`.");
        }

        return std::get<std::string>(value);
    }

    int toInteger() {
        if (value_type != ValueType::Integer) {
            throw new std::runtime_error("Value type is not an `Integer`.");
        }

        return std::get<int>(value);
    }

    bool toBoolean() {
        if (value_type != ValueType::Boolean) {
            throw new std::runtime_error("Value type is not a `Boolean`.");
        }

        return std::get<bool>(value);
    }

    double toDouble() {
        if (value_type != ValueType::Double) {
            throw new std::runtime_error("Value type is not a `Double`.");
        }
        
        return std::get<double>(value);
    }
};

#endif