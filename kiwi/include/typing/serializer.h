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
#include "value.h"

struct Serializer {
  static k_string get_typename_string(KName typeName) {
    switch (typeName) {
      case KName::Types_Any:
        return TypeNames.Any;

      case KName::Types_Boolean:
        return TypeNames.Boolean;

      case KName::Types_Float:
        return TypeNames.Float;

      case KName::Types_Hash:
        return TypeNames.Hashmap;

      case KName::Types_Integer:
        return TypeNames.Integer;

      case KName::Types_Lambda:
        return TypeNames.Lambda;

      case KName::Types_List:
        return TypeNames.List;

      case KName::Types_None:
        return TypeNames.None;

      case KName::Types_Object:
        return TypeNames.Object;

      case KName::Types_String:
        return TypeNames.String;

      case KName::Types_Pointer:
        return TypeNames.Pointer;

      default:
        break;
    }

    return "";
  }

  static bool assert_typematch(k_value v, KName typeName) {
    switch (typeName) {
      case KName::Types_Any:
        return true;

      case KName::Types_Boolean:
        return std::holds_alternative<bool>(v);

      case KName::Types_Float:
        return std::holds_alternative<double>(v);

      case KName::Types_Hash:
        return std::holds_alternative<k_hashmap>(v);

      case KName::Types_Integer:
        return std::holds_alternative<k_int>(v);

      case KName::Types_Lambda:
        return std::holds_alternative<k_lambda>(v);

      case KName::Types_List:
        return std::holds_alternative<k_list>(v);

      case KName::Types_None:
        return std::holds_alternative<k_null>(v);

      case KName::Types_Object:
        return std::holds_alternative<k_object>(v);

      case KName::Types_String:
        return std::holds_alternative<k_string>(v);

      case KName::Types_Pointer:
        return std::holds_alternative<k_pointer>(v);

      default:
        break;
    }

    return false;
  }

  static k_string get_value_type_string(k_value v) {
    if (std::holds_alternative<k_int>(v)) {
      return TypeNames.Integer;
    } else if (std::holds_alternative<double>(v)) {
      return TypeNames.Float;
    } else if (std::holds_alternative<bool>(v)) {
      return TypeNames.Boolean;
    } else if (std::holds_alternative<k_string>(v)) {
      return TypeNames.String;
    } else if (std::holds_alternative<k_null>(v)) {
      return TypeNames.None;
    } else if (std::holds_alternative<k_list>(v)) {
      return TypeNames.List;
    } else if (std::holds_alternative<k_hashmap>(v)) {
      return TypeNames.Hashmap;
    } else if (std::holds_alternative<k_object>(v)) {
      return std::get<k_object>(v)->structName;
    } else if (std::holds_alternative<k_lambda>(v)) {
      return TypeNames.Lambda;
    } else if (std::holds_alternative<k_pointer>(v)) {
      return TypeNames.Pointer;
    }

    return "";
  }

  static k_list convert_value_to_list(k_value& rhsValues) {
    if (std::holds_alternative<k_list>(rhsValues)) {
      return std::get<k_list>(rhsValues);
    } else {
      auto newList = std::make_shared<List>();
      newList->elements.emplace_back(rhsValues);
      return newList;
    }
  }

  static k_string serialize(k_value v, bool wrapStrings = false) {
    std::ostringstream sv;

    if (std::holds_alternative<k_int>(v)) {
      sv << std::get<k_int>(v);
    } else if (std::holds_alternative<double>(v)) {
      sv << std::get<double>(v);
    } else if (std::holds_alternative<bool>(v)) {
      sv << std::boolalpha << std::get<bool>(v);
    } else if (std::holds_alternative<k_null>(v)) {
      sv << Keywords.Null;
    } else if (std::holds_alternative<k_string>(v)) {
      if (wrapStrings) {
        sv << "\"" << std::get<k_string>(v) << "\"";
      } else {
        sv << std::get<k_string>(v);
      }
    } else if (std::holds_alternative<k_list>(v)) {
      sv << serialize_list(std::get<k_list>(v));
    } else if (std::holds_alternative<k_hashmap>(v)) {
      sv << serialize_hash(std::get<k_hashmap>(v));
    } else if (std::holds_alternative<k_object>(v)) {
      sv << basic_serialize_object(std::get<k_object>(v));
    } else if (std::holds_alternative<k_lambda>(v)) {
      sv << basic_serialize_lambda(std::get<k_lambda>(v));
    } else if (std::holds_alternative<k_pointer>(v)) {
      sv << k_pointer::serialize(std::get<k_pointer>(v));
    }

    return sv.str();
  }

  static k_string serialize_list(const k_list& list) {
    std::ostringstream sv;
    sv << "[";

    for (auto it = list->elements.begin(); it != list->elements.end(); ++it) {
      if (it != list->elements.begin()) {
        sv << ", ";
      }

      if (std::holds_alternative<k_string>(*it)) {
        sv << "\"" << serialize(*it) << "\"";
      } else {
        sv << serialize(*it);
      }
    }

    sv << "]";
    return sv.str();
  }

