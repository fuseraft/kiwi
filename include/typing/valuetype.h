#ifndef KIWI_TYPING_VALUETYPE_H
#define KIWI_TYPING_VALUETYPE_H

#include <variant>
#include <stdexcept>
#include <sstream>
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
  ValueType type = ValueType::None;  // TODO: Handle this.

  std::visit(
      [&](auto&& arg) {
        using T = std::decay_t<decltype(arg)>;

        if constexpr (std::is_same_v<T, int>) {
          type = ValueType::Integer;
        } else if constexpr (std::is_same_v<T, double>) {
          type = ValueType::Double;
        } else if constexpr (std::is_same_v<T, bool>) {
          type = ValueType::Boolean;
        } else if constexpr (std::is_same_v<T, std::string>) {
          type = ValueType::String;
        } else {
          type = ValueType::Unknown;
        }
      },
      v);

  return type;
}

std::string get_value_string(std::variant<int, double, bool, std::string> v) {
  ValueType vt = get_value_type(v);
  std::ostringstream sv;

  if (vt == ValueType::Integer) {
    sv << std::get<int>(v);
  } else if (vt == ValueType::Double) {
    sv << std::get<double>(v);
  } else if (vt == ValueType::Boolean) {
    sv << std::boolalpha << std::get<bool>(v);
  } else if (vt == ValueType::String) {
    sv << std::get<std::string>(v);
  }

  return sv.str();
}

#endif