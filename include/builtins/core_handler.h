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

      case KName::Builtin_Astral_ToBytes:
        return executeToBytes(term, value, args);

      case KName::Builtin_Astral_ToHex:
        return executeToHex(term, value, args);

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
    auto stringValue = get_string(term, value);
    auto& elements = newList->elements;

    for (char c : stringValue) {
      elements.emplace_back(k_string(1, c));
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
    auto& elements = list->elements;
    std::ostringstream sv;
    k_string joiner;

    if (argSize == 1) {
      joiner = get_string(term, args.at(0));
    }

    for (auto it = elements.begin(); it != elements.end(); ++it) {
      if (it != elements.begin()) {
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

  static k_value executeToHex(const Token& term, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.ToHex);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw ConversionError(
          term, "Expected a `List` value for byte to string conversion.");
    }

    auto& elements = std::get<k_list>(value)->elements;

    if (elements.empty()) {
      throw EmptyListError(term);
    }

    std::stringstream ss;

    for (const auto& item : elements) {
      if (!std::holds_alternative<k_int>(item)) {
        throw ConversionError(
            term, "Expected an `Integer` value for byte to string conversion.");
      }

      auto itemValue = std::get<k_int>(item);
      auto byte = static_cast<unsigned int>(itemValue) & 0xFF;
      ss << std::hex << std::setw(2) << std::setfill('0') << byte;
    }

    return ss.str();
  }

  static k_value executeToBytes(const Token& term, const k_value& value,
                                const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.ToBytes);
    }

    if (std::holds_alternative<k_string>(value)) {
      auto stringValue = std::get<k_string>(value);
      std::vector<uint8_t> bytes(stringValue.begin(), stringValue.end());
      auto byteList = std::make_shared<List>();
      auto& elements = byteList->elements;

      for (const auto& byte : bytes) {
        elements.emplace_back(static_cast<k_int>(byte));
      }

      return byteList;
    } else if (std::holds_alternative<k_list>(value)) {
      auto listElements = std::get<k_list>(value)->elements;
      auto byteList = std::make_shared<List>();
      auto& elements = byteList->elements;

      for (const auto& item : listElements) {
        if (!std::holds_alternative<k_string>(item)) {
          throw ConversionError(
              term, "Expected a `List` to contain only `String` values.");
        }

        auto stringValue = std::get<k_string>(item);
        std::vector<uint8_t> bytes(stringValue.begin(), stringValue.end());

        for (const auto& byte : bytes) {
          elements.emplace_back(static_cast<k_int>(byte));
        }
      }

      return byteList;
    } else {
      throw ConversionError(
          term, "Expected a `String` or `List` to convert to bytes.");
    }
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
    auto& elements = newList->elements;

    if (delimiter.empty()) {
      for (char c : input) {
        elements.emplace_back(k_string(1, c));
      }
    } else {
      for (k_string token : String::split(input, delimiter)) {
        elements.emplace_back(token);
      }
    }

    return newList;
  }

  static k_value executeLeftTrim(const Token& term, const k_value& value,
                                 const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.LeftTrim);
    }

    return String::trimLeft(get_string(term, value));
  }

  static k_value executeRightTrim(const Token& term, const k_value& value,
                                  const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.RightTrim);
    }

    return String::trimRight(get_string(term, value));
  }

  static k_value executeTrim(const Token& term, const k_value& value,
                             const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Trim);
    }

    return String::trim(get_string(term, value));
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

    return std::get<k_hash>(value)->hasKey(get_string(term, args.at(0)));
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

    return Serializer::get_hash_keys_list(std::get<k_hash>(value));
  }

  static k_value executeBeginsWith(const Token& term, const k_value& value,
                                   const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.BeginsWith);
    }

    return String::beginsWith(get_string(term, value),
                              get_string(term, args.at(0)));
  }

  static k_value executeStringContains(const Token& term, const k_value& value,
                                       const k_value& arg) {
    return String::contains(get_string(term, value), get_string(term, arg));
  }

  static k_value executeListContains(const k_value& value, const k_value& arg) {
    auto list = std::get<k_list>(value);
    auto& elements = list->elements;

    for (const auto& item : elements) {
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

    return String::endsWith(get_string(term, value),
                            get_string(term, args.at(0)));
  }

  static k_value executeIsA(const Token& term, const k_value& value,
                            const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.IsA);
    }

    auto typeName = get_string(term, args.at(0));
    if (!TypeNames.is_typename(typeName)) {
      if (std::holds_alternative<k_object>(value)) {
        return same_value(std::get<k_object>(value)->className, typeName);
      }
      throw InvalidTypeNameError(term, typeName);
    }

    switch (value.index()) {
      case 0:  // k_int
        return typeName == TypeNames.Integer;

      case 1:  // double
        return typeName == TypeNames.Double;

      case 2:  // bool
        return typeName == TypeNames.Boolean;

      case 3:  // k_string
        return typeName == TypeNames.String;

      case 4:  // k_list
        return typeName == TypeNames.List;

      case 5:  // k_hash
        return typeName == TypeNames.Hash;

      case 6:  // k_object
        return typeName == TypeNames.Object;

      case 7:  // k_lambda
        return typeName == TypeNames.Lambda;

      default:
        return false;
    }
  }

  static k_value executeReplace(const Token& term, const k_value& value,
                                const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Replace);
    }

    return String::replace(get_string(term, value),
                           get_string(term, args.at(0)),
                           get_string(term, args.at(1)));
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