  static k_string pretty_serialize(k_value v, int indent = 0) {
    std::ostringstream sv;

    if (std::holds_alternative<k_int>(v)) {
      sv << std::get<k_int>(v);
    } else if (std::holds_alternative<double>(v)) {
      sv << std::get<double>(v);
    } else if (std::holds_alternative<bool>(v)) {
      sv << std::boolalpha << std::get<bool>(v);
    } else if (std::holds_alternative<k_null>(v)) {
      sv << Keywords.Null;
    } else if (std::holds_alternative<k_string>(v)) {
      sv << "\"" << std::get<k_string>(v) << "\"";
    } else if (std::holds_alternative<k_list>(v)) {
      sv << pretty_serialize_list(std::get<k_list>(v), indent);
    } else if (std::holds_alternative<k_hashmap>(v)) {
      sv << pretty_serialize_hash(std::get<k_hashmap>(v), indent);
    } else if (std::holds_alternative<k_object>(v)) {
      sv << basic_serialize_object(std::get<k_object>(v));
    } else if (std::holds_alternative<k_lambda>(v)) {
      sv << basic_serialize_lambda(std::get<k_lambda>(v));
    } else if (std::holds_alternative<k_pointer>(v)) {
      sv << k_pointer::serialize(std::get<k_pointer>(v));
    }

    return sv.str();
  }

  static k_string pretty_serialize_list_experimental(const k_list& list,
                                                     int indent = 0,
                                                     bool isNested = false) {
    std::ostringstream sv;
    if (!isNested) {
      sv << "[" << std::endl;
    } else {
      sv << "[";
    }

    k_string indentString(indent + 2, ' ');
    bool first = true;

    for (const auto& item : list->elements) {
      if (!first) {
        sv << ", ";
      } else {
        first = false;
        if (!isNested) {
          sv << indentString;
        }
      }

      if (std::holds_alternative<k_list>(item)) {
        sv << pretty_serialize_list_experimental(std::get<k_list>(item),
                                                 indent + 2, true);
      } else if (std::holds_alternative<k_hashmap>(item)) {
        sv << pretty_serialize_hash(std::get<k_hashmap>(item), indent + 2);
      } else if (std::holds_alternative<k_string>(item)) {
        sv << "\"" << serialize(item) << "\"";
      } else {
        sv << serialize(item);
      }
    }

    if (!isNested) {
      sv << std::endl << k_string(indent, ' ') << "]";
    } else {
      sv << "]";
    }
    return sv.str();
  }

  static k_string pretty_serialize_list(const k_list& list, int indent = 0) {
    std::ostringstream sv;
    sv << "[" << std::endl;
    k_string indentString(indent + 2, ' ');

    for (auto it = list->elements.begin(); it != list->elements.end(); ++it) {
      if (it != list->elements.begin()) {
        sv << "," << std::endl;
      }

      sv << indentString;

      if (std::holds_alternative<k_list>(*it)) {
        sv << pretty_serialize_list(std::get<k_list>(*it), indent + 2);
      } else if (std::holds_alternative<k_hashmap>(*it)) {
        sv << pretty_serialize_hash(std::get<k_hashmap>(*it), indent + 2);
      } else if (std::holds_alternative<k_string>(*it)) {
        sv << "\"" << serialize(*it) << "\"";
      } else {
        sv << serialize(*it);
      }
    }

    sv << std::endl << k_string(indent, ' ') << "]";
    return sv.str();
  }

  static k_string pretty_serialize_hash(const k_hashmap& hash, int indent = 0) {
    std::ostringstream sv;
    sv << "{" << std::endl;
    k_string indentString(indent + 2, ' ');

    bool first = true;
    auto& keys = hash->keys;
    for (const auto& key : keys) {
      if (!first) {
        sv << "," << std::endl;
      } else {
        first = false;
      }
      sv << indentString << serialize(key, true) << ": ";

      auto v = hash->get(key);
      if (std::holds_alternative<k_hashmap>(v)) {
        sv << pretty_serialize_hash(std::get<k_hashmap>(v), indent + 2);
      } else if (std::holds_alternative<k_list>(v)) {
        sv << pretty_serialize_list(std::get<k_list>(v), indent + 2);
      } else if (std::holds_alternative<k_string>(v)) {
        sv << "\"" << serialize(v) << "\"";
      } else {
        sv << serialize(v, true);
      }
    }

    sv << std::endl << k_string(indent, ' ') << "}";
    return sv.str();
  }

  static k_list get_hash_keys_list(const k_hashmap& hash) {
    auto keys = std::make_shared<List>();
    auto& elements = keys->elements;
    elements.reserve(hash->keys.size());

    for (const auto& key : hash->keys) {
      elements.emplace_back(key);
    }

    return keys;
  }

  static k_list get_hash_values_list(const k_hashmap& hash) {
    auto values = std::make_shared<List>();
    auto& elements = values->elements;
    elements.reserve(hash->keys.size());

    for (const auto& key : hash->keys) {
      elements.emplace_back(hash->kvp[key]);
    }

    return values;
  }

  static k_string basic_serialize_object(const k_object& object) {
    if (object->identifier.empty()) {
      return "@(struct=" + object->structName + ")";
    } else {
      return "@(struct=" + object->structName +
             ", identifier=" + object->identifier + ")";
    }
  }

  static k_string basic_serialize_lambda(const k_lambda& lambda) {
    if (lambda->identifier.empty()) {
      return "[" + TypeNames.Lambda + "]";
    }
    return "[" + TypeNames.Lambda + "(identifier=" + lambda->identifier + ")]";
  }

  static k_string serialize_hash(const k_hashmap& hash) {
    std::ostringstream sv;
    sv << "{";

    bool first = true;
    auto& keys = hash->keys;

    for (const auto& key : keys) {
      if (!first) {
        sv << ", ";
      } else {
        first = false;
      }

      sv << serialize(key, true) << ": ";
      auto v = hash->get(key);

      if (std::holds_alternative<k_hashmap>(v)) {
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