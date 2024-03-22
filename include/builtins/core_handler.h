#ifndef ASTRAL_BUILTINS_COREHANDLER_H
#define ASTRAL_BUILTINS_COREHANDLER_H

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
  static k_value execute(const Token& term, const KName& builtin,
                         const k_value& value,
                         const std::vector<k_value>& args) {
    if (AstralBuiltins.is_builtin(builtin)) {
      return executeAstralBuiltin(term, builtin, value, args);
    }

    throw UnknownBuiltinError(term, "");
  }

 private:
  static k_value executeAstralBuiltin(const Token& term, const KName& builtin,
                                      const k_value& value,
                                      const std::vector<k_value>& args) {
    switch (builtin) {
      case KName::Builtin_Astral_Chars:
        return executeChars(term, value, args);

      case KName::Builtin_Astral_IsA:
        return executeIsA(term, value, args);

      case KName::Builtin_Astral_Join:
        return executeJoin(term, value, args);

      case KName::Builtin_Astral_Split:
        return executeSplit(term, value, args);

      case KName::Builtin_Astral_LeftTrim:
        return executeLeftTrim(term, value, args);

      case KName::Builtin_Astral_RightTrim:
        return executeRightTrim(term, value, args);

      case KName::Builtin_Astral_Trim:
        return executeTrim(term, value, args);

      case KName::Builtin_Astral_Type:
        return executeType(term, value, args);

      case KName::Builtin_Astral_Size:
        return executeSize(term, value, args);

      case KName::Builtin_Astral_ToD:
        return executeToDouble(term, value, args);

      case KName::Builtin_Astral_ToI:
        return executeToInteger(term, value, args);

      case KName::Builtin_Astral_ToS:
        return executeToString(term, value, args);

      case KName::Builtin_Astral_BeginsWith:
        return executeBeginsWith(term, value, args);

      case KName::Builtin_Astral_Contains:
        return executeContains(term, value, args);

      case KName::Builtin_Astral_EndsWith:
        return executeEndsWith(term, value, args);

      case KName::Builtin_Astral_Replace:
        return executeReplace(term, value, args);

      case KName::Builtin_Astral_Reverse:
        return executeReverse(term, value, args);

      case KName::Builtin_Astral_IndexOf:
        return executeIndexOf(term, value, args);

      case KName::Builtin_Astral_LastIndexOf:
        return executeLastIndexOf(term, value, args);

      case KName::Builtin_Astral_Upcase:
        return executeUpcase(term, value, args);

      case KName::Builtin_Astral_Downcase:
        return executeDowncase(term, value, args);

      case KName::Builtin_Astral_Empty:
        return executeEmpty(term, value, args);

      case KName::Builtin_Astral_Keys:
        return executeKeys(term, value, args);

      case KName::Builtin_Astral_HasKey:
        return executeHasKey(term, value, args);

      default:
        break;
    }

    throw UnknownBuiltinError(term, "");
  }

  static k_value executeChars(const Token& term, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Chars);
    }

    auto newList = std::make_shared<List>();
    k_string stringValue = get_string(term, value);
    for (char c : stringValue) {
      newList->elements.push_back(k_string(1, c));
    }
    return newList;
  }

  static k_value executeJoin(const Token& term, const k_value& value,
                             const std::vector<k_value>& args) {
    int argSize = args.size();

    if (argSize != 0 && argSize != 1) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Join);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw ConversionError(term, "Cannot join a non-list type.");
    }

    auto list = std::get<k_list>(value);
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

  static k_value executeSize(const Token& term, const k_value& value,
                             const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Size);
    }

    if (std::holds_alternative<k_string>(value)) {
      return static_cast<k_int>(std::get<k_string>(value).length());
    } else if (std::holds_alternative<k_list>(value)) {
      return static_cast<k_int>(std::get<k_list>(value)->elements.size());
    } else if (std::holds_alternative<k_hash>(value)) {
      return static_cast<k_int>(std::get<k_hash>(value)->size());
    }

    throw ConversionError(term);
  }

  static k_value executeToDouble(const Token& term, const k_value& value,
                                 const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.ToD);
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

  static k_value executeToInteger(const Token& term, const k_value& value,
                                  const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.ToI);
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

  static k_value executeToString(const Token& term, const k_value& value,
                                 const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.ToS);
    }

    return Serializer::serialize(value);
  }

  static k_value executeSplit(const Token& term, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Split);
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

  static k_value executeLeftTrim(const Token& term, const k_value& value,
                                 const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.LeftTrim);
    }

    k_string input = get_string(term, value);
    return String::trimLeft(input);
  }

  static k_value executeRightTrim(const Token& term, const k_value& value,
                                  const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.RightTrim);
    }

    k_string input = get_string(term, value);
    return String::trimRight(input);
  }

  static k_value executeTrim(const Token& term, const k_value& value,
                             const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Trim);
    }

    k_string input = get_string(term, value);
    return String::trim(input);
  }

  static k_value executeType(const Token& term, const k_value& value,
                             const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Type);
    }

    return Serializer::get_value_type_string(value);
  }

  static k_value executeHasKey(const Token& term, const k_value& value,
                               const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.HasKey);
    }

    if (!std::holds_alternative<k_hash>(value)) {
      throw InvalidOperationError(
          term, "Attempted to retrieve keys from non-Hash type.");
    }

    auto key = get_string(term, args.at(0));
    auto hash = std::get<k_hash>(value);

    return hash->hasKey(key);
  }

  static k_value executeKeys(const Token& term, const k_value& value,
                             const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Keys);
    }

    if (!std::holds_alternative<k_hash>(value)) {
      throw InvalidOperationError(
          term, "Attempted to retrieve keys from non-Hash type.");
    }

    auto hash = std::get<k_hash>(value);

    return Serializer::get_hash_keys_list(hash);
  }

  static k_value executeBeginsWith(const Token& term, const k_value& value,
                                   const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.BeginsWith);
    }

    auto str = get_string(term, value);
    auto search = get_string(term, args.at(0));
    return String::beginsWith(str, search);
  }

  static k_value executeStringContains(const Token& term, const k_value& value,
                                       const k_value& arg) {
    auto str = get_string(term, value);
    auto search = get_string(term, arg);
    return String::contains(str, search);
  }

  static k_value executeListContains(const k_value& value, const k_value& arg) {
    auto list = std::get<k_list>(value);

    for (const auto& item : list->elements) {
      if (same_value(item, arg)) {
        return true;
      }
    }

    return false;
  }

  static k_value executeContains(const Token& term, const k_value& value,
                                 const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Contains);
    }

    if (std::holds_alternative<k_string>(value)) {
      return executeStringContains(term, value, args.at(0));
    } else if (std::holds_alternative<k_list>(value)) {
      return executeListContains(value, args.at(0));
    }

    throw ConversionError(term, "Expected a `String` or `List` value.");
  }

  static k_value executeEndsWith(const Token& term, const k_value& value,
                                 const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Contains);
    }

    auto str = get_string(term, value);
    auto search = get_string(term, args.at(0));
    return String::endsWith(str, search);
  }

  static k_value executeIsA(const Token& term, const k_value& value,
                            const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.IsA);
    }

    auto typeName = get_string(term, args.at(0));
    if (!TypeNames.is_typename(typeName)) {
      if (std::holds_alternative<k_object>(value)) {
        k_object object = std::get<k_object>(value);
        return object->className == typeName;
      }
      throw InvalidTypeNameError(term, typeName);
    }

    return (typeName == TypeNames.Boolean &&
            std::holds_alternative<bool>(value)) ||
           (typeName == TypeNames.Double &&
            std::holds_alternative<double>(value)) ||
           (typeName == TypeNames.Hash &&
            std::holds_alternative<k_hash>(value)) ||
           (typeName == TypeNames.Integer &&
            std::holds_alternative<k_int>(value)) ||
           (typeName == TypeNames.List &&
            std::holds_alternative<k_list>(value)) ||
           (typeName == TypeNames.Object &&
            std::holds_alternative<k_object>(value)) ||
           (typeName == TypeNames.Lambda &&
            std::holds_alternative<k_lambda>(value)) ||
           (typeName == TypeNames.String &&
            std::holds_alternative<k_string>(value));
  }

  static k_value executeReplace(const Token& term, const k_value& value,
                                const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Replace);
    }

    auto str = get_string(term, value);
    auto search = get_string(term, args.at(0));
    auto replacement = get_string(term, args.at(1));
    return String::replace(str, search, replacement);
  }

  static k_value executeReverse(const Token& term, const k_value& value,
                                const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Reverse);
    }

    if (std::holds_alternative<k_string>(value)) {
      auto s = std::get<k_string>(value);
      std::reverse(s.begin(), s.end());
      return s;
    } else if (std::holds_alternative<k_list>(value)) {
      auto v = std::get<k_list>(value)->elements;
      std::reverse(v.begin(), v.end());
      auto list = std::make_shared<List>();
      list->elements = v;
      return list;
    }

    throw ConversionError(term,
                          "Expected a `String` or a `List` for builtin `" +
                              AstralBuiltins.Reverse + "`.");
  }

  static k_value executeIndexOf(const Token& term, const k_value& value,
                                const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.IndexOf);
    }

    if (std::holds_alternative<k_string>(value)) {
      return static_cast<k_int>(String::indexOf(std::get<k_string>(value),
                                                get_string(term, args.at(0))));
    } else if (std::holds_alternative<k_list>(value)) {
      return indexof_listvalue(std::get<k_list>(value), args.at(0));
    }

    throw ConversionError(term,
                          "Expected a `String` or a `List` for builtin `" +
                              AstralBuiltins.IndexOf + "`.");
  }

  static k_value executeLastIndexOf(const Token& term, const k_value& value,
                                    const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.LastIndexOf);
    }

    if (std::holds_alternative<k_string>(value)) {
      return static_cast<k_int>(String::lastIndexOf(
          std::get<k_string>(value), get_string(term, args.at(0))));
    } else if (std::holds_alternative<k_list>(value)) {
      return lastindexof_listvalue(std::get<k_list>(value), args.at(0));
    }

    throw ConversionError(term,
                          "Expected a `String` or a `List` for builtin `" +
                              AstralBuiltins.LastIndexOf + "`.");
  }

  static k_value executeUpcase(const Token& term, const k_value& value,
                               const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Upcase);
    }

    return String::toUppercase(get_string(term, value));
  }

  static k_value executeDowncase(const Token& term, const k_value& value,
                                 const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Downcase);
    }

    return String::toLowercase(get_string(term, value));
  }

  static k_value executeEmpty(const Token& term, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Empty);
    }

    if (std::holds_alternative<k_string>(value)) {
      return std::get<k_string>(value).empty();
    } else if (std::holds_alternative<k_list>(value)) {
      return std::get<k_list>(value)->elements.empty();
    } else if (std::holds_alternative<k_hash>(value)) {
      return std::get<k_hash>(value)->keys.empty();
    } else if (std::holds_alternative<k_int>(value)) {
      return std::get<k_int>(value) == 0;
    } else if (std::holds_alternative<double>(value)) {
      return std::get<double>(value) == 0.0;
    } else if (std::holds_alternative<bool>(value)) {
      return !std::get<bool>(value);
    }

    throw ConversionError(
        term, "Invalid type for builtin `" + AstralBuiltins.Empty + "`.");
  }
};

#endif