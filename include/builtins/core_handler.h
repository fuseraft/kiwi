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

      case KName::Builtin_Astral_Members:
        return executeMembers(term, value, args);

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

      case KName::Builtin_Astral_Push:
        return executePush(term, value, args);

      case KName::Builtin_Astral_Pop:
        return executePop(term, value, args);

      case KName::Builtin_Astral_Enqueue:
        return executeEnqueue(term, value, args);

      case KName::Builtin_Astral_Dequeue:
        return executeDequeue(term, value, args);

      case KName::Builtin_Astral_Shift:
        return executeShift(term, value, args);

      case KName::Builtin_Astral_Unshift:
        return executeUnshift(term, value, args);

      case KName::Builtin_Astral_Clear:
        return executeClear(term, value, args);

      case KName::Builtin_Astral_Substring:
        return executeSubstring(term, value, args);

      case KName::Builtin_Astral_Remove:
        return executeRemove(term, value, args);

      case KName::Builtin_Astral_RemoveAt:
        return executeRemoveAt(term, value, args);

      case KName::Builtin_Astral_Rotate:
        return executeRotate(term, value, args);

      case KName::Builtin_Astral_Insert:
        return executeInsert(term, value, args);

      case KName::Builtin_Astral_Slice:
        return executeSlice(term, value, args);

      case KName::Builtin_Astral_Concat:
        return executeConcat(term, value, args);

      case KName::Builtin_Astral_Unique:
        return executeUnique(term, value, args);

      case KName::Builtin_Astral_Count:
        return executeCount(term, value, args);

      case KName::Builtin_Astral_Flatten:
        return executeFlatten(term, value, args);

      case KName::Builtin_Astral_Zip:
        return executeZip(term, value, args);

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

  static k_value executeMembers(const Token& term, const k_value& value,
                                const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Members);
    }

    if (!std::holds_alternative<k_object>(value)) {
      throw InvalidOperationError(term, "Expected an `Object` in call to `" +
                                            AstralBuiltins.Members + "`");
    }

    auto memberHash = std::make_shared<Hash>();
    auto obj = std::get<k_object>(value);
    auto& instanceVariables = obj->instanceVariables;
    auto clazz = classes[obj->className];

    for (const auto& method : clazz.getMethods()) {
      memberHash->add(method.first, {});
    }

    for (const auto& instanceVar : instanceVariables) {
      memberHash->add(instanceVar.first, instanceVar.second);
    }

    return memberHash;
  }

  static k_value executeJoin(const Token& term, const k_value& value,
                             const std::vector<k_value>& args) {
    int argSize = args.size();

    if (argSize != 0 && argSize != 1) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Join);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(term, "Cannot join a non-list type.");
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

    throw InvalidOperationError(
        term, "Invalid type for builtin `" + AstralBuiltins.Empty + "`.");
  }

  static k_value executeToHex(const Token& term, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.ToHex);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          term, "Expected a `List` value for byte to string conversion.");
    }

    auto& elements = std::get<k_list>(value)->elements;

    if (elements.empty()) {
      throw EmptyListError(term);
    }

    std::stringstream ss;

    for (const auto& item : elements) {
      if (!std::holds_alternative<k_int>(item)) {
        throw InvalidOperationError(
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
          throw InvalidOperationError(
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
      throw InvalidOperationError(
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
      k_int intValue = 0;
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

  static k_value executeSubstring(const Token& term, const k_value& value,
                                  const std::vector<k_value>& args) {
    if (args.size() != 1 && args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Substring);
    }

    if (!std::holds_alternative<k_string>(value)) {
      throw InvalidOperationError(term,
                                  "Expected a `String` value for builtin `" +
                                      AstralBuiltins.Substring + "`.");
    }

    auto stringValue = get_string(term, value);
    auto pos = static_cast<size_t>(get_integer(term, args.at(0)));
    auto size = stringValue.size();

    if (args.size() == 2) {
      size = static_cast<size_t>(get_integer(term, args.at(1)));
    }

    return String::substring(stringValue, pos, size);
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

    throw InvalidOperationError(term, "Expected a `String` or `List` value.");
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

    throw InvalidOperationError(
        term, "Expected a `String` or a `List` for builtin `" +
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

    throw InvalidOperationError(
        term, "Expected a `String` or a `List` for builtin `" +
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

    throw InvalidOperationError(
        term, "Expected a `String` or a `List` for builtin `" +
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

    throw InvalidOperationError(
        term, "Invalid type for builtin `" + AstralBuiltins.Empty + "`.");
  }

  static k_value executePush(const Token& term, const k_value& value,
                             const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Push);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          term, "Expected a `List` for builtin `" + AstralBuiltins.Push + "`.");
    }

    std::get<k_list>(value)->elements.push_back(args.at(0));
    return true;
  }

  static k_value executePop(const Token& term, const k_value& value,
                            const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Pop);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          term, "Expected a `List` for builtin `" + AstralBuiltins.Pop + "`.");
    }

    auto& elements = std::get<k_list>(value)->elements;

    if (elements.empty()) {
      return static_cast<k_int>(0);
    }

    auto _value = elements.back();
    elements.pop_back();
    return _value;
  }

  static k_value executeEnqueue(const Token& term, const k_value& value,
                                const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Enqueue);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(term, "Expected a `List` for builtin `" +
                                            AstralBuiltins.Enqueue + "`.");
    }

    std::get<k_list>(value)->elements.push_back(args.at(0));
    return true;
  }

  static k_value executeDequeue(const Token& term, const k_value& value,
                                const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Dequeue);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(term, "Expected a `List` for builtin `" +
                                            AstralBuiltins.Dequeue + "`.");
    }

    auto& elements = std::get<k_list>(value)->elements;

    if (elements.empty()) {
      return static_cast<k_int>(0);
    }

    auto _value = elements.front();
    elements.erase(elements.begin());
    return _value;
  }

  static k_value executeShift(const Token& term, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Shift);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(term, "Expected a `List` for builtin `" +
                                            AstralBuiltins.Shift + "`.");
    }

    auto& elements = std::get<k_list>(value)->elements;

    if (elements.empty()) {
      return static_cast<k_int>(0);
    }

    auto _value = elements.front();
    elements.erase(elements.begin());
    return _value;
  }

  static k_value executeUnshift(const Token& term, const k_value& value,
                                const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Unshift);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(term, "Expected a `List` for builtin `" +
                                            AstralBuiltins.Unshift + "`.");
    }

    auto& elements = std::get<k_list>(value)->elements;
    elements.insert(elements.begin(), args.at(0));
    return value;
  }

  static k_value executeConcat(const Token& term, const k_value& value,
                               const std::vector<k_value>& args) {
    if (args.size() != 1 || !std::holds_alternative<k_list>(args.at(0))) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Concat);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(term, "Expected a `List` for builtin `" +
                                            AstralBuiltins.Concat + "`.");
    }

    auto& elements = std::get<k_list>(value)->elements;
    const auto& concat = std::get<k_list>(args.at(0))->elements;
    elements.insert(elements.end(), concat.begin(), concat.end());
    return value;
  }

  static k_value executeInsert(const Token& term, const k_value& value,
                               const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Insert);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(term, "Expected a `List` for builtin `" +
                                            AstralBuiltins.Insert + "`.");
    }

    auto& elements = std::get<k_list>(value)->elements;
    size_t index = get_integer(term, args.at(1));

    if (index > elements.size()) {
      throw InvalidOperationError(
          term, "Index out of bounds for `insert` operation.");
    }

    elements.insert(elements.begin() + index, args.at(0));
    return value;
  }

  static k_value executeRemove(const Token& term, const k_value& value,
                               const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Remove);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(term, "Expected a `List` for builtin `" +
                                            AstralBuiltins.Remove + "`.");
    }

    auto& elements = std::get<k_list>(value)->elements;
    auto it = std::find(elements.begin(), elements.end(), args.at(0));

    if (it != elements.end()) {
      elements.erase(it);
    }

    return value;
  }

  static k_value executeRemoveAt(const Token& term, const k_value& value,
                                 const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.RemoveAt);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(term, "Expected a `List` for builtin `" +
                                            AstralBuiltins.RemoveAt + "`.");
    }

    auto& elements = std::get<k_list>(value)->elements;
    size_t index = get_integer(term, args.at(0));

    if (index >= elements.size()) {
      throw InvalidOperationError(
          term, "Index out of bounds for `removeAt` operation.");
    }

    elements.erase(elements.begin() + index);
    return value;
  }

  static k_value executeRotate(const Token& term, const k_value& value,
                               const std::vector<k_value>& args) {
    if (args.size() != 1 || !std::holds_alternative<k_int>(args[0])) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Rotate);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(term, "Expected a `List` for builtin `" +
                                            AstralBuiltins.Rotate + "`.");
    }

    auto& elements = std::get<k_list>(value)->elements;
    auto rotation = get_integer(term, args[0]);

    if (elements.empty()) {
      throw EmptyListError(term, "Cannot rotate an empty list.");
    }

    // Normalize the rotation
    rotation %= static_cast<int>(elements.size());
    if (rotation < 0) {
      rotation +=
          elements
              .size();  // Convert negative rotation to equivalent positive rotation
    }

    // Calculate the equivalent left rotation since std::rotate performs left rotation
    rotation = elements.size() - rotation;

    // Perform the rotation
    std::rotate(elements.begin(), elements.begin() + rotation, elements.end());
    return value;
  }

  static k_value executeUnique(const Token& term, const k_value& value,
                               const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Unique);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(term, "Expected a `List` for builtin `" +
                                            AstralBuiltins.Unique + "`.");
    }

    auto& elements = std::get<k_list>(value)->elements;
    std::unordered_set<k_value> seen;
    auto newEnd = std::remove_if(
        elements.begin(), elements.end(),
        [&seen](const k_value& item) { return !seen.insert(item).second; });
    elements.erase(newEnd, elements.end());
    return value;
  }

  static k_value executeCount(const Token& term, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Count);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(term, "Expected a `List` for builtin `" +
                                            AstralBuiltins.Count + "`.");
    }

    const auto& elements = std::get<k_list>(value)->elements;
    return std::count(elements.begin(), elements.end(), args.at(0));
  }

  static k_value executeFlatten(const Token& term, const k_value& value,
                                const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Flatten);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(term, "Expected a `List` for builtin `" +
                                            AstralBuiltins.Flatten + "`.");
    }

    auto flattened = std::make_shared<List>();
    std::function<void(const k_value&)> flattenElement;
    flattenElement = [&flattened, &flattenElement](const k_value& element) {
      if (std::holds_alternative<k_list>(element)) {
        for (const auto& subElement : std::get<k_list>(element)->elements) {
          flattenElement(subElement);
        }
      } else {
        flattened->elements.push_back(element);
      }
    };

    const auto& elements = std::get<k_list>(value)->elements;
    for (const auto& element : elements) {
      flattenElement(element);
    }
    return flattened;
  }

  static k_value executeZip(const Token& term, const k_value& value,
                            const std::vector<k_value>& args) {
    if (args.size() != 1 || !std::holds_alternative<k_list>(args.at(0))) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Zip);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          term, "Expected a `List` for builtin `" + AstralBuiltins.Zip + "`.");
    }

    const auto& elements1 = std::get<k_list>(value)->elements;
    const auto& elements2 = std::get<k_list>(args.at(0))->elements;
    auto zipped = std::make_shared<List>();

    for (size_t i = 0; i < std::min(elements1.size(), elements2.size()); ++i) {
      auto pair = std::make_shared<List>();
      pair->elements.push_back(elements1.at(i));
      pair->elements.push_back(elements2.at(i));
      zipped->elements.push_back(pair);
    }

    return zipped;
  }

  static k_value executeSlice(const Token& term, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Slice);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(term, "Expected a `List` for builtin `" +
                                            AstralBuiltins.Slice + "`.");
    }

    auto& elements = std::get<k_list>(value)->elements;
    auto start = static_cast<size_t>(get_integer(term, args.at(0)));
    auto end = static_cast<size_t>(get_integer(term, args.at(1)));

    if (start > end || end > elements.size()) {
      throw InvalidOperationError(
          term, "Invalid start or end index for `slice` operation.");
    }

    auto slicedList = std::make_shared<List>();
    auto& slice = slicedList->elements;
    slice.insert(slice.begin(), elements.begin() + start,
                 elements.begin() + end);
    return slicedList;
  }

  static k_value executeClear(const Token& term, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, AstralBuiltins.Clear);
    }

    if (std::holds_alternative<k_list>(value)) {
      std::get<k_list>(value)->elements.clear();
      return value;
    } else if (std::holds_alternative<k_hash>(value)) {
      auto hash = std::get<k_hash>(value);
      hash->keys.clear();
      hash->kvp.clear();
      return hash;
    }

    throw InvalidOperationError(
        term, "Invalid type for builtin `" + AstralBuiltins.Empty + "`.");
  }
};

#endif