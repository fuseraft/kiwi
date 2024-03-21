#ifndef KIWI_BUILTINS_COREHANDLER_H
#define KIWI_BUILTINS_COREHANDLER_H

#include <algorithm>
#include <charconv>
#include <sstream>
#include <string>
#include <vector>
#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "tracing/error.h"
#include "typing/serializer.h"
#include "typing/value.h"
#include "util/file.h"
#include "util/string.h"
#include "util/time.h"

class CoreBuiltinHandler {
 public:
  static Value execute(const Token& term, const KName& builtin,
                       const Value& value, const std::vector<Value>& args) {
    if (KiwiBuiltins.is_builtin(builtin)) {
      return executeKiwiBuiltin(term, builtin, value, args);
    }

    throw UnknownBuiltinError(term, "");
  }

 private:
  static Value executeKiwiBuiltin(const Token& term, const KName& builtin,
                                  const Value& value,
                                  const std::vector<Value>& args) {
    switch (builtin) {
      case KName::Builtin_Kiwi_Chars:
        return executeChars(term, value, args);

      case KName::Builtin_Kiwi_IsA:
        return executeIsA(term, value, args);

      case KName::Builtin_Kiwi_Join:
        return executeJoin(term, value, args);

      case KName::Builtin_Kiwi_Split:
        return executeSplit(term, value, args);

      case KName::Builtin_Kiwi_LeftTrim:
        return executeLeftTrim(term, value, args);

      case KName::Builtin_Kiwi_RightTrim:
        return executeRightTrim(term, value, args);

      case KName::Builtin_Kiwi_Trim:
        return executeTrim(term, value, args);

      case KName::Builtin_Kiwi_Type:
        return executeType(term, value, args);

      case KName::Builtin_Kiwi_Size:
        return executeSize(term, value, args);

      case KName::Builtin_Kiwi_ToD:
        return executeToDouble(term, value, args);

      case KName::Builtin_Kiwi_ToI:
        return executeToInteger(term, value, args);

      case KName::Builtin_Kiwi_ToS:
        return executeToString(term, value, args);

      case KName::Builtin_Kiwi_BeginsWith:
        return executeBeginsWith(term, value, args);

      case KName::Builtin_Kiwi_Contains:
        return executeContains(term, value, args);

      case KName::Builtin_Kiwi_EndsWith:
        return executeEndsWith(term, value, args);

      case KName::Builtin_Kiwi_Replace:
        return executeReplace(term, value, args);

      case KName::Builtin_Kiwi_Reverse:
        return executeReverse(term, value, args);

      case KName::Builtin_Kiwi_IndexOf:
        return executeIndexOf(term, value, args);

      case KName::Builtin_Kiwi_LastIndexOf:
        return executeLastIndexOf(term, value, args);

      case KName::Builtin_Kiwi_Upcase:
        return executeUpcase(term, value, args);

      case KName::Builtin_Kiwi_Downcase:
        return executeDowncase(term, value, args);

      case KName::Builtin_Kiwi_Empty:
        return executeEmpty(term, value, args);

      case KName::Builtin_Kiwi_Keys:
        return executeKeys(term, value, args);

      case KName::Builtin_Kiwi_HasKey:
        return executeHasKey(term, value, args);

      default:
        break;
    }

    throw UnknownBuiltinError(term, "");
  }

  static Value executeChars(const Token& term, const Value& value,
                            const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Chars);
    }

    auto newList = std::make_shared<List>();
    k_string stringValue = get_string(term, value);
    for (char c : stringValue) {
      newList->elements.push_back(k_string(1, c));
    }
    return newList;
  }

  static Value executeJoin(const Token& term, const Value& value,
                           const std::vector<Value>& args) {
    int argSize = args.size();

    if (argSize != 0 && argSize != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Join);
    }

    if (!std::holds_alternative<std::shared_ptr<List>>(value)) {
      throw ConversionError(term, "Cannot join a non-list type.");
    }

    auto list = std::get<std::shared_ptr<List>>(value);
    std::ostringstream sv;
    k_string joiner;

    if (argSize == 1) {
      joiner = get_string(term, args.at(0));
    }

    for (auto it = list->elements.begin(); it != list->elements.end(); ++it) {
      if (it != list->elements.begin()) {
        sv << joiner;
      }
      sv << Serializer::serialize(*it);
    }

    return sv.str();
  }

  static Value executeSize(const Token& term, const Value& value,
                           const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Size);
    }

    if (std::holds_alternative<k_string>(value)) {
      return static_cast<k_int>(std::get<k_string>(value).length());
    } else if (std::holds_alternative<std::shared_ptr<List>>(value)) {
      return static_cast<k_int>(
          std::get<std::shared_ptr<List>>(value)->elements.size());
    } else if (std::holds_alternative<std::shared_ptr<Hash>>(value)) {
      return static_cast<k_int>(std::get<std::shared_ptr<Hash>>(value)->size());
    }

    throw ConversionError(term);
  }

  static Value executeToDouble(const Token& term, const Value& value,
                               const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.ToD);
    }

    if (std::holds_alternative<k_string>(value)) {
      k_string stringValue = std::get<k_string>(value);
      double doubleValue = 0;
      auto [ptr, ec] =
          std::from_chars(stringValue.data(),
                          stringValue.data() + stringValue.size(), doubleValue);

      if (ec == std::errc()) {
        return doubleValue;
      } else {
        throw ConversionError(
            term, "Cannot convert non-numeric value to a double: `" +
                      stringValue + "`");
      }
    } else if (std::holds_alternative<k_int>(value)) {
      return static_cast<double>(std::get<k_int>(value));
    } else {
      throw ConversionError(term,
                            "Cannot convert non-numeric value to a double.");
    }
  }

  static Value executeToInteger(const Token& term, const Value& value,
                                const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.ToI);
    }

    if (std::holds_alternative<k_string>(value)) {
      k_string stringValue = std::get<k_string>(value);
      int intValue = 0;
      auto [ptr, ec] =
          std::from_chars(stringValue.data(),
                          stringValue.data() + stringValue.size(), intValue);

      if (ec == std::errc()) {
        return static_cast<k_int>(intValue);
      } else {
        throw ConversionError(
            term, "Cannot convert non-numeric value to an integer: `" +
                      stringValue + "`");
      }
    } else if (std::holds_alternative<double>(value)) {
      return static_cast<k_int>(std::get<double>(value));
    } else {
      throw ConversionError(term,
                            "Cannot convert non-numeric value to an integer.");
    }
  }

  static Value executeToString(const Token& term, const Value& value,
                               const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.ToS);
    }

    return Serializer::serialize(value);
  }

  static Value executeSplit(const Token& term, const Value& value,
                            const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Split);
    }

    k_string input = get_string(term, value);
    auto delimiter = get_string(term, args.at(0));
    auto newList = std::make_shared<List>();

    if (delimiter.empty()) {
      for (char c : input) {
        newList->elements.push_back(k_string(1, c));
      }
    } else {
      for (k_string token : String::split(input, delimiter)) {
        newList->elements.push_back(token);
      }
    }

    return newList;
  }

  static Value executeLeftTrim(const Token& term, const Value& value,
                               const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.LeftTrim);
    }

    k_string input = get_string(term, value);
    return String::trimLeft(input);
  }

  static Value executeRightTrim(const Token& term, const Value& value,
                                const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.RightTrim);
    }

    k_string input = get_string(term, value);
    return String::trimRight(input);
  }

  static Value executeTrim(const Token& term, const Value& value,
                           const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Trim);
    }

    k_string input = get_string(term, value);
    return String::trim(input);
  }

  static Value executeType(const Token& term, const Value& value,
                           const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Type);
    }

    return Serializer::get_value_type_string(value);
  }

  static Value executeHasKey(const Token& term, const Value& value,
                             const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.HasKey);
    }

    if (!std::holds_alternative<std::shared_ptr<Hash>>(value)) {
      throw InvalidOperationError(
          term, "Attempted to retrieve keys from non-Hash type.");
    }

    auto key = get_string(term, args.at(0));
    auto hash = std::get<std::shared_ptr<Hash>>(value);

    return hash->hasKey(key);
  }

  static Value executeKeys(const Token& term, const Value& value,
                           const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Keys);
    }

    if (!std::holds_alternative<std::shared_ptr<Hash>>(value)) {
      throw InvalidOperationError(
          term, "Attempted to retrieve keys from non-Hash type.");
    }

    auto hash = std::get<std::shared_ptr<Hash>>(value);

    return Serializer::get_hash_keys_list(hash);
  }

  static Value executeBeginsWith(const Token& term, const Value& value,
                                 const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.BeginsWith);
    }

    auto str = get_string(term, value);
    auto search = get_string(term, args.at(0));
    return String::beginsWith(str, search);
  }

  static Value executeStringContains(const Token& term, const Value& value,
                                     const Value& arg) {
    auto str = get_string(term, value);
    auto search = get_string(term, arg);
    return String::contains(str, search);
  }

  static Value executeListContains(const Value& value, const Value& arg) {
    auto list = std::get<std::shared_ptr<List>>(value);

    for (const auto& item : list->elements) {
      if (same_value(item, arg)) {
        return true;
      }
    }

    return false;
  }

  static Value executeContains(const Token& term, const Value& value,
                               const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Contains);
    }

    if (std::holds_alternative<k_string>(value)) {
      return executeStringContains(term, value, args.at(0));
    } else if (std::holds_alternative<std::shared_ptr<List>>(value)) {
      return executeListContains(value, args.at(0));
    }

    throw ConversionError(term, "Expected a `String` or `List` value.");
  }

  static Value executeEndsWith(const Token& term, const Value& value,
                               const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Contains);
    }

    auto str = get_string(term, value);
    auto search = get_string(term, args.at(0));
    return String::endsWith(str, search);
  }

  static Value executeIsA(const Token& term, const Value& value,
                          const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.IsA);
    }

    auto typeName = get_string(term, args.at(0));
    if (!TypeNames.is_typename(typeName)) {
      if (std::holds_alternative<std::shared_ptr<Object>>(value)) {
        std::shared_ptr<Object> object =
            std::get<std::shared_ptr<Object>>(value);
        return object->className == typeName;
      }
      throw InvalidTypeNameError(term, typeName);
    }

    return (typeName == TypeNames.Boolean &&
            std::holds_alternative<bool>(value)) ||
           (typeName == TypeNames.Double &&
            std::holds_alternative<double>(value)) ||
           (typeName == TypeNames.Hash &&
            std::holds_alternative<std::shared_ptr<Hash>>(value)) ||
           (typeName == TypeNames.Integer &&
            std::holds_alternative<k_int>(value)) ||
           (typeName == TypeNames.List &&
            std::holds_alternative<std::shared_ptr<List>>(value)) ||
           (typeName == TypeNames.Object &&
            std::holds_alternative<std::shared_ptr<Object>>(value)) ||
           (typeName == TypeNames.Lambda &&
            std::holds_alternative<std::shared_ptr<LambdaRef>>(value)) ||
           (typeName == TypeNames.String &&
            std::holds_alternative<k_string>(value));
  }

  static Value executeReplace(const Token& term, const Value& value,
                              const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Replace);
    }

    auto str = get_string(term, value);
    auto search = get_string(term, args.at(0));
    auto replacement = get_string(term, args.at(1));
    return String::replace(str, search, replacement);
  }

  static Value executeReverse(const Token& term, const Value& value,
                              const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Reverse);
    }

    if (std::holds_alternative<k_string>(value)) {
      auto s = std::get<k_string>(value);
      std::reverse(s.begin(), s.end());
      return s;
    } else if (std::holds_alternative<std::shared_ptr<List>>(value)) {
      auto v = std::get<std::shared_ptr<List>>(value)->elements;
      std::reverse(v.begin(), v.end());
      auto list = std::make_shared<List>();
      list->elements = v;
      return list;
    }

    throw ConversionError(term,
                          "Expected a `String` or a `List` for builtin `" +
                              KiwiBuiltins.Reverse + "`.");
  }

  static Value executeIndexOf(const Token& term, const Value& value,
                              const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.IndexOf);
    }

    if (std::holds_alternative<k_string>(value)) {
      return static_cast<k_int>(String::indexOf(std::get<k_string>(value),
                                                get_string(term, args.at(0))));
    } else if (std::holds_alternative<std::shared_ptr<List>>(value)) {
      return indexof_listvalue(std::get<std::shared_ptr<List>>(value),
                               args.at(0));
    }

    throw ConversionError(term,
                          "Expected a `String` or a `List` for builtin `" +
                              KiwiBuiltins.IndexOf + "`.");
  }

  static Value executeLastIndexOf(const Token& term, const Value& value,
                                  const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.LastIndexOf);
    }

    if (std::holds_alternative<k_string>(value)) {
      return static_cast<k_int>(String::lastIndexOf(
          std::get<k_string>(value), get_string(term, args.at(0))));
    } else if (std::holds_alternative<std::shared_ptr<List>>(value)) {
      return lastindexof_listvalue(std::get<std::shared_ptr<List>>(value),
                                   args.at(0));
    }

    throw ConversionError(term,
                          "Expected a `String` or a `List` for builtin `" +
                              KiwiBuiltins.LastIndexOf + "`.");
  }

  static Value executeUpcase(const Token& term, const Value& value,
                             const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Upcase);
    }

    return String::toUppercase(get_string(term, value));
  }

  static Value executeDowncase(const Token& term, const Value& value,
                               const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Downcase);
    }

    return String::toLowercase(get_string(term, value));
  }

  static Value executeEmpty(const Token& term, const Value& value,
                            const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Empty);
    }

    if (std::holds_alternative<k_string>(value)) {
      return std::get<k_string>(value).empty();
    } else if (std::holds_alternative<std::shared_ptr<List>>(value)) {
      return std::get<std::shared_ptr<List>>(value)->elements.empty();
    } else if (std::holds_alternative<std::shared_ptr<Hash>>(value)) {
      return std::get<std::shared_ptr<Hash>>(value)->keys.empty();
    } else if (std::holds_alternative<k_int>(value)) {
      return std::get<k_int>(value) == 0;
    } else if (std::holds_alternative<double>(value)) {
      return std::get<double>(value) == 0.0;
    } else if (std::holds_alternative<bool>(value)) {
      return !std::get<bool>(value);
    }

    throw ConversionError(
        term, "Invalid type for builtin `" + KiwiBuiltins.Empty + "`.");
  }
};

#endif