#ifndef KIWI_BUILTINS_COREHANDLER_H
#define KIWI_BUILTINS_COREHANDLER_H

#include <algorithm>
#include <bitset>
#include <charconv>
#include <stdexcept>
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
  static k_value execute(const Token& token, const KName& builtin,
                         const k_value& value,
                         const std::vector<k_value>& args) {
    if (KiwiBuiltins.is_builtin(builtin)) {
      return executeKiwiBuiltin(token, builtin, value, args);
    }

    throw UnknownBuiltinError(token, "");
  }

 private:
  static k_value executeKiwiBuiltin(const Token& token, const KName& builtin,
                                    const k_value& value,
                                    const std::vector<k_value>& args) {
    switch (builtin) {
      case KName::Builtin_Kiwi_Chars:
        return executeChars(token, value, args);

        // case KName::Builtin_Kiwi_Members:
        //   return executeMembers(token, value, args);

      case KName::Builtin_Kiwi_IsA:
        return executeIsA(token, value, args);

      case KName::Builtin_Kiwi_Join:
        return executeJoin(token, value, args);

      case KName::Builtin_Kiwi_Split:
        return executeSplit(token, value, args);

      case KName::Builtin_Kiwi_RSplit:
        return executeRSplit(token, value, args);

      case KName::Builtin_Kiwi_Get:
        return executeGet(token, value, args);

      case KName::Builtin_Kiwi_Set:
        return executeSet(token, value, args);

      case KName::Builtin_Kiwi_First:
        return executeFirst(token, value, args);

      case KName::Builtin_Kiwi_Last:
        return executeLast(token, value, args);

      case KName::Builtin_Kiwi_LeftTrim:
        return executeLeftTrim(token, value, args);

      case KName::Builtin_Kiwi_RightTrim:
        return executeRightTrim(token, value, args);

      case KName::Builtin_Kiwi_Trim:
        return executeTrim(token, value, args);

      case KName::Builtin_Kiwi_Type:
        return executeType(token, value, args);

      case KName::Builtin_Kiwi_Size:
        return executeSize(token, value, args);

      case KName::Builtin_Kiwi_ToBytes:
        return executeToBytes(token, value, args);

      case KName::Builtin_Kiwi_ToHex:
        return executeToHex(token, value, args);

      case KName::Builtin_Kiwi_ToD:
        return executeToDouble(token, value, args);

      case KName::Builtin_Kiwi_ToI:
        return executeToInteger(token, value, args);

      case KName::Builtin_Kiwi_ToS:
        return executeToString(token, value, args);

      case KName::Builtin_Kiwi_BeginsWith:
        return executeBeginsWith(token, value, args);

      case KName::Builtin_Kiwi_Contains:
        return executeContains(token, value, args);

      case KName::Builtin_Kiwi_EndsWith:
        return executeEndsWith(token, value, args);

      case KName::Builtin_Kiwi_Replace:
        return executeReplace(token, value, args);

      case KName::Builtin_Kiwi_RReplace:
        return executeRReplace(token, value, args);

      case KName::Builtin_Kiwi_Reverse:
        return executeReverse(token, value, args);

      case KName::Builtin_Kiwi_IndexOf:
        return executeIndexOf(token, value, args);

      case KName::Builtin_Kiwi_LastIndexOf:
        return executeLastIndexOf(token, value, args);

      case KName::Builtin_Kiwi_Uppercase:
        return executeUppercase(token, value, args);

      case KName::Builtin_Kiwi_Lowercase:
        return executeLowercase(token, value, args);

      case KName::Builtin_Kiwi_Empty:
        return executeEmpty(token, value, args);

      case KName::Builtin_Kiwi_Keys:
        return executeKeys(token, value, args);

      case KName::Builtin_Kiwi_HasKey:
        return executeHasKey(token, value, args);

      case KName::Builtin_Kiwi_Merge:
        return executeMerge(token, value, args);

      case KName::Builtin_Kiwi_Values:
        return executeValues(token, value, args);

      case KName::Builtin_Kiwi_Push:
        return executePush(token, value, args);

      case KName::Builtin_Kiwi_Pop:
        return executePop(token, value, args);

      case KName::Builtin_Kiwi_Enqueue:
        return executeEnqueue(token, value, args);

      case KName::Builtin_Kiwi_Dequeue:
        return executeDequeue(token, value, args);

      case KName::Builtin_Kiwi_Shift:
        return executeShift(token, value, args);

      case KName::Builtin_Kiwi_Unshift:
        return executeUnshift(token, value, args);

      case KName::Builtin_Kiwi_Clear:
        return executeClear(token, value, args);

      case KName::Builtin_Kiwi_Substring:
        return executeSubstring(token, value, args);

      case KName::Builtin_Kiwi_Remove:
        return executeRemove(token, value, args);

      case KName::Builtin_Kiwi_RemoveAt:
        return executeRemoveAt(token, value, args);

      case KName::Builtin_Kiwi_Rotate:
        return executeRotate(token, value, args);

      case KName::Builtin_Kiwi_Insert:
        return executeInsert(token, value, args);

      case KName::Builtin_Kiwi_Slice:
        return executeSlice(token, value, args);

      case KName::Builtin_Kiwi_Swap:
        return executeSwap(token, value, args);

      case KName::Builtin_Kiwi_Concat:
        return executeConcat(token, value, args);

      case KName::Builtin_Kiwi_Unique:
        return executeUnique(token, value, args);

      case KName::Builtin_Kiwi_Count:
        return executeCount(token, value, args);

      case KName::Builtin_Kiwi_Flatten:
        return executeFlatten(token, value, args);

      case KName::Builtin_Kiwi_Zip:
        return executeZip(token, value, args);

      case KName::Builtin_Kiwi_Clone:
        return executeClone(token, value, args);

      case KName::Builtin_Kiwi_Pretty:
        return executePretty(token, value, args);

      case KName::Builtin_Kiwi_Find:
        return executeFind(token, value, args);

      case KName::Builtin_Kiwi_Match:
        return executeMatch(token, value, args);

      case KName::Builtin_Kiwi_Matches:
        return executeMatches(token, value, args);

      case KName::Builtin_Kiwi_MatchesAll:
        return executeMatchesAll(token, value, args);

      case KName::Builtin_Kiwi_Scan:
        return executeScan(token, value, args);

      case KName::Builtin_Kiwi_Truthy:
        return executeTruthy(token, value, args);

      case KName::Builtin_Kiwi_Lines:
        return executeLines(token, value, args);

      case KName::Builtin_Kiwi_Tokens:
        return executeTokens(token, value, args);

      default:
        break;
    }

    throw UnknownBuiltinError(token, "");
  }

  static k_value executeLines(const Token& token, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Lines);
    }

    auto s = get_string(token, value);
    return String::lines(s);
  }

  static k_value executeTokens(const Token& token, const k_value& value,
                               const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Tokens);
    }

    auto s = get_string(token, value);
    std::vector<k_value> tokens;

    Lexer lex(token.getFile(), s);

    auto ts = lex.getAllTokens();
    tokens.reserve(ts.size());

    for (const auto& token : ts) {
      tokens.emplace_back(token.getText());
    }

    return std::make_shared<List>(tokens);
  }

  static k_value executeTruthy(const Token& token, const k_value& value,
                               const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Truthy);
    }

    switch (value.index()) {
      case KValueType::INTEGER:
        return std::get<k_int>(value) != static_cast<k_int>(0);

      case KValueType::FLOAT:
        return std::get<double>(value) != static_cast<double>(0);

      case KValueType::BOOLEAN:
        return std::get<bool>(value);

      case KValueType::STRING:
        return !std::get<k_string>(value).empty();

      case KValueType::LIST:
        return !std::get<k_list>(value)->elements.empty();

      case KValueType::HASHMAP:
        return std::get<k_hashmap>(value)->size() > 0;

      case KValueType::OBJECT:
        return true;

      case KValueType::LAMBDA:
        return true;

      case KValueType::NONE:
        return false;

      default:
        return false;
    }
  }

  static k_value executeGet(const Token& token, const k_value& value,
                            const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Get);
    }

    if (std::holds_alternative<k_hashmap>(value)) {
      const auto& hash = std::get<k_hashmap>(value);
      const auto& key = args.at(0);
      if (!hash->hasKey(key)) {
        throw HashKeyError(token, Serializer::serialize(key));
      }

      return hash->get(key);
    } else if (std::holds_alternative<k_list>(value)) {
      auto index = get_integer(token, args.at(0));
      auto elements = std::get<k_list>(value)->elements;
      if (index < 0 || index >= static_cast<k_int>(elements.size())) {
        throw RangeError(token, "List index out of range.");
      }
      return elements.at(index);
    } else if (std::holds_alternative<k_string>(value)) {
      auto index = get_integer(token, args.at(0));
      auto str = get_string(token, value);
      if (index < 0 || index >= static_cast<k_int>(str.size())) {
        throw RangeError(token, "List index out of range.");
      }
      return k_string(1, str.at(index));
    }

    throw InvalidOperationError(
        token, "Expected a hashmap, list, or string in call to `" +
                   KiwiBuiltins.Get + "`");
  }

  static k_value executeSet(const Token& token, const k_value& value,
                            const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Set);
    }

    if (std::holds_alternative<k_hashmap>(value)) {
      auto& hash = std::get<k_hashmap>(value);
      hash->add(args.at(0), args.at(1));
      return hash;
    } else if (std::holds_alternative<k_list>(value)) {
      const auto& index = get_integer(token, args.at(0));
      auto& elements = std::get<k_list>(value)->elements;
      if (index < 0 || index >= static_cast<k_int>(elements.size())) {
        throw RangeError(token, "List index out of range.");
      }
      elements[index] = args.at(1);
      return value;
    }

    throw InvalidOperationError(
        token,
        "Expected a hashmap or list in call to `" + KiwiBuiltins.Set + "`");
  }

  static k_value executeSwap(const Token& token, const k_value& value,
                             const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Swap);
    }

    if (std::holds_alternative<k_list>(value)) {
      auto firstIndex = get_integer(token, args.at(0));
      auto secondIndex = get_integer(token, args.at(1));
      auto& elements = std::get<k_list>(value)->elements;
      if (firstIndex < 0 || firstIndex >= static_cast<k_int>(elements.size())) {
        throw RangeError(token, "The first parameter for " + KiwiBuiltins.Swap +
                                    " is out of range.");
      }
      if (secondIndex < 0 ||
          secondIndex >= static_cast<k_int>(elements.size())) {
        throw RangeError(token, "The second parameter for " +
                                    KiwiBuiltins.Swap + " is out of range.");
      }

      auto firstValue = elements[firstIndex];
      auto secondValue = elements[secondIndex];
      elements[firstIndex] = secondValue;
      elements[secondIndex] = firstValue;
      return value;
    }

    throw InvalidOperationError(
        token,
        "Expected a hashmap or list in call to `" + KiwiBuiltins.Set + "`");
  }

  static k_value executeFirst(const Token& token, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() > 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.First);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          token, "Expected a list in call to `" + KiwiBuiltins.First + "`");
    }

    auto list = std::get<k_list>(value)->elements;
    if (list.empty()) {
      if (args.size() == 1) {
        return args.at(0);
      }
      return std::make_shared<Null>();
    }

    return list.front();
  }

  static k_value executeLast(const Token& token, const k_value& value,
                             const std::vector<k_value>& args) {
    if (args.size() > 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Last);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          token, "Expected a list in call to `" + KiwiBuiltins.Last + "`");
    }

    auto list = std::get<k_list>(value)->elements;
    if (list.empty()) {
      if (args.size() == 1) {
        return args.at(0);
      }
      return std::make_shared<Null>();
    }

    return list.back();
  }

  static k_value executeChars(const Token& token, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Chars);
    }

    auto newList = std::make_shared<List>();
    auto stringValue = get_string(token, value);
    auto& elements = newList->elements;

    elements.reserve(stringValue.size());
    k_string temp(1, '\0');
    for (const char& c : stringValue) {
      temp[0] = c;
      elements.emplace_back(temp);
    }

    return newList;
  }

  static k_value executeClone(const Token& token, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Clone);
    }

    return clone_value(value);
  }

  static k_value executePretty(const Token& token, const k_value& value,
                               const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Pretty);
    }

    return Serializer::pretty_serialize(value);
  }

  static k_value executeJoin(const Token& token, const k_value& value,
                             const std::vector<k_value>& args) {
    int argSize = args.size();

    if (argSize != 0 && argSize != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Join);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(token, "Cannot join a non-list type.");
    }

    auto list = std::get<k_list>(value);
    auto& elements = list->elements;
    std::ostringstream sv;
    k_string joiner;

    if (argSize == 1) {
      joiner = get_string(token, args.at(0));
    }

    for (auto it = elements.begin(); it != elements.end(); ++it) {
      if (it != elements.begin()) {
        sv << joiner;
      }
      sv << Serializer::serialize(*it);
    }

    return sv.str();
  }

  static k_value executeSize(const Token& token, const k_value& value,
                             const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Size);
    }

    if (std::holds_alternative<k_string>(value)) {
      return static_cast<k_int>(std::get<k_string>(value).length());
    } else if (std::holds_alternative<k_list>(value)) {
      return static_cast<k_int>(std::get<k_list>(value)->elements.size());
    } else if (std::holds_alternative<k_hashmap>(value)) {
      return static_cast<k_int>(std::get<k_hashmap>(value)->size());
    }

    throw InvalidOperationError(
        token, "Invalid type for builtin `" + KiwiBuiltins.Size + "`.");
  }

  static k_value executeToHex(const Token& token, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() > 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.ToHex);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          token, "Expected a list value for byte to string conversion.");
    }

    auto& elements = std::get<k_list>(value)->elements;

    if (elements.empty()) {
      throw EmptyListError(token);
    }

    std::stringstream ss;
    int width = 2;

    if (!args.empty()) {
      if (!std::holds_alternative<k_int>(args[0])) {
        throw ConversionError(token,
                              "Expected an integer as the width argument.");
      }

      width = std::get<k_int>(args[0]);
      if (width < 2) {
        throw InvalidOperationError(token, "Width must be >= 2.");
      }
    }

    for (const auto& item : elements) {
      if (!std::holds_alternative<k_int>(item)) {
        throw InvalidOperationError(
            token, "Expected an integer value for byte to string conversion.");
      }

      auto byte = std::get<k_int>(item);
      ss << std::hex << std::setw(width) << std::setfill('0') << byte;
    }

    return ss.str();
  }

  static k_value executeToBytes(const Token& token, const k_value& value,
                                const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.ToBytes);
    }

    if (std::holds_alternative<k_string>(value)) {
      auto stringValue = std::get<k_string>(value);
      std::vector<uint8_t> bytes(stringValue.begin(), stringValue.end());
      auto byteList = std::make_shared<List>();
      auto& elements = byteList->elements;
      elements.reserve(bytes.size());

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
              token, "Expected a list to contain only string values.");
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
          token, "Expected a string or list to convert to bytes.");
    }
  }

  static k_value executeToDouble(const Token& token, const k_value& value,
                                 const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.ToD);
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
            token, "Cannot convert non-numeric value to a double: `" +
                       stringValue + "`");
      }
    } else if (std::holds_alternative<k_int>(value)) {
      return static_cast<double>(std::get<k_int>(value));
    } else {
      throw ConversionError(token,
                            "Cannot convert non-numeric value to a double.");
    }
  }

  static k_value executeToInteger(const Token& token, const k_value& value,
                                  const std::vector<k_value>& args) {
    if (args.size() > 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.ToI);
    }

    int base = 10;

    if (!args.empty()) {
      if (!std::holds_alternative<k_int>(args[0])) {
        throw ConversionError(token,
                              "Expected an integer as the base argument.");
      }

      base = std::get<k_int>(args[0]);
      if (base < 2 || base > 36) {
        throw InvalidOperationError(
            token, "Base must be between 2 and 36, inclusive.");
      }
    }

    if (std::holds_alternative<k_string>(value)) {
      k_string stringValue = std::get<k_string>(value);
      k_int intValue = 0;
      auto [ptr, ec] = std::from_chars(stringValue.data(),
                                       stringValue.data() + stringValue.size(),
                                       intValue, base);

      if (ec == std::errc()) {
        return static_cast<k_int>(intValue);
      } else {
        throw ConversionError(
            token, "Cannot convert non-numeric value to an integer: `" +
                       stringValue + "`");
      }
    } else if (std::holds_alternative<double>(value)) {
      return static_cast<k_int>(std::get<double>(value));
    } else if (std::holds_alternative<k_int>(value)) {
      return std::get<k_int>(value);
    } else {
      throw ConversionError(token,
                            "Cannot convert non-numeric value to an integer.");
    }
  }

  static k_value executeToString(const Token& token, const k_value& value,
                                 const std::vector<k_value>& args) {
    if (args.size() != 0 && args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.ToS);
    }

    if (args.empty()) {
      return Serializer::serialize(value);
    }

    auto format = get_string(token, args.at(0));

    if (String::trim(format).empty()) {
      return Serializer::serialize(value);
    }

    if (!std::holds_alternative<double>(value) &&
        !std::holds_alternative<k_int>(value)) {
      throw ArgumentError(
          token, "Expected an integer or double for numeric formatting.");
    }

    std::ostringstream sv;

    if (format == "b" || format == "B") {
      auto toBinary = get_integer(token, value);
      sv << std::bitset<16>(toBinary);
      return sv.str();
    } else if (format == "x" || format == "X") {
      sv << std::hex << get_integer(token, value);
      if (format == "x") {
        return sv.str();
      }
      return String::toUppercase(sv.str());
    } else if (format == "o" || format == "O") {
      sv << std::oct << get_integer(token, value);
      if (format == "o") {
        return sv.str();
      }
      return String::toUppercase(sv.str());
    } else if (String::beginsWith(String::toLowercase(format), "f") ||
               String::toLowercase(format) == "f") {
      // Fixed point
      try {
        auto precision = String::replace(String::toLowercase(format), "f", "");
        if (precision.empty()) {
          sv << std::fixed << std::setprecision(0) << get_double(token, value);
        } else {
          sv << std::fixed << std::setprecision(std::stoi(precision))
             << get_double(token, value);
        }
        return sv.str();
      } catch (const std::exception& e) {
        throw ArgumentError(token,
                            "Invalid fixed-point format `" + format + "`");
      }
    }

    throw ArgumentError(token, "Unknown format specifier `" + format + "`");
  }

  static k_value executeSubstring(const Token& token, const k_value& value,
                                  const std::vector<k_value>& args) {
    if (args.size() != 1 && args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Substring);
    }

    if (!std::holds_alternative<k_string>(value)) {
      throw InvalidOperationError(token,
                                  "Expected a string value for builtin `" +
                                      KiwiBuiltins.Substring + "`.");
    }

    auto stringValue = get_string(token, value);
    auto pos = static_cast<size_t>(get_integer(token, args.at(0)));
    auto size = stringValue.size();

    if (args.size() == 2) {
      size = static_cast<size_t>(get_integer(token, args.at(1)));
    }

    return String::substring(stringValue, pos, size);
  }

  static k_value executeFind(const Token& token, const k_value& value,
                             const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Find);
    }

    auto stringValue = get_string(token, value);
    auto pattern = get_string(token, args.at(0));

    return String::find(stringValue, pattern);
  }

  static k_value executeMatch(const Token& token, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Match);
    }

    auto stringValue = get_string(token, value);
    auto pattern = get_string(token, args.at(0));

    return String::match(stringValue, pattern);
  }

  static k_value executeMatches(const Token& token, const k_value& value,
                                const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Matches);
    }

    auto stringValue = get_string(token, value);
    auto pattern = get_string(token, args.at(0));

    return String::matches(stringValue, pattern);
  }

  static k_value executeMatchesAll(const Token& token, const k_value& value,
                                   const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.MatchesAll);
    }

    auto stringValue = get_string(token, value);
    auto pattern = get_string(token, args.at(0));

    return String::matchesAll(stringValue, pattern);
  }

  static k_value executeScan(const Token& token, const k_value& value,
                             const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Scan);
    }

    auto stringValue = get_string(token, value);
    auto pattern = get_string(token, args.at(0));

    return String::scan(stringValue, pattern);
  }

  static k_value executeSplit(const Token& token, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Split);
    }

    k_string input = get_string(token, value);
    auto delimiter = get_string(token, args.at(0));
    auto newList = std::make_shared<List>();
    auto& elements = newList->elements;

    if (delimiter.empty()) {
      elements.reserve(input.size());
      k_string temp(1, '\0');
      for (char c : input) {
        temp[0] = c;
        elements.emplace_back(temp);
      }
    } else {
      const auto& tokens = String::split(input, delimiter);
      elements.reserve(tokens.size());
      for (const auto& token : tokens) {
        elements.emplace_back(token);
      }
    }

    return newList;
  }

  static k_value executeRSplit(const Token& token, const k_value& value,
                               const std::vector<k_value>& args) {
    if (args.size() != 1 && args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.RSplit);
    }

    k_string input = get_string(token, value);
    auto delimiter = get_string(token, args.at(0));
    auto newList = std::make_shared<List>();
    auto& elements = newList->elements;
    k_int limit = -1;

    if (args.size() == 2) {
      limit = get_integer(token, args.at(1));
    }

    if (delimiter.empty()) {
      elements.reserve(input.size());
      k_string temp(1, '\0');
      for (char c : input) {
        temp[0] = c;
        elements.emplace_back(temp);
      }
    } else {
      const auto& tokens = String::rsplit(input, delimiter, limit);
      elements.reserve(tokens.size());
      for (const auto& token : tokens) {
        elements.emplace_back(token);
      }
    }

    return newList;
  }

  static k_value executeLeftTrim(const Token& token, const k_value& value,
                                 const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.LeftTrim);
    }

    return String::trimLeft(get_string(token, value));
  }

  static k_value executeRightTrim(const Token& token, const k_value& value,
                                  const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.RightTrim);
    }

    return String::trimRight(get_string(token, value));
  }

  static k_value executeTrim(const Token& token, const k_value& value,
                             const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Trim);
    }

    return String::trim(get_string(token, value));
  }

  static k_value executeType(const Token& token, const k_value& value,
                             const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Type);
    }

    return Serializer::get_value_type_string(value);
  }

  static k_value executeHasKey(const Token& token, const k_value& value,
                               const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.HasKey);
    }

    if (!std::holds_alternative<k_hashmap>(value)) {
      throw InvalidOperationError(
          token, "Expected a hashmap for `" + KiwiBuiltins.HasKey + "`.");
    }

    return std::get<k_hashmap>(value)->hasKey(args.at(0));
  }

  static k_value executeKeys(const Token& token, const k_value& value,
                             const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Keys);
    }

    if (!std::holds_alternative<k_hashmap>(value)) {
      throw InvalidOperationError(
          token, "Expected a hashmap `" + KiwiBuiltins.Keys + "`.");
    }

    return Serializer::get_hash_keys_list(std::get<k_hashmap>(value));
  }

  static k_value executeValues(const Token& token, const k_value& value,
                               const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Values);
    }

    if (!std::holds_alternative<k_hashmap>(value)) {
      throw InvalidOperationError(
          token, "Expected a hashmap for `" + KiwiBuiltins.Values + "`.");
    }

    return Serializer::get_hash_values_list(std::get<k_hashmap>(value));
  }

  static k_value executeMerge(const Token& token, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Merge);
    }

    if (!std::holds_alternative<k_hashmap>(value) ||
        !std::holds_alternative<k_hashmap>(args.at(0))) {
      throw InvalidOperationError(
          token, "Expected a hashmap for `" + KiwiBuiltins.Merge + "`.");
    }

    auto hashValue = std::get<k_hashmap>(value);
    auto mergeHash = std::get<k_hashmap>(args.at(0));

    hashValue->merge(mergeHash);

    return hashValue;
  }

  static k_value executeBeginsWith(const Token& token, const k_value& value,
                                   const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.BeginsWith);
    }

    return String::beginsWith(get_string(token, value),
                              get_string(token, args.at(0)));
  }

  static k_value executeStringContains(const Token& token, const k_value& value,
                                       const k_value& arg) {
    return String::contains(get_string(token, value), get_string(token, arg));
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

  static k_value executeContains(const Token& token, const k_value& value,
                                 const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Contains);
    }

    if (std::holds_alternative<k_string>(value)) {
      return executeStringContains(token, value, args.at(0));
    } else if (std::holds_alternative<k_list>(value)) {
      return executeListContains(value, args.at(0));
    }

    throw InvalidOperationError(token, "Expected a string or list value.");
  }

  static k_value executeEndsWith(const Token& token, const k_value& value,
                                 const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Contains);
    }

    return String::endsWith(get_string(token, value),
                            get_string(token, args.at(0)));
  }

  static k_value executeIsA(const Token& token, const k_value& value,
                            const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.IsA);
    }

    auto typeName = get_string(token, args.at(0));
    // if (!TypeNames.is_typename(typeName)) {
    //   if (std::holds_alternative<k_object>(value)) {
    //     const auto& obj = std::get<k_object>(value);
    //     const auto& structName = obj->structName;

    //     return same_value(structName, typeName) ||
    //            same_value(structs.at(structName).getBaseClassName(), typeName);
    //   }
    //   throw InvalidTypeNameError(token, typeName);
    // }

    switch (value.index()) {
      case KValueType::INTEGER:
        return typeName == TypeNames.Integer;

      case KValueType::FLOAT:
        return typeName == TypeNames.Float;

      case KValueType::BOOLEAN:
        return typeName == TypeNames.Boolean;

      case KValueType::STRING:
        return typeName == TypeNames.String;

      case KValueType::LIST:
        return typeName == TypeNames.List;

      case KValueType::HASHMAP:
        return typeName == TypeNames.Hashmap;

      case KValueType::OBJECT:
        return typeName == TypeNames.Object;

      case KValueType::LAMBDA:
        return typeName == TypeNames.Lambda;

      case KValueType::NONE:
        return typeName == TypeNames.None;

      default:
        return false;
    }
  }

  static k_value executeReplace(const Token& token, const k_value& value,
                                const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Replace);
    }

    return String::replace(get_string(token, value),
                           get_string(token, args.at(0)),
                           get_string(token, args.at(1)));
  }

  static k_value executeRReplace(const Token& token, const k_value& value,
                                 const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.RReplace);
    }

    return String::rreplace(get_string(token, value),
                            get_string(token, args.at(0)),
                            get_string(token, args.at(1)));
  }

  static k_value executeReverse(const Token& token, const k_value& value,
                                const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Reverse);
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

    throw InvalidOperationError(token,
                                "Expected a string or a list for builtin `" +
                                    KiwiBuiltins.Reverse + "`.");
  }

  static k_value executeIndexOf(const Token& token, const k_value& value,
                                const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.IndexOf);
    }

    if (std::holds_alternative<k_string>(value)) {
      return static_cast<k_int>(String::indexOf(std::get<k_string>(value),
                                                get_string(token, args.at(0))));
    } else if (std::holds_alternative<k_list>(value)) {
      return indexof_listvalue(std::get<k_list>(value), args.at(0));
    }

    throw InvalidOperationError(token,
                                "Expected a string or a list for builtin `" +
                                    KiwiBuiltins.IndexOf + "`.");
  }

  static k_value executeLastIndexOf(const Token& token, const k_value& value,
                                    const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.LastIndexOf);
    }

    if (std::holds_alternative<k_string>(value)) {
      return static_cast<k_int>(String::lastIndexOf(
          std::get<k_string>(value), get_string(token, args.at(0))));
    } else if (std::holds_alternative<k_list>(value)) {
      return lastindexof_listvalue(std::get<k_list>(value), args.at(0));
    }

    throw InvalidOperationError(token,
                                "Expected a string or a list for builtin `" +
                                    KiwiBuiltins.LastIndexOf + "`.");
  }

  static k_value executeUppercase(const Token& token, const k_value& value,
                                  const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Uppercase);
    }

    return String::toUppercase(get_string(token, value));
  }

  static k_value executeLowercase(const Token& token, const k_value& value,
                                  const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Lowercase);
    }

    return String::toLowercase(get_string(token, value));
  }

  static k_value executeEmpty(const Token& token, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() > 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Empty);
    }

    bool isEmpty = false;
    if (std::holds_alternative<k_string>(value)) {
      isEmpty = std::get<k_string>(value).empty();
    } else if (std::holds_alternative<k_list>(value)) {
      isEmpty = std::get<k_list>(value)->elements.empty();
    } else if (std::holds_alternative<k_hashmap>(value)) {
      isEmpty = std::get<k_hashmap>(value)->keys.empty();
    } else if (std::holds_alternative<k_int>(value)) {
      isEmpty = std::get<k_int>(value) == 0;
    } else if (std::holds_alternative<double>(value)) {
      isEmpty = std::get<double>(value) == 0.0;
    } else if (std::holds_alternative<bool>(value)) {
      isEmpty = !std::get<bool>(value);
    } else if (std::holds_alternative<k_null>(value)) {
      isEmpty = true;
    } else {
      throw InvalidOperationError(
          token, "Invalid type for builtin `" + KiwiBuiltins.Empty + "`.");
    }

    // This is a workaround for null-coalescing.
    if (!args.empty()) {
      if (isEmpty) {
        return args.at(0);
      } else {
        return value;
      }
    }

    return isEmpty;
  }

  static k_value executePush(const Token& token, const k_value& value,
                             const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Push);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          token, "Expected a list for builtin `" + KiwiBuiltins.Push + "`.");
    }

    std::get<k_list>(value)->elements.push_back(args.at(0));
    return true;
  }

  static k_value executePop(const Token& token, const k_value& value,
                            const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Pop);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          token, "Expected a list for builtin `" + KiwiBuiltins.Pop + "`.");
    }

    auto& elements = std::get<k_list>(value)->elements;

    if (elements.empty()) {
      return static_cast<k_int>(0);
    }

    auto _value = elements.back();
    elements.pop_back();
    return _value;
  }

  static k_value executeEnqueue(const Token& token, const k_value& value,
                                const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Enqueue);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          token, "Expected a list for builtin `" + KiwiBuiltins.Enqueue + "`.");
    }

    std::get<k_list>(value)->elements.push_back(args.at(0));
    return true;
  }

  static k_value executeDequeue(const Token& token, const k_value& value,
                                const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Dequeue);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          token, "Expected a list for builtin `" + KiwiBuiltins.Dequeue + "`.");
    }

    auto& elements = std::get<k_list>(value)->elements;

    if (elements.empty()) {
      return static_cast<k_int>(0);
    }

    auto _value = elements.front();
    elements.erase(elements.begin());
    return _value;
  }

  static k_value executeShift(const Token& token, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Shift);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          token, "Expected a list for builtin `" + KiwiBuiltins.Shift + "`.");
    }

    auto& elements = std::get<k_list>(value)->elements;

    if (elements.empty()) {
      return static_cast<k_int>(0);
    }

    auto _value = elements.front();
    elements.erase(elements.begin());
    return _value;
  }

  static k_value executeUnshift(const Token& token, const k_value& value,
                                const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Unshift);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          token, "Expected a list for builtin `" + KiwiBuiltins.Unshift + "`.");
    }

    auto& elements = std::get<k_list>(value)->elements;
    elements.insert(elements.begin(), args.at(0));
    return value;
  }

  static k_value executeConcat(const Token& token, const k_value& value,
                               const std::vector<k_value>& args) {
    if (args.size() != 1 || !std::holds_alternative<k_list>(args.at(0))) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Concat);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          token, "Expected a list for builtin `" + KiwiBuiltins.Concat + "`.");
    }

    auto& elements = std::get<k_list>(value)->elements;
    const auto& concat = std::get<k_list>(args.at(0))->elements;
    elements.insert(elements.end(), concat.begin(), concat.end());
    return value;
  }

  static k_value executeInsert(const Token& token, const k_value& value,
                               const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Insert);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          token, "Expected a list for builtin `" + KiwiBuiltins.Insert + "`.");
    }

    auto& elements = std::get<k_list>(value)->elements;
    size_t index = get_integer(token, args.at(1));

    if (index > elements.size()) {
      throw InvalidOperationError(
          token, "Index out of bounds for `insert` operation.");
    }

    elements.insert(elements.begin() + index, args.at(0));
    return value;
  }

  static k_value executeRemove(const Token& token, const k_value& value,
                               const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Remove);
    }

    if (std::holds_alternative<k_hashmap>(value)) {
      auto hash = std::get<k_hashmap>(value);
      auto key = get_string(token, args.at(0));
      hash->remove(key);
      return hash;
    } else if (std::holds_alternative<k_list>(value)) {
      auto& elements = std::get<k_list>(value)->elements;
      auto it = std::find(elements.begin(), elements.end(), args.at(0));

      if (it != elements.end()) {
        elements.erase(it);
      }

      return value;
    }

    throw InvalidOperationError(token,
                                "Expected a hashmap or list for builtin `" +
                                    KiwiBuiltins.Remove + "`.");
  }

  static k_value executeRemoveAt(const Token& token, const k_value& value,
                                 const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.RemoveAt);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(token, "Expected a list for builtin `" +
                                             KiwiBuiltins.RemoveAt + "`.");
    }

    auto& elements = std::get<k_list>(value)->elements;
    size_t index = get_integer(token, args.at(0));

    if (index >= elements.size()) {
      throw InvalidOperationError(
          token, "Index out of bounds for `removeAt` operation.");
    }

    elements.erase(elements.begin() + index);
    return value;
  }

  static k_value executeRotate(const Token& token, const k_value& value,
                               const std::vector<k_value>& args) {
    if (args.size() != 1 || !std::holds_alternative<k_int>(args[0])) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Rotate);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          token, "Expected a list for builtin `" + KiwiBuiltins.Rotate + "`.");
    }

    auto& elements = std::get<k_list>(value)->elements;
    auto rotation = get_integer(token, args[0]);

    if (elements.empty()) {
      throw EmptyListError(token, "Cannot rotate an empty list.");
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

  static k_value executeUnique(const Token& token, const k_value& value,
                               const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Unique);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          token, "Expected a list for builtin `" + KiwiBuiltins.Unique + "`.");
    }

    auto& elements = std::get<k_list>(value)->elements;
    std::unordered_set<k_value> seen;
    auto newEnd = std::remove_if(
        elements.begin(), elements.end(),
        [&seen](const k_value& item) { return !seen.insert(item).second; });
    elements.erase(newEnd, elements.end());
    return value;
  }

  static k_value executeCount(const Token& token, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Count);
    }

    if (std::holds_alternative<k_string>(value)) {
      const auto& needle = get_string(token, args.at(0));
      const auto& haystack = std::get<k_string>(value);
      return String::count(haystack, needle);
    } else if (std::holds_alternative<k_list>(value)) {
      const auto& elements = std::get<k_list>(value)->elements;
      return static_cast<k_int>(
          std::count(elements.begin(), elements.end(), args.at(0)));
    }

    throw InvalidOperationError(
        token,
        "Expected a list or string for builtin `" + KiwiBuiltins.Count + "`.");
  }

  static k_value executeFlatten(const Token& token, const k_value& value,
                                const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Flatten);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          token, "Expected a list for builtin `" + KiwiBuiltins.Flatten + "`.");
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

  static k_value executeZip(const Token& token, const k_value& value,
                            const std::vector<k_value>& args) {
    if (args.size() != 1 || !std::holds_alternative<k_list>(args.at(0))) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Zip);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          token, "Expected a list for builtin `" + KiwiBuiltins.Zip + "`.");
    }

    const auto& elements1 = std::get<k_list>(value)->elements;
    const auto& elements2 = std::get<k_list>(args.at(0))->elements;
    auto zipped = std::make_shared<List>();
    auto win_min = (elements1.size() < elements2.size()) ? elements1.size()
                                                         : elements2.size();
    for (size_t i = 0; i < win_min; ++i) {
      auto pair = std::make_shared<List>();
      pair->elements.push_back(elements1.at(i));
      pair->elements.push_back(elements2.at(i));
      zipped->elements.push_back(pair);
    }

    return zipped;
  }

  static k_value executeSlice(const Token& token, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Slice);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          token, "Expected a list for builtin `" + KiwiBuiltins.Slice + "`.");
    }

    auto& elements = std::get<k_list>(value)->elements;
    auto start = static_cast<size_t>(get_integer(token, args.at(0)));
    auto end = static_cast<size_t>(get_integer(token, args.at(1)));

    if (start > end || end > elements.size()) {
      throw InvalidOperationError(
          token, "Invalid start or end index for `slice` operation.");
    }

    auto slicedList = std::make_shared<List>();
    auto& slice = slicedList->elements;
    slice.insert(slice.begin(), elements.begin() + start,
                 elements.begin() + end);
    return slicedList;
  }

  static k_value executeClear(const Token& token, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Clear);
    }

    if (std::holds_alternative<k_list>(value)) {
      std::get<k_list>(value)->elements.clear();
      return value;
    } else if (std::holds_alternative<k_hashmap>(value)) {
      auto hash = std::get<k_hashmap>(value);
      hash->keys.clear();
      hash->kvp.clear();
      return hash;
    }

    throw InvalidOperationError(
        token, "Invalid type for builtin `" + KiwiBuiltins.Empty + "`.");
  }
};

#endif