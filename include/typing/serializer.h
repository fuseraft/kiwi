#ifndef KIWI_TYPING_SERIALIZER_H
#define KIWI_TYPING_SERIALIZER_H

#include <memory>
#include <stdexcept>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include "valuetype.h"

struct Serializer {
  static ValueType get_value_type(Value v) {
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
          } else if constexpr (std::is_same_v<T, std::shared_ptr<Hash>>) {
            type = ValueType::Hash;
          } else {
            type = ValueType::Unknown;
          }
        },
        v);

    return type;
  }

  static std::shared_ptr<List> convert_value_to_list(Value& rhsValues) {
    if (std::holds_alternative<std::shared_ptr<List>>(rhsValues)) {
      return std::get<std::shared_ptr<List>>(rhsValues);
    } else {
      auto newList = std::make_shared<List>();
      newList->elements.push_back(rhsValues);
      return newList;
    }
  }

  static std::string get_value_string(Value v) {
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
      case ValueType::Hash:
        sv << hash_to_string(std::get<std::shared_ptr<Hash>>(v));
        break;
      default:
        // WIP: handle ValueType::None
        break;
    }

    return sv.str();
  }

  static std::string list_to_string(const std::shared_ptr<List>& list) {
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

  static std::shared_ptr<List> get_hash_keys_list(
      const std::shared_ptr<Hash>& hash) {
    std::shared_ptr<List> keys = std::make_shared<List>();
    for (const auto& pair : hash->kvp) {
      keys->elements.push_back(pair.first);
    }
    return keys;
  }

  static std::string hash_to_string(const std::shared_ptr<Hash>& hash) {
    std::ostringstream sv;
    sv << "{";

    std::vector<std::string> keys;
    for (const auto& pair : hash->kvp) {
      keys.push_back(pair.first);
    }

    bool first = true;
    for (std::string key : keys) {
      if (!first) {
        sv << ", ";
      } else {
        first = false;
      }

      sv << "\"" << key << "\": ";
      Value v = hash->kvp[key];

      if (get_value_type(v) == ValueType::Hash) {
        sv << get_value_string(v);
      } else {
        sv << get_value_string(v);
      }
    }

    sv << "}";
    return sv.str();
  }
};

#endif