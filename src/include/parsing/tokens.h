#ifndef TOKENS_H
#define TOKENS_H

#include <variant>
#include <string>

enum class ValueType {
    None,
    Unknown,
    Integer,
    Double,
    Boolean,
    String,
};

ValueType get_value_type(std::variant<int, double, bool, std::string> v) {
    ValueType type = ValueType::None;
    std::visit([&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, int>)
            type = ValueType::Integer;
        else if constexpr (std::is_same_v<T, double>)
            type = ValueType::Double;
        else if constexpr (std::is_same_v<T, bool>)
            type = ValueType::Boolean;
        else if constexpr (std::is_same_v<T, std::string>)
            type = ValueType::String;
        else
            type = ValueType::Unknown;
    }, v);
    return type;
}

enum TokenType {
    IDENTIFIER,
    COMMENT,
    KEYWORD,
    OPERATOR,
    LITERAL,
    STRING,
    NEWLINE,
    ENDOFFILE
};

struct Token {
    TokenType type;
    ValueType value_type;
    std::variant<int, double, bool, std::string> value;

    Token(TokenType t, const std::variant<int, double, bool, std::string>& v) : type(t), value(v) {
        value_type = get_value_type(v);
    }
};


#endif