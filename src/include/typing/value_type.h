#ifndef VALUE_TYPE_H
#define VALUE_TYPE_H

#include <variant>

enum class ValueType {
    None,
    Unknown,
    Integer,
    Double,
    Boolean,
    String,
};

ValueType get_value_type(std::variant<int, double, bool, std::string> v) {
    ValueType type = ValueType::None; // TODO: Handle this.

    std::visit(
        [&](auto &&arg) {
            using T = std::decay_t<decltype(arg)>;

            if constexpr (std::is_same_v<T, int>) {
                type = ValueType::Integer;
            }
            else if constexpr (std::is_same_v<T, double>) {
                type = ValueType::Double;
            }
            else if constexpr (std::is_same_v<T, bool>) {
                type = ValueType::Boolean;
            }
            else if constexpr (std::is_same_v<T, std::string>) {
                type = ValueType::String;
            }
            else {
                type = ValueType::Unknown;
            }
        }, v);

    return type;
}

std::string get_value_type_string(ValueType vt) {
    switch (vt) {
        case ValueType::Boolean:
            return "Boolean";
        case ValueType::String:
            return "String";
        case ValueType::Integer:
            return "Integer";
        case ValueType::Double:
            return "Double";
        case ValueType::Unknown:
            return "Unknown";
        case ValueType::None:
            return "None";
    }

    std::ostringstream error;
    error << "Cannot determine value type `" << static_cast<int>(vt) << "`.";
    throw std::runtime_error(error.str());
}

#endif