#ifndef KIWI_TYPING_VALUETYPE_H
#define KIWI_TYPING_VALUETYPE_H

#include <memory>
#include <stdexcept>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include "errors/error.h"

struct Hash;
struct List;

using Value = std::variant<int, double, bool, std::string,
                           std::shared_ptr<List>, std::shared_ptr<Hash>>;

struct List {
  std::vector<Value> elements;
};

struct Hash {
  std::unordered_map<std::string, Value> kvp;
};

std::shared_ptr<List> convert_value_to_list(Value& rhsValues) {
  if (std::holds_alternative<std::shared_ptr<List>>(rhsValues)) {
    return std::get<std::shared_ptr<List>>(rhsValues);
  } else {
    auto newList = std::make_shared<List>();
    newList->elements.push_back(rhsValues);
    return newList;
  }
}

enum class ValueType { None, Unknown, Integer, Double, Boolean, String, List };

ValueType get_value_type(Value v) {
  ValueType type = ValueType::None;

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
        } else if constexpr (std::is_same_v<T, std::shared_ptr<List>>) {
          type = ValueType::List;
        } else {
          type = ValueType::Unknown;
        }
      },
      v);

  return type;
}

std::string list_to_string(const std::shared_ptr<List>& list);

std::string get_value_string(Value v) {
  std::ostringstream sv;

  switch (get_value_type(v)) {
    case ValueType::Integer:
      sv << std::get<int>(v);
      break;
    case ValueType::Double:
      sv << std::get<double>(v);
      break;
    case ValueType::Boolean:
      sv << std::boolalpha << std::get<bool>(v);
      break;
    case ValueType::String:
      sv << std::get<std::string>(v);
      break;
    case ValueType::List:
      sv << list_to_string(std::get<std::shared_ptr<List>>(v));
      break;
    default:
      // WIP: handle ValueType::None
      break;
  }

  return sv.str();
}

std::string list_to_string(const std::shared_ptr<List>& list) {
  std::ostringstream sv;
  sv << "[";

  for (auto it = list->elements.begin(); it != list->elements.end(); ++it) {
    if (it != list->elements.begin()) {
      sv << ", ";
    }

    if (get_value_type(*it) == ValueType::String) {
      sv << "\"" << get_value_string(*it) << "\"";
    } else {
      sv << get_value_string(*it);
    }
  }

  sv << "]";
  return sv.str();
}

#endif