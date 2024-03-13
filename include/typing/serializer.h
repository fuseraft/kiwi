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
  static std::string get_value_type_string(Value v) {
    if (std::holds_alternative<k_int>(v)) {
      return TypeNames.Integer;
    } else if (std::holds_alternative<double>(v)) {
      return TypeNames.Double;

    } else if (std::holds_alternative<bool>(v)) {
      return TypeNames.Boolean;
    } else if (std::holds_alternative<std::string>(v)) {
      return TypeNames.String;
    } else if (std::holds_alternative<std::shared_ptr<List>>(v)) {
      return TypeNames.List;
    } else if (std::holds_alternative<std::shared_ptr<Hash>>(v)) {
      return TypeNames.Hash;
    } else if (std::holds_alternative<std::shared_ptr<Object>>(v)) {
      return std::get<std::shared_ptr<Object>>(v)->className;
    } else if (std::holds_alternative<std::shared_ptr<LambdaRef>>(v)) {
      return TypeNames.Lambda;
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

    if (std::holds_alternative<k_int>(v)) {
      sv << std::get<k_int>(v);
    } else if (std::holds_alternative<double>(v)) {
      sv << std::get<double>(v);
    } else if (std::holds_alternative<bool>(v)) {
      sv << std::boolalpha << std::get<bool>(v);
    } else if (std::holds_alternative<std::string>(v)) {
      if (wrapStrings) {
        sv << "\"" << std::get<std::string>(v) << "\"";
      } else {
        sv << std::get<std::string>(v);
      }
    } else if (std::holds_alternative<std::shared_ptr<List>>(v)) {
      sv << serialize_list(std::get<std::shared_ptr<List>>(v));
    } else if (std::holds_alternative<std::shared_ptr<Hash>>(v)) {
      sv << serialize_hash(std::get<std::shared_ptr<Hash>>(v));
    } else if (std::holds_alternative<std::shared_ptr<Object>>(v)) {
      sv << basic_serialize_object(std::get<std::shared_ptr<Object>>(v));
    } else if (std::holds_alternative<std::shared_ptr<LambdaRef>>(v)) {
      sv << basic_serialize_lambda(std::get<std::shared_ptr<LambdaRef>>(v));
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

      if (std::holds_alternative<std::string>(*it)) {
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
    for (const auto& key : hash->keys) {
      if (!first) {
        sv << ", ";
      } else {
        first = false;
      }

      sv << "\"" << key << "\": ";
      Value v = hash->get(key);

      if (std::holds_alternative<std::shared_ptr<Hash>>(v)) {
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