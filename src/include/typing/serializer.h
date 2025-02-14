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

  static bool assert_typematch(KValue v, KName typeName) {
    switch (typeName) {
      case KName::Types_Any:
        return true;

      case KName::Types_Boolean:
        return v.isBoolean();

      case KName::Types_Float:
        return v.isFloat();

      case KName::Types_Hash:
        return v.isHashmap();

      case KName::Types_Integer:
        return v.isInteger();

      case KName::Types_Lambda:
        return v.isLambda();

      case KName::Types_List:
        return v.isList();

      case KName::Types_None:
        return v.isNull();

      case KName::Types_Object:
        return v.isObject();

      case KName::Types_String:
        return v.isString();

      case KName::Types_Pointer:
        return v.isPointer();

      default:
        break;
    }

    return false;
  }

  static k_string get_value_type_string(KValue v) {
    if (v.isInteger()) {
      return TypeNames.Integer;
    } else if (v.isFloat()) {
      return TypeNames.Float;
    } else if (v.isBoolean()) {
      return TypeNames.Boolean;
    } else if (v.isString()) {
      return TypeNames.String;
    } else if (v.isNull()) {
      return TypeNames.None;
    } else if (v.isList()) {
      return TypeNames.List;
    } else if (v.isHashmap()) {
      return TypeNames.Hashmap;
    } else if (v.isObject()) {
      return v.getObject()->structName;
    } else if (v.isLambda()) {
      return TypeNames.Lambda;
    } else if (v.isPointer()) {
      return TypeNames.Pointer;
    }

    return "";
  }

  static k_list convert_value_to_list(KValue& rhsValues) {
    if (rhsValues.isList()) {
      return rhsValues.getList();
    } else {
      auto newList = std::make_shared<List>();
      newList->elements.emplace_back(rhsValues);
      return newList;
    }
  }

  static k_string serialize(KValue v, bool wrapStrings = false) {
    std::ostringstream sv;

    if (v.isInteger()) {
      sv << v.getInteger();
    } else if (v.isFloat()) {
      sv << v.getFloat();
    } else if (v.isBoolean()) {
      sv << std::boolalpha << v.getBoolean();
    } else if (v.isNull()) {
      sv << Keywords.Null;
    } else if (v.isString()) {
      if (wrapStrings) {
        sv << "\"" << v.getString() << "\"";
      } else {
        sv << v.getString();
      }
    } else if (v.isList()) {
      sv << serialize_list(v.getList());
    } else if (v.isHashmap()) {
      sv << serialize_hash(v.getHashmap());
    } else if (v.isObject()) {
      sv << basic_serialize_object(v.getObject());
    } else if (v.isLambda()) {
      sv << basic_serialize_lambda(v.getLambda());
    } else if (v.isPointer()) {
      sv << k_pointer::serialize(v.getPointer());
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

      if ((*it).isString()) {
        sv << "\"" << serialize(*it) << "\"";
      } else {
        sv << serialize(*it);
      }
    }

    sv << "]";
    return sv.str();
  }

  static k_string pretty_serialize(KValue v, int indent = 0) {
    std::ostringstream sv;

    if (v.isInteger()) {
      sv << v.getInteger();
    } else if (v.isFloat()) {
      sv << v.getFloat();
    } else if (v.isBoolean()) {
      sv << std::boolalpha << v.getBoolean();
    } else if (v.isNull()) {
      sv << Keywords.Null;
    } else if (v.isString()) {
      sv << "\"" << v.getString() << "\"";
    } else if (v.isList()) {
      sv << pretty_serialize_list(v.getList(), indent);
    } else if (v.isHashmap()) {
      sv << pretty_serialize_hash(v.getHashmap(), indent);
    } else if (v.isObject()) {
      sv << basic_serialize_object(v.getObject());
    } else if (v.isLambda()) {
      sv << basic_serialize_lambda(v.getLambda());
    } else if (v.isPointer()) {
      sv << k_pointer::serialize(v.getPointer());
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

      if (item.getList()) {
        sv << pretty_serialize_list_experimental(item.getList(), indent + 2,
                                                 true);
      } else if (item.isHashmap()) {
        sv << pretty_serialize_hash(item.getHashmap(), indent + 2);
      } else if (item.isString()) {
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

      if ((*it).isList()) {
        sv << pretty_serialize_list((*it).getList(), indent + 2);
      } else if ((*it).isHashmap()) {
        sv << pretty_serialize_hash((*it).getHashmap(), indent + 2);
      } else if ((*it).isString()) {
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
      if (v.isHashmap()) {
        sv << pretty_serialize_hash(v.getHashmap(), indent + 2);
      } else if (v.isList()) {
        sv << pretty_serialize_list(v.getList(), indent + 2);
      } else if (v.isString()) {
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

      if (v.isHashmap()) {
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