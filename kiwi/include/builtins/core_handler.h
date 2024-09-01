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
  static k_value execute(const Token& term, const KName& builtin,
                         const k_value& value,
                         const std::vector<k_value>& args) {
    if (KiwiBuiltins.is_builtin(builtin)) {
      return executeKiwiBuiltin(term, builtin, value, args);
    }

    throw UnknownBuiltinError(term, "");
  }

 private:
  static k_value executeKiwiBuiltin(const Token& term, const KName& builtin,
                                    const k_value& value,
                                    const std::vector<k_value>& args) {
    switch (builtin) {
      case KName::Builtin_Kiwi_Chars:
        return executeChars(term, value, args);

        // case KName::Builtin_Kiwi_Members:
        //   return executeMembers(term, value, args);

      case KName::Builtin_Kiwi_IsA:
        return executeIsA(term, value, args);

      case KName::Builtin_Kiwi_Join:
        return executeJoin(term, value, args);

      case KName::Builtin_Kiwi_Split:
        return executeSplit(term, value, args);

      case KName::Builtin_Kiwi_RSplit:
        return executeRSplit(term, value, args);

      case KName::Builtin_Kiwi_Get:
        return executeGet(term, value, args);

      case KName::Builtin_Kiwi_Set:
        return executeSet(term, value, args);

      case KName::Builtin_Kiwi_First:
        return executeFirst(term, value, args);

      case KName::Builtin_Kiwi_Last:
        return executeLast(term, value, args);

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

      case KName::Builtin_Kiwi_ToBytes:
        return executeToBytes(term, value, args);

      case KName::Builtin_Kiwi_ToHex:
        return executeToHex(term, value, args);

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

      case KName::Builtin_Kiwi_RReplace:
        return executeRReplace(term, value, args);

      case KName::Builtin_Kiwi_Reverse:
        return executeReverse(term, value, args);

      case KName::Builtin_Kiwi_IndexOf:
        return executeIndexOf(term, value, args);

      case KName::Builtin_Kiwi_LastIndexOf:
        return executeLastIndexOf(term, value, args);

      case KName::Builtin_Kiwi_Uppercase:
        return executeUppercase(term, value, args);

      case KName::Builtin_Kiwi_Lowercase:
        return executeLowercase(term, value, args);

      case KName::Builtin_Kiwi_Empty:
        return executeEmpty(term, value, args);

      case KName::Builtin_Kiwi_Keys:
        return executeKeys(term, value, args);

      case KName::Builtin_Kiwi_HasKey:
        return executeHasKey(term, value, args);

      case KName::Builtin_Kiwi_Merge:
        return executeMerge(term, value, args);

      case KName::Builtin_Kiwi_Values:
        return executeValues(term, value, args);

      case KName::Builtin_Kiwi_Push:
        return executePush(term, value, args);

      case KName::Builtin_Kiwi_Pop:
        return executePop(term, value, args);

      case KName::Builtin_Kiwi_Enqueue:
        return executeEnqueue(term, value, args);

      case KName::Builtin_Kiwi_Dequeue:
        return executeDequeue(term, value, args);

      case KName::Builtin_Kiwi_Shift:
        return executeShift(term, value, args);

      case KName::Builtin_Kiwi_Unshift:
        return executeUnshift(term, value, args);

      case KName::Builtin_Kiwi_Clear:
        return executeClear(term, value, args);

      case KName::Builtin_Kiwi_Substring:
        return executeSubstring(term, value, args);

      case KName::Builtin_Kiwi_Remove:
        return executeRemove(term, value, args);

      case KName::Builtin_Kiwi_RemoveAt:
        return executeRemoveAt(term, value, args);

      case KName::Builtin_Kiwi_Rotate:
        return executeRotate(term, value, args);

      case KName::Builtin_Kiwi_Insert:
        return executeInsert(term, value, args);

      case KName::Builtin_Kiwi_Slice:
        return executeSlice(term, value, args);

      case KName::Builtin_Kiwi_Swap:
        return executeSwap(term, value, args);

      case KName::Builtin_Kiwi_Concat:
        return executeConcat(term, value, args);

      case KName::Builtin_Kiwi_Unique:
        return executeUnique(term, value, args);

      case KName::Builtin_Kiwi_Count:
        return executeCount(term, value, args);

      case KName::Builtin_Kiwi_Flatten:
        return executeFlatten(term, value, args);

      case KName::Builtin_Kiwi_Zip:
        return executeZip(term, value, args);

      case KName::Builtin_Kiwi_Clone:
        return executeClone(term, value, args);

      case KName::Builtin_Kiwi_Pretty:
        return executePretty(term, value, args);

      case KName::Builtin_Kiwi_Find:
        return executeFind(term, value, args);

      case KName::Builtin_Kiwi_Match:
        return executeMatch(term, value, args);

      case KName::Builtin_Kiwi_Matches:
        return executeMatches(term, value, args);

      case KName::Builtin_Kiwi_MatchesAll:
        return executeMatchesAll(term, value, args);

      case KName::Builtin_Kiwi_Scan:
        return executeScan(term, value, args);

      case KName::Builtin_Kiwi_Truthy:
        return executeTruthy(term, value, args);

      default:
        break;
    }

    throw UnknownBuiltinError(term, "");
  }

  static k_value executeTruthy(const Token& term, const k_value& value,
                               const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Truthy);
    }

    switch (value.index()) {
      case 0:  // k_int
        return std::get<k_int>(value) != static_cast<k_int>(0);

      case 1:  // double
        return std::get<double>(value) != static_cast<double>(0);

      case 2:  // bool
        return std::get<bool>(value);

      case 3:  // k_string
        return !std::get<k_string>(value).empty();

      case 4:  // k_list
        return !std::get<k_list>(value)->elements.empty();

      case 5:  // k_hash
        return std::get<k_hash>(value)->size() > 0;

      case 6:  // k_object
        return true;

      case 7:  // k_lambda
        return true;

      case 8:  // k_null
        return false;

      default:
        return false;
    }
  }

  static k_value executeGet(const Token& term, const k_value& value,
                            const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Get);
    }

    if (std::holds_alternative<k_hash>(value)) {
      auto key = get_string(term, args.at(0));
      return std::get<k_hash>(value)->get(key);
    } else if (std::holds_alternative<k_list>(value)) {
      auto index = get_integer(term, args.at(0));
      auto elements = std::get<k_list>(value)->elements;
      if (index < 0 || index >= static_cast<k_int>(elements.size())) {
        throw RangeError(term, "List index out of range.");
      }
      return elements.at(index);
    } else if (std::holds_alternative<k_string>(value)) {
      auto index = get_integer(term, args.at(0));
      auto str = get_string(term, value);
      if (index < 0 || index >= static_cast<k_int>(str.size())) {
        throw RangeError(term, "List index out of range.");
      }
      return k_string(1, str.at(index));
    }

    throw InvalidOperationError(
        term, "Expected a hash, list, or string in call to `" +
                  KiwiBuiltins.Get + "`");
  }

  static k_value executeSet(const Token& term, const k_value& value,
                            const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Set);
    }

    if (std::holds_alternative<k_hash>(value)) {
      auto key = get_string(term, args.at(0));
      auto& hash = std::get<k_hash>(value);
      hash->add(key, args.at(1));
      return hash;
    } else if (std::holds_alternative<k_list>(value)) {
      auto index = get_integer(term, args.at(0));
      auto& elements = std::get<k_list>(value)->elements;
      if (index < 0 || index >= static_cast<k_int>(elements.size())) {
        throw RangeError(term, "List index out of range.");
      }
      elements[index] = args.at(1);
      return value;
    }

    throw InvalidOperationError(
        term, "Expected a hash or list in call to `" + KiwiBuiltins.Set + "`");
  }

  static k_value executeSwap(const Token& term, const k_value& value,
                             const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Swap);
    }

    if (std::holds_alternative<k_list>(value)) {
      auto firstIndex = get_integer(term, args.at(0));
      auto secondIndex = get_integer(term, args.at(1));
      auto& elements = std::get<k_list>(value)->elements;
      if (firstIndex < 0 || firstIndex >= static_cast<k_int>(elements.size())) {
        throw RangeError(term, "The first parameter for " + KiwiBuiltins.Swap +
                                   " is out of range.");
      }
      if (secondIndex < 0 ||
          secondIndex >= static_cast<k_int>(elements.size())) {
        throw RangeError(term, "The second parameter for " + KiwiBuiltins.Swap +
                                   " is out of range.");
      }

      auto firstValue = elements[firstIndex];
      auto secondValue = elements[secondIndex];
      elements[firstIndex] = secondValue;
      elements[secondIndex] = firstValue;
      return value;
    }

    throw InvalidOperationError(
        term, "Expected a hash or list in call to `" + KiwiBuiltins.Set + "`");
  }

  static k_value executeFirst(const Token& term, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() > 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.First);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          term, "Expected a list in call to `" + KiwiBuiltins.First + "`");
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

  static k_value executeLast(const Token& term, const k_value& value,
                             const std::vector<k_value>& args) {
    if (args.size() > 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Last);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          term, "Expected a list in call to `" + KiwiBuiltins.Last + "`");
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

  static k_value executeChars(const Token& term, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Chars);
    }

    auto newList = std::make_shared<List>();
    auto stringValue = get_string(term, value);
    auto& elements = newList->elements;

    elements.reserve(stringValue.size());
    k_string temp(1, '\0');
    for (const char& c : stringValue) {
      temp[0] = c;
      elements.emplace_back(temp);
    }

    return newList;
  }

  static k_value executeClone(const Token& term, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Clone);
    }

    return clone_value(value);
  }

  static k_value executePretty(const Token& term, const k_value& value,
                               const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Pretty);
    }

    return Serializer::pretty_serialize(value);
  }

  // static k_value executeMembers(const Token& term, const k_value& value,
  //                               const std::vector<k_value>& args) {
  //   if (args.size() != 0) {
  //     throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Members);
  //   }

  //   if (!std::holds_alternative<k_object>(value)) {
  //     throw InvalidOperationError(
  //         term, "Expected an object in call to `" + KiwiBuiltins.Members + "`");
  //   }

  //   auto memberHash = std::make_shared<Hash>();
  //   auto obj = std::get<k_object>(value);
  //   auto& instanceVariables = obj->instanceVariables;
  //   auto clazz = classes[obj->className];

  //   for (const auto& method : clazz.getMethods()) {
  //     memberHash->add(method.first, {});
  //   }

  //   for (const auto& instanceVar : instanceVariables) {
  //     memberHash->add(instanceVar.first, instanceVar.second);
  //   }

  //   return memberHash;
  // }

  static k_value executeJoin(const Token& term, const k_value& value,
                             const std::vector<k_value>& args) {
    int argSize = args.size();

    if (argSize != 0 && argSize != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Join);
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
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Size);
    }

    if (std::holds_alternative<k_string>(value)) {
      return static_cast<k_int>(std::get<k_string>(value).length());
    } else if (std::holds_alternative<k_list>(value)) {
      return static_cast<k_int>(std::get<k_list>(value)->elements.size());
    } else if (std::holds_alternative<k_hash>(value)) {
      return static_cast<k_int>(std::get<k_hash>(value)->size());
    }

    throw InvalidOperationError(
        term, "Invalid type for builtin `" + KiwiBuiltins.Size + "`.");
  }

  static k_value executeToHex(const Token& term, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.ToHex);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          term, "Expected a list value for byte to string conversion.");
    }

    auto& elements = std::get<k_list>(value)->elements;

    if (elements.empty()) {
      throw EmptyListError(term);
    }

    std::stringstream ss;

    for (const auto& item : elements) {
      if (!std::holds_alternative<k_int>(item)) {
        throw InvalidOperationError(
            term, "Expected an integer value for byte to string conversion.");
      }

      auto byte = static_cast<unsigned int>(std::get<k_int>(item)) & 0xFF;
      ss << std::hex << std::setw(2) << std::setfill('0') << byte;
    }

    return ss.str();
  }

  static k_value executeToBytes(const Token& term, const k_value& value,
                                const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.ToBytes);
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
              term, "Expected a list to contain only string values.");
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
          term, "Expected a string or list to convert to bytes.");
    }
  }

  static k_value executeToDouble(const Token& term, const k_value& value,
                                 const std::vector<k_value>& args) {
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

  static k_value executeToInteger(const Token& term, const k_value& value,
                                  const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.ToI);
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
    if (args.size() != 0 && args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.ToS);
    }

    if (args.empty()) {
      return Serializer::serialize(value);
    }

    auto format = get_string(term, args.at(0));

    if (String::trim(format).empty()) {
      return Serializer::serialize(value);
    }

    if (!std::holds_alternative<double>(value) &&
        !std::holds_alternative<k_int>(value)) {
      throw ArgumentError(
          term, "Expected an integer or double for numeric formatting.");
    }

    std::ostringstream sv;

    if (format == "b" || format == "B") {
      auto toBinary = get_integer(term, value);
      sv << std::bitset<16>(toBinary);
      return sv.str();
    } else if (format == "x" || format == "X") {
      sv << std::hex << get_integer(term, value);
      if (format == "x") {
        return sv.str();
      }
      return String::toUppercase(sv.str());
    } else if (format == "o" || format == "O") {
      sv << std::oct << get_integer(term, value);
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
          sv << std::fixed << std::setprecision(0) << get_double(term, value);
        } else {
          sv << std::fixed << std::setprecision(std::stoi(precision))
             << get_double(term, value);
        }
        return sv.str();
      } catch (const std::exception& e) {
        throw ArgumentError(term,
                            "Invalid fixed-point format `" + format + "`");
      }
    }

    throw ArgumentError(term, "Unknown format specifier `" + format + "`");
  }

  static k_value executeSubstring(const Token& term, const k_value& value,
                                  const std::vector<k_value>& args) {
    if (args.size() != 1 && args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Substring);
    }

    if (!std::holds_alternative<k_string>(value)) {
      throw InvalidOperationError(term,
                                  "Expected a string value for builtin `" +
                                      KiwiBuiltins.Substring + "`.");
    }

    auto stringValue = get_string(term, value);
    auto pos = static_cast<size_t>(get_integer(term, args.at(0)));
    auto size = stringValue.size();

    if (args.size() == 2) {
      size = static_cast<size_t>(get_integer(term, args.at(1)));
    }

    return String::substring(stringValue, pos, size);
  }

  static k_value executeFind(const Token& term, const k_value& value,
                             const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Find);
    }

    auto stringValue = get_string(term, value);
    auto pattern = get_string(term, args.at(0));

    return String::find(stringValue, pattern);
  }

  static k_value executeMatch(const Token& term, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Match);
    }

    auto stringValue = get_string(term, value);
    auto pattern = get_string(term, args.at(0));

    return String::match(stringValue, pattern);
  }

  static k_value executeMatches(const Token& term, const k_value& value,
                                const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Matches);
    }

    auto stringValue = get_string(term, value);
    auto pattern = get_string(term, args.at(0));

    return String::matches(stringValue, pattern);
  }

  static k_value executeMatchesAll(const Token& term, const k_value& value,
                                   const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.MatchesAll);
    }

    auto stringValue = get_string(term, value);
    auto pattern = get_string(term, args.at(0));

    return String::matchesAll(stringValue, pattern);
  }

  static k_value executeScan(const Token& term, const k_value& value,
                             const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Scan);
    }

    auto stringValue = get_string(term, value);
    auto pattern = get_string(term, args.at(0));

    return String::scan(stringValue, pattern);
  }

  static k_value executeSplit(const Token& term, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Split);
    }

    k_string input = get_string(term, value);
    auto delimiter = get_string(term, args.at(0));
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

  static k_value executeRSplit(const Token& term, const k_value& value,
                               const std::vector<k_value>& args) {
    if (args.size() != 1 && args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.RSplit);
    }

    k_string input = get_string(term, value);
    auto delimiter = get_string(term, args.at(0));
    auto newList = std::make_shared<List>();
    auto& elements = newList->elements;
    k_int limit = -1;

    if (args.size() == 2) {
      limit = get_integer(term, args.at(1));
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

  static k_value executeLeftTrim(const Token& term, const k_value& value,
                                 const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.LeftTrim);
    }

    return String::trimLeft(get_string(term, value));
  }

  static k_value executeRightTrim(const Token& term, const k_value& value,
                                  const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.RightTrim);
    }

    return String::trimRight(get_string(term, value));
  }

  static k_value executeTrim(const Token& term, const k_value& value,
                             const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Trim);
    }

    return String::trim(get_string(term, value));
  }

  static k_value executeType(const Token& term, const k_value& value,
                             const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Type);
    }

    return Serializer::get_value_type_string(value);
  }

  static k_value executeHasKey(const Token& term, const k_value& value,
                               const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.HasKey);
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
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Keys);
    }

    if (!std::holds_alternative<k_hash>(value)) {
      throw InvalidOperationError(
          term, "Attempted to retrieve keys from non-Hash type.");
    }

    return Serializer::get_hash_keys_list(std::get<k_hash>(value));
  }

  static k_value executeValues(const Token& term, const k_value& value,
                               const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Values);
    }

    if (!std::holds_alternative<k_hash>(value)) {
      throw InvalidOperationError(
          term, "Attempted to retrieve values from non-Hash type.");
    }

    return Serializer::get_hash_values_list(std::get<k_hash>(value));
  }

  static k_value executeMerge(const Token& term, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Merge);
    }

    if (!std::holds_alternative<k_hash>(value) ||
        !std::holds_alternative<k_hash>(args.at(0))) {
      throw InvalidOperationError(term, "Attempted to merge a non-Hash type.");
    }

    auto hashValue = std::get<k_hash>(value);
    auto mergeHash = std::get<k_hash>(args.at(0));

    hashValue->merge(mergeHash);

    return hashValue;
  }

  static k_value executeBeginsWith(const Token& term, const k_value& value,
                                   const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.BeginsWith);
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
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Contains);
    }

    if (std::holds_alternative<k_string>(value)) {
      return executeStringContains(term, value, args.at(0));
    } else if (std::holds_alternative<k_list>(value)) {
      return executeListContains(value, args.at(0));
    }

    throw InvalidOperationError(term, "Expected a string or list value.");
  }

  static k_value executeEndsWith(const Token& term, const k_value& value,
                                 const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Contains);
    }

    return String::endsWith(get_string(term, value),
                            get_string(term, args.at(0)));
  }

  static k_value executeIsA(const Token& term, const k_value& value,
                            const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.IsA);
    }

    auto typeName = get_string(term, args.at(0));
    // if (!TypeNames.is_typename(typeName)) {
    //   if (std::holds_alternative<k_object>(value)) {
    //     const auto& obj = std::get<k_object>(value);
    //     const auto& className = obj->className;

    //     return same_value(className, typeName) ||
    //            same_value(classes.at(className).getBaseClassName(), typeName);
    //   }
    //   throw InvalidTypeNameError(term, typeName);
    // }

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
        return typeName == TypeNames.With;

      case 8:  // k_null
        return typeName == TypeNames.None;

      default:
        return false;
    }
  }

  static k_value executeReplace(const Token& term, const k_value& value,
                                const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Replace);
    }

    return String::replace(get_string(term, value),
                           get_string(term, args.at(0)),
                           get_string(term, args.at(1)));
  }

  static k_value executeRReplace(const Token& term, const k_value& value,
                                 const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.RReplace);
    }

    return String::rreplace(get_string(term, value),
                            get_string(term, args.at(0)),
                            get_string(term, args.at(1)));
  }

  static k_value executeReverse(const Token& term, const k_value& value,
                                const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Reverse);
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

    throw InvalidOperationError(term,
                                "Expected a string or a list for builtin `" +
                                    KiwiBuiltins.Reverse + "`.");
  }

  static k_value executeIndexOf(const Token& term, const k_value& value,
                                const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.IndexOf);
    }

    if (std::holds_alternative<k_string>(value)) {
      return static_cast<k_int>(String::indexOf(std::get<k_string>(value),
                                                get_string(term, args.at(0))));
    } else if (std::holds_alternative<k_list>(value)) {
      return indexof_listvalue(std::get<k_list>(value), args.at(0));
    }

    throw InvalidOperationError(term,
                                "Expected a string or a list for builtin `" +
                                    KiwiBuiltins.IndexOf + "`.");
  }

  static k_value executeLastIndexOf(const Token& term, const k_value& value,
                                    const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.LastIndexOf);
    }

    if (std::holds_alternative<k_string>(value)) {
      return static_cast<k_int>(String::lastIndexOf(
          std::get<k_string>(value), get_string(term, args.at(0))));
    } else if (std::holds_alternative<k_list>(value)) {
      return lastindexof_listvalue(std::get<k_list>(value), args.at(0));
    }

    throw InvalidOperationError(term,
                                "Expected a string or a list for builtin `" +
                                    KiwiBuiltins.LastIndexOf + "`.");
  }

  static k_value executeUppercase(const Token& term, const k_value& value,
                                  const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Uppercase);
    }

    return String::toUppercase(get_string(term, value));
  }

  static k_value executeLowercase(const Token& term, const k_value& value,
                                  const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Lowercase);
    }

    return String::toLowercase(get_string(term, value));
  }

  static k_value executeEmpty(const Token& term, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Empty);
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
        term, "Invalid type for builtin `" + KiwiBuiltins.Empty + "`.");
  }

  static k_value executePush(const Token& term, const k_value& value,
                             const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Push);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          term, "Expected a list for builtin `" + KiwiBuiltins.Push + "`.");
    }

    std::get<k_list>(value)->elements.push_back(args.at(0));
    return true;
  }

  static k_value executePop(const Token& term, const k_value& value,
                            const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Pop);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          term, "Expected a list for builtin `" + KiwiBuiltins.Pop + "`.");
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
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Enqueue);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          term, "Expected a list for builtin `" + KiwiBuiltins.Enqueue + "`.");
    }

    std::get<k_list>(value)->elements.push_back(args.at(0));
    return true;
  }

  static k_value executeDequeue(const Token& term, const k_value& value,
                                const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Dequeue);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          term, "Expected a list for builtin `" + KiwiBuiltins.Dequeue + "`.");
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
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Shift);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          term, "Expected a list for builtin `" + KiwiBuiltins.Shift + "`.");
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
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Unshift);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          term, "Expected a list for builtin `" + KiwiBuiltins.Unshift + "`.");
    }

    auto& elements = std::get<k_list>(value)->elements;
    elements.insert(elements.begin(), args.at(0));
    return value;
  }

  static k_value executeConcat(const Token& term, const k_value& value,
                               const std::vector<k_value>& args) {
    if (args.size() != 1 || !std::holds_alternative<k_list>(args.at(0))) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Concat);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          term, "Expected a list for builtin `" + KiwiBuiltins.Concat + "`.");
    }

    auto& elements = std::get<k_list>(value)->elements;
    const auto& concat = std::get<k_list>(args.at(0))->elements;
    elements.insert(elements.end(), concat.begin(), concat.end());
    return value;
  }

  static k_value executeInsert(const Token& term, const k_value& value,
                               const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Insert);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          term, "Expected a list for builtin `" + KiwiBuiltins.Insert + "`.");
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
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Remove);
    }

    if (std::holds_alternative<k_hash>(value)) {
      auto hash = std::get<k_hash>(value);
      auto key = get_string(term, args.at(0));
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

    throw InvalidOperationError(term, "Expected a hash or list for builtin `" +
                                          KiwiBuiltins.Remove + "`.");
  }

  static k_value executeRemoveAt(const Token& term, const k_value& value,
                                 const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.RemoveAt);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          term, "Expected a list for builtin `" + KiwiBuiltins.RemoveAt + "`.");
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
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Rotate);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          term, "Expected a list for builtin `" + KiwiBuiltins.Rotate + "`.");
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
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Unique);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          term, "Expected a list for builtin `" + KiwiBuiltins.Unique + "`.");
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
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Count);
    }

    if (std::holds_alternative<k_string>(value)) {
      const auto& needle = get_string(term, args.at(0));
      const auto& haystack = std::get<k_string>(value);
      return String::count(haystack, needle);
    } else if (std::holds_alternative<k_list>(value)) {
      const auto& elements = std::get<k_list>(value)->elements;
      return std::count(elements.begin(), elements.end(), args.at(0));
    }

    throw InvalidOperationError(
        term,
        "Expected a list or string for builtin `" + KiwiBuiltins.Count + "`.");
  }

  static k_value executeFlatten(const Token& term, const k_value& value,
                                const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Flatten);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          term, "Expected a list for builtin `" + KiwiBuiltins.Flatten + "`.");
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
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Zip);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          term, "Expected a list for builtin `" + KiwiBuiltins.Zip + "`.");
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

  static k_value executeSlice(const Token& term, const k_value& value,
                              const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Slice);
    }

    if (!std::holds_alternative<k_list>(value)) {
      throw InvalidOperationError(
          term, "Expected a list for builtin `" + KiwiBuiltins.Slice + "`.");
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
      throw BuiltinUnexpectedArgumentError(term, KiwiBuiltins.Clear);
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
        term, "Invalid type for builtin `" + KiwiBuiltins.Empty + "`.");
  }
};

#endif