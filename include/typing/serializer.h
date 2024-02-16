#ifndef KIWI_TYPING_SERIALIZER_H
#define KIWI_TYPING_SERIALIZER_H

#include <memory>
#include <stdexcept>
#include <sstream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include "parsing/keywords.h"
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
          } else if constexpr (std::is_same_v<T, std::shared_ptr<Object>>) {
            type = ValueType::Object;
          } else if constexpr (std::is_same_v<T, std::shared_ptr<LambdaRef>>) {
            type = ValueType::Lambda;
          }
        },
        v);

    return type;
  }

  static std::string get_value_type_string(Value v) {
    switch (get_value_type(v)) {
      case ValueType::Integer:
        return TypeNames.Integer;
      case ValueType::Double:
        return TypeNames.Double;
      case ValueType::Boolean:
        return TypeNames.Boolean;
      case ValueType::String:
        return TypeNames.String;
      case ValueType::List:
        return TypeNames.List;
      case ValueType::Hash:
        return TypeNames.Hash;
      case ValueType::Lambda:
        return TypeNames.Lambda;
      case ValueType::Object:
        return std::get<std::shared_ptr<Object>>(v)->className;
      case ValueType::None:
        return TypeNames.None;
    }

    return "";
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

  static std::string serialize(Value v, bool wrapStrings = false) {
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
        if (wrapStrings) {
          sv << "\"" << std::get<std::string>(v) << "\"";
        } else {
          sv << std::get<std::string>(v);
        }
        break;
      case ValueType::List:
        sv << serialize_list(std::get<std::shared_ptr<List>>(v));
        break;
      case ValueType::Hash:
        sv << serialize_hash(std::get<std::shared_ptr<Hash>>(v));
        break;
      case ValueType::Object:
        sv << basic_serialize_object(std::get<std::shared_ptr<Object>>(v));
        break;
      case ValueType::Lambda:
        sv << basic_serialize_lambda(std::get<std::shared_ptr<LambdaRef>>(v));
        break;
      default:
        // WIP: handle ValueType::None
        break;
    }

    return sv.str();
  }

  static std::string serialize_list(const std::shared_ptr<List>& list) {
    std::ostringstream sv;
    sv << "[";

    for (auto it = list->elements.begin(); it != list->elements.end(); ++it) {
      if (it != list->elements.begin()) {
        sv << ", ";
      }

      if (get_value_type(*it) == ValueType::String) {
        sv << "\"" << serialize(*it) << "\"";
      } else {
        sv << serialize(*it);
      }
    }

    sv << "]";
    return sv.str();
  }

  static std::shared_ptr<List> get_hash_keys_list(
      const std::shared_ptr<Hash>& hash) {
    std::shared_ptr<List> keys = std::make_shared<List>();
    for (const auto& key : hash->keys) {
      keys->elements.push_back(key);
    }
    return keys;
  }

  static std::string basic_serialize_object(
      const std::shared_ptr<Object>& object) {
    return "[Object(class=" + object->className + ", identifier=@" +
           object->identifier + ")]";
  }

  static std::string basic_serialize_lambda(
      const std::shared_ptr<LambdaRef>& lambda) {
    if (lambda->identifier.empty()) {
      return "[" + TypeNames.Lambda + "]";
    }
    return "[" + TypeNames.Lambda + "(identifier=" + lambda->identifier + ")]";
  }

  static std::string serialize_hash(const std::shared_ptr<Hash>& hash) {
    std::ostringstream sv;
    sv << "{";

    bool first = true;
    for (std::string key : hash->keys) {
      if (!first) {
        sv << ", ";
      } else {
        first = false;
      }

      sv << "\"" << key << "\": ";
      Value v = hash->get(key);

      if (get_value_type(v) == ValueType::Hash) {
        sv << serialize(v);
      } else {
        sv << serialize(v, true);
      }
    }

    sv << "}";
    return sv.str();
  }
};

#endif