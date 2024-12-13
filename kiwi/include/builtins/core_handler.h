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
  static KValue execute(const Token& token, const KName& builtin,
                        const KValue& value, const std::vector<KValue>& args) {
    if (KiwiBuiltins.is_builtin(builtin)) {
      return executeKiwiBuiltin(token, builtin, value, args);
    }

    throw UnknownBuiltinError(token, "");
  }

 private:
  static KValue executeKiwiBuiltin(const Token& token, const KName& builtin,
                                   const KValue& value,
                                   const std::vector<KValue>& args) {
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

  static KValue executeLines(const Token& token, const KValue& value,
                             const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Lines);
    }

    auto s = get_string(token, value);
    return KValue::createList(String::lines(s));
  }

  static KValue executeTokens(const Token& token, const KValue& value,
                              const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Tokens);
    }

    auto s = get_string(token, value);
    std::vector<KValue> tokens;

    Lexer lex(token.getFile(), s);

    auto ts = lex.getAllTokens();
    tokens.reserve(ts.size());

    for (const auto& token : ts) {
      tokens.emplace_back(KValue::createString(token.getText()));
    }

    return KValue::createList(std::make_shared<List>(tokens));
  }

  static KValue executeTruthy(const Token& token, const KValue& value,
                              const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Truthy);
    }

    switch (value.getType()) {
      case KValueType::_INTEGER:
        return KValue::createBoolean(value.getInteger() !=
                                     static_cast<k_int>(0));

      case KValueType::_FLOAT:
        return KValue::createBoolean(value.getFloat() !=
                                     static_cast<double>(0));

      case KValueType::_BOOLEAN:
        return KValue::createBoolean(value.getBoolean());

      case KValueType::_STRING:
        return KValue::createBoolean(!value.getString().empty());

      case KValueType::_LIST:
        return KValue::createBoolean(!value.getList()->elements.empty());

      case KValueType::_HASHMAP:
        return KValue::createBoolean(value.getHashmap()->size() > 0);

      case KValueType::_OBJECT:
        return KValue::createBoolean(true);

      case KValueType::_LAMBDA:
        return KValue::createBoolean(true);

      case KValueType::_NONE:
        return KValue::createBoolean(false);

      default:
        return KValue::createBoolean(false);
    }
  }

  static KValue executeGet(const Token& token, const KValue& value,
                           const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Get);
    }

    if (value.isHashmap()) {
      const auto& hash = value.getHashmap();
      const auto& key = args.at(0);
      if (!hash->hasKey(key)) {
        throw HashKeyError(token, Serializer::serialize(key));
      }

      return hash->get(key);
    } else if (value.isList()) {
      auto index = get_integer(token, args.at(0));
      auto elements = value.getList()->elements;
      if (index < 0 || index >= static_cast<k_int>(elements.size())) {
        throw RangeError(token, "List index out of range.");
      }
      return elements.at(index);
    } else if (value.isString()) {
      auto index = get_integer(token, args.at(0));
      auto str = get_string(token, value);
      if (index < 0 || index >= static_cast<k_int>(str.size())) {
        throw RangeError(token, "List index out of range.");
      }
      return KValue::createString(k_string(1, str.at(index)));
    }

    throw InvalidOperationError(
        token, "Expected a hashmap, list, or string in call to `" +
                   KiwiBuiltins.Get + "`");
  }

  static KValue executeSet(const Token& token, const KValue& value,
                           const std::vector<KValue>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Set);
    }

    if (value.isHashmap()) {
      auto& hash = value.getHashmap();
      hash->add(args.at(0), args.at(1));
      return value;
    } else if (value.isList()) {
      const auto& index = get_integer(token, args.at(0));
      auto& elements = value.getList()->elements;
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

  static KValue executeSwap(const Token& token, const KValue& value,
                            const std::vector<KValue>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Swap);
    }

    if (value.isList()) {
      auto firstIndex = get_integer(token, args.at(0));
      auto secondIndex = get_integer(token, args.at(1));
      auto& elements = value.getList()->elements;
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

  static KValue executeFirst(const Token& token, const KValue& value,
                             const std::vector<KValue>& args) {
    if (args.size() > 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.First);
    }

    if (!value.isList()) {
      throw InvalidOperationError(
          token, "Expected a list in call to `" + KiwiBuiltins.First + "`");
    }

    auto list = value.getList()->elements;
    if (list.empty()) {
      if (args.size() == 1) {
        return args.at(0);
      }
      return KValue::createNull();
    }

    return list.front();
  }

  static KValue executeLast(const Token& token, const KValue& value,
                            const std::vector<KValue>& args) {
    if (args.size() > 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Last);
    }

    if (!value.isList()) {
      throw InvalidOperationError(
          token, "Expected a list in call to `" + KiwiBuiltins.Last + "`");
    }

    auto list = value.getList()->elements;
    if (list.empty()) {
      if (args.size() == 1) {
        return args.at(0);
      }
      return KValue::createNull();
    }

    return list.back();
  }

  static KValue executeChars(const Token& token, const KValue& value,
                             const std::vector<KValue>& args) {
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
      elements.emplace_back(KValue::createString(temp));
    }

    return KValue::createList(newList);
  }

  static KValue executeClone(const Token& token, const KValue& value,
                             const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Clone);
    }

    return clone_value(value);
  }

  static KValue executePretty(const Token& token, const KValue& value,
                              const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Pretty);
    }

    return KValue::createString(Serializer::pretty_serialize(value));
  }

  static KValue executeJoin(const Token& token, const KValue& value,
                            const std::vector<KValue>& args) {
    int argSize = args.size();

    if (argSize != 0 && argSize != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Join);
    }

    if (!value.isList()) {
      throw InvalidOperationError(token, "Cannot join a non-list type.");
    }

    auto list = value.getList();
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

    return KValue::createString(sv.str());
  }

  static KValue executeSize(const Token& token, const KValue& value,
                            const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Size);
    }

    if (value.isString()) {
      return KValue::createInteger(
          static_cast<k_int>(value.getString().length()));
    } else if (value.isList()) {
      return KValue::createInteger(
          static_cast<k_int>(value.getList()->elements.size()));
    } else if (value.isHashmap()) {
      return KValue::createInteger(
          static_cast<k_int>(value.getHashmap()->size()));
    }

    throw InvalidOperationError(
        token, "Invalid type for builtin `" + KiwiBuiltins.Size + "`.");
  }

  static KValue executeToHex(const Token& token, const KValue& value,
                             const std::vector<KValue>& args) {
    if (args.size() > 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.ToHex);
    }

    if (!value.isList()) {
      throw InvalidOperationError(
          token, "Expected a list value for byte to string conversion.");
    }

    auto& elements = value.getList()->elements;

    if (elements.empty()) {
      throw EmptyListError(token);
    }

    std::stringstream ss;
    int width = 2;

    if (!args.empty()) {
      if (!args.at(0).isInteger()) {
        throw ConversionError(token,
                              "Expected an integer as the width argument.");
      }

      width = args.at(0).getInteger();
      if (width < 2) {
        throw InvalidOperationError(token, "Width must be >= 2.");
      }
    }

    for (const auto& item : elements) {
      if (!item.isInteger()) {
        throw InvalidOperationError(
            token, "Expected an integer value for byte to string conversion.");
      }

      auto byte = item.getInteger();
      ss << std::hex << std::setw(width) << std::setfill('0') << byte;
    }

    return KValue::createString(ss.str());
  }

  static KValue executeToBytes(const Token& token, const KValue& value,
                               const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.ToBytes);
    }

    if (value.isString()) {
      auto stringValue = value.getString();
      std::vector<uint8_t> bytes(stringValue.begin(), stringValue.end());
      auto byteList = std::make_shared<List>();
      auto& elements = byteList->elements;
      elements.reserve(bytes.size());

      for (const auto& byte : bytes) {
        elements.emplace_back(KValue::createInteger(static_cast<k_int>(byte)));
      }

      return KValue::createList(byteList);
    } else if (value.isList()) {
      auto listElements = value.getList()->elements;
      auto byteList = std::make_shared<List>();
      auto& elements = byteList->elements;

      for (const auto& item : listElements) {
        if (!item.isString()) {
          throw InvalidOperationError(
              token, "Expected a list to contain only string values.");
        }

        auto stringValue = item.getString();
        std::vector<uint8_t> bytes(stringValue.begin(), stringValue.end());

        for (const auto& byte : bytes) {
          elements.emplace_back(
              KValue::createInteger(static_cast<k_int>(byte)));
        }
      }

      return KValue::createList(byteList);
    } else {
      throw InvalidOperationError(
          token, "Expected a string or list to convert to bytes.");
    }
  }

  static KValue executeToDouble(const Token& token, const KValue& value,
                                const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.ToD);
    }

    if (value.isString()) {
      k_string stringValue = value.getString();
      double doubleValue = 0;
      auto [ptr, ec] =
          std::from_chars(stringValue.data(),
                          stringValue.data() + stringValue.size(), doubleValue);

      if (ec == std::errc()) {
        return KValue::createFloat(doubleValue);
      } else {
        throw ConversionError(
            token, "Cannot convert non-numeric value to a double: `" +
                       stringValue + "`");
      }
    } else if (value.isInteger()) {
      return KValue::createFloat(static_cast<double>(value.getInteger()));
    } else {
      throw ConversionError(token,
                            "Cannot convert non-numeric value to a double.");
    }
  }

  static KValue executeToInteger(const Token& token, const KValue& value,
                                 const std::vector<KValue>& args) {
    if (args.size() > 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.ToI);
    }

    int base = 10;

    if (!args.empty()) {
      if (!args.at(0).isInteger()) {
        throw ConversionError(token,
                              "Expected an integer as the base argument.");
      }

      base = args.at(0).getInteger();
      if (base < 2 || base > 36) {
        throw InvalidOperationError(
            token, "Base must be between 2 and 36, inclusive.");
      }
    }

    if (value.isString()) {
      k_string stringValue = value.getString();
      k_int intValue = 0;
      auto [ptr, ec] = std::from_chars(stringValue.data(),
                                       stringValue.data() + stringValue.size(),
                                       intValue, base);

      if (ec == std::errc()) {
        return KValue::createInteger(intValue);
      } else {
        throw ConversionError(
            token, "Cannot convert non-numeric value to an integer: `" +
                       stringValue + "`");
      }
    } else if (value.isFloat()) {
      return KValue::createInteger(static_cast<k_int>(value.getFloat()));
    } else if (value.isInteger()) {
      return value;
    } else {
      throw ConversionError(token,
                            "Cannot convert non-numeric value to an integer.");
    }
  }

  static KValue executeToString(const Token& token, const KValue& value,
                                const std::vector<KValue>& args) {
    if (args.size() != 0 && args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.ToS);
    }

    if (args.empty()) {
      return KValue::createString(Serializer::serialize(value));
    }

    auto format = get_string(token, args.at(0));

    if (String::trim(format).empty()) {
      return KValue::createString(Serializer::serialize(value));
    }

    if (!value.isFloat() && !value.isInteger()) {
      throw ArgumentError(
          token, "Expected an integer or double for numeric formatting.");
    }

    std::ostringstream sv;

    if (format == "b" || format == "B") {
      auto toBinary = get_integer(token, value);
      sv << std::bitset<16>(toBinary);
      return KValue::createString(sv.str());
    } else if (format == "x" || format == "X") {
      sv << std::hex << get_integer(token, value);
      if (format == "x") {
        return KValue::createString(sv.str());
      }
      return KValue::createString(String::toUppercase(sv.str()));
    } else if (format == "o" || format == "O") {
      sv << std::oct << get_integer(token, value);
      if (format == "o") {
        return KValue::createString(sv.str());
      }
      return KValue::createString(String::toUppercase(sv.str()));
    } else if (String::beginsWith(String::toLowercase(format), "f") ||
               String::toLowercase(format) == "f") {
      // Fixed point
      try {
        auto precision = String::replace(String::toLowercase(format), "f", "");
        if (precision.empty()) {
          sv << std::fixed << std::setprecision(0) << get_float(token, value);
        } else {
          sv << std::fixed << std::setprecision(std::stoi(precision))
             << get_float(token, value);
        }
        return KValue::createString(sv.str());
      } catch (const std::exception& e) {
        throw ArgumentError(token,
                            "Invalid fixed-point format `" + format + "`");
      }
    }

    throw ArgumentError(token, "Unknown format specifier `" + format + "`");
  }

  static KValue executeSubstring(const Token& token, const KValue& value,
                                 const std::vector<KValue>& args) {
    if (args.size() != 1 && args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Substring);
    }

    if (!value.isString()) {
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

    return KValue::createString(String::substring(stringValue, pos, size));
  }

  static KValue executeFind(const Token& token, const KValue& value,
                            const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Find);
    }

    auto stringValue = get_string(token, value);
    auto pattern = get_string(token, args.at(0));

    return KValue::createString(String::find(stringValue, pattern));
  }

  static KValue executeMatch(const Token& token, const KValue& value,
                             const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Match);
    }

    auto stringValue = get_string(token, value);
    auto pattern = get_string(token, args.at(0));

    return KValue::createList(String::match(stringValue, pattern));
  }

  static KValue executeMatches(const Token& token, const KValue& value,
                               const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Matches);
    }

    auto stringValue = get_string(token, value);
    auto pattern = get_string(token, args.at(0));

    return KValue::createBoolean(String::matches(stringValue, pattern));
  }

  static KValue executeMatchesAll(const Token& token, const KValue& value,
                                  const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.MatchesAll);
    }

    auto stringValue = get_string(token, value);
    auto pattern = get_string(token, args.at(0));

    return KValue::createBoolean(String::matchesAll(stringValue, pattern));
  }

  static KValue executeScan(const Token& token, const KValue& value,
                            const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Scan);
    }

    auto stringValue = get_string(token, value);
    auto pattern = get_string(token, args.at(0));

    return KValue::createList(String::scan(stringValue, pattern));
  }

  static KValue executeSplit(const Token& token, const KValue& value,
                             const std::vector<KValue>& args) {
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
        elements.emplace_back(KValue::createString(temp));
      }
    } else {
      const auto& tokens = String::split(input, delimiter);
      elements.reserve(tokens.size());
      for (const auto& token : tokens) {
        elements.emplace_back(KValue::createString(token));
      }
    }

    return KValue::createList(newList);
  }

  static KValue executeRSplit(const Token& token, const KValue& value,
                              const std::vector<KValue>& args) {
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
        elements.emplace_back(KValue::createString(temp));
      }
    } else {
      const auto& tokens = String::rsplit(input, delimiter, limit);
      elements.reserve(tokens.size());
      for (const auto& token : tokens) {
        elements.emplace_back(KValue::createString(token));
      }
    }

    return KValue::createList(newList);
  }

  static KValue executeLeftTrim(const Token& token, const KValue& value,
                                const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.LeftTrim);
    }

    return KValue::createString(String::trimLeft(get_string(token, value)));
  }

  static KValue executeRightTrim(const Token& token, const KValue& value,
                                 const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.RightTrim);
    }

    return KValue::createString(String::trimRight(get_string(token, value)));
  }

  static KValue executeTrim(const Token& token, const KValue& value,
                            const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Trim);
    }

    return KValue::createString(String::trim(get_string(token, value)));
  }

  static KValue executeType(const Token& token, const KValue& value,
                            const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Type);
    }

    return KValue::createString(Serializer::get_value_type_string(value));
  }

  static KValue executeHasKey(const Token& token, const KValue& value,
                              const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.HasKey);
    }

    if (!value.isHashmap()) {
      throw InvalidOperationError(
          token, "Expected a hashmap for `" + KiwiBuiltins.HasKey + "`.");
    }

    return KValue::createBoolean(value.getHashmap()->hasKey(args.at(0)));
  }

  static KValue executeKeys(const Token& token, const KValue& value,
                            const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Keys);
    }

    if (!value.isHashmap()) {
      throw InvalidOperationError(
          token, "Expected a hashmap `" + KiwiBuiltins.Keys + "`.");
    }

    return KValue::createList(
        Serializer::get_hash_keys_list(value.getHashmap()));
  }

  static KValue executeValues(const Token& token, const KValue& value,
                              const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Values);
    }

    if (!value.isHashmap()) {
      throw InvalidOperationError(
          token, "Expected a hashmap for `" + KiwiBuiltins.Values + "`.");
    }

    return KValue::createList(
        Serializer::get_hash_values_list(value.getHashmap()));
  }

  static KValue executeMerge(const Token& token, const KValue& value,
                             const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Merge);
    }

    if (!value.isHashmap() || !args.at(0).isHashmap()) {
      throw InvalidOperationError(
          token, "Expected a hashmap for `" + KiwiBuiltins.Merge + "`.");
    }

    auto& hashValue = value.getHashmap();
    auto mergeHash = args.at(0).getHashmap();

    hashValue->merge(mergeHash);

    return value;
  }

  static KValue executeBeginsWith(const Token& token, const KValue& value,
                                  const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.BeginsWith);
    }

    return KValue::createBoolean(String::beginsWith(
        get_string(token, value), get_string(token, args.at(0))));
  }

  static KValue executeStringContains(const Token& token, const KValue& value,
                                      const KValue& arg) {
    return KValue::createBoolean(
        String::contains(get_string(token, value), get_string(token, arg)));
  }

  static KValue executeListContains(const KValue& value, const KValue& arg) {
    auto list = value.getList();
    auto& elements = list->elements;

    for (const auto& item : elements) {
      if (same_value(item.getValue(), arg.getValue())) {
        return KValue::createBoolean(true);
      }
    }

    return KValue::createBoolean(false);
  }

  static KValue executeContains(const Token& token, const KValue& value,
                                const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Contains);
    }

    if (value.isString()) {
      return executeStringContains(token, value, args.at(0));
    } else if (value.isList()) {
      return executeListContains(value, args.at(0));
    }

    throw InvalidOperationError(token, "Expected a string or list value.");
  }

  static KValue executeEndsWith(const Token& token, const KValue& value,
                                const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Contains);
    }

    return KValue::createBoolean(String::endsWith(
        get_string(token, value), get_string(token, args.at(0))));
  }

  static KValue executeIsA(const Token& token, const KValue& value,
                           const std::vector<KValue>& args) {
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

    switch (value.getType()) {
      case KValueType::_INTEGER:
        return KValue::createBoolean(typeName == TypeNames.Integer);

      case KValueType::_FLOAT:
        return KValue::createBoolean(typeName == TypeNames.Float);

      case KValueType::_BOOLEAN:
        return KValue::createBoolean(typeName == TypeNames.Boolean);

      case KValueType::_STRING:
        return KValue::createBoolean(typeName == TypeNames.String);

      case KValueType::_LIST:
        return KValue::createBoolean(typeName == TypeNames.List);

      case KValueType::_HASHMAP:
        return KValue::createBoolean(typeName == TypeNames.Hashmap);

      case KValueType::_OBJECT:
        return KValue::createBoolean(typeName == TypeNames.Object);

      case KValueType::_LAMBDA:
        return KValue::createBoolean(typeName == TypeNames.Lambda);

      case KValueType::_NONE:
        return KValue::createBoolean(typeName == TypeNames.None);

      default:
        return KValue::createBoolean(false);
    }
  }

  static KValue executeReplace(const Token& token, const KValue& value,
                               const std::vector<KValue>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Replace);
    }

    return KValue::createString(String::replace(get_string(token, value),
                                                get_string(token, args.at(0)),
                                                get_string(token, args.at(1))));
  }

  static KValue executeRReplace(const Token& token, const KValue& value,
                                const std::vector<KValue>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.RReplace);
    }

    return KValue::createString(String::rreplace(
        get_string(token, value), get_string(token, args.at(0)),
        get_string(token, args.at(1))));
  }

  static KValue executeReverse(const Token& token, const KValue& value,
                               const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Reverse);
    }

    if (value.isString()) {
      auto s = value.getString();
      std::reverse(s.begin(), s.end());
      return KValue::createString(s);
    } else if (value.isList()) {
      auto v = value.getList()->elements;
      std::reverse(v.begin(), v.end());
      auto list = std::make_shared<List>();
      list->elements = v;
      return KValue::createList(list);
    }

    throw InvalidOperationError(token,
                                "Expected a string or a list for builtin `" +
                                    KiwiBuiltins.Reverse + "`.");
  }

  static KValue executeIndexOf(const Token& token, const KValue& value,
                               const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.IndexOf);
    }

    if (value.isString()) {
      return KValue::createInteger(
          String::indexOf(value.getString(), get_string(token, args.at(0))));
    } else if (value.isList()) {
      return indexof_listvalue(value.getList(), args.at(0).getValue());
    }

    throw InvalidOperationError(token,
                                "Expected a string or a list for builtin `" +
                                    KiwiBuiltins.IndexOf + "`.");
  }

  static KValue executeLastIndexOf(const Token& token, const KValue& value,
                                   const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.LastIndexOf);
    }

    if (value.isString()) {
      return KValue::createInteger(String::lastIndexOf(
          value.getString(), get_string(token, args.at(0))));
    } else if (value.isList()) {
      return lastindexof_listvalue(value.getList(), args.at(0).getValue());
    }

    throw InvalidOperationError(token,
                                "Expected a string or a list for builtin `" +
                                    KiwiBuiltins.LastIndexOf + "`.");
  }

  static KValue executeUppercase(const Token& token, const KValue& value,
                                 const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Uppercase);
    }

    return KValue::createString(String::toUppercase(get_string(token, value)));
  }

  static KValue executeLowercase(const Token& token, const KValue& value,
                                 const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Lowercase);
    }

    return KValue::createString(String::toLowercase(get_string(token, value)));
  }

  static KValue executeEmpty(const Token& token, const KValue& value,
                             const std::vector<KValue>& args) {
    if (args.size() > 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Empty);
    }

    bool isEmpty = false;
    if (value.isString()) {
      isEmpty = value.getString().empty();
    } else if (value.isList()) {
      isEmpty = value.getList()->elements.empty();
    } else if (value.isHashmap()) {
      isEmpty = value.getHashmap()->keys.empty();
    } else if (value.isInteger()) {
      isEmpty = value.getInteger() == 0;
    } else if (value.isFloat()) {
      isEmpty = value.getFloat() == 0.0;
    } else if (value.isBoolean()) {
      isEmpty = !value.getBoolean();
    } else if (value.isNull()) {
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

    return KValue::createBoolean(isEmpty);
  }

  static KValue executePush(const Token& token, const KValue& value,
                            const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Push);
    }

    if (!value.isList()) {
      throw InvalidOperationError(
          token, "Expected a list for builtin `" + KiwiBuiltins.Push + "`.");
    }

    value.getList()->elements.push_back(args.at(0));
    return KValue::createBoolean(true);
  }

  static KValue executePop(const Token& token, const KValue& value,
                           const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Pop);
    }

    if (!value.isList()) {
      throw InvalidOperationError(
          token, "Expected a list for builtin `" + KiwiBuiltins.Pop + "`.");
    }

    auto& elements = value.getList()->elements;

    if (elements.empty()) {
      return {};
    }

    auto _value = elements.back();
    elements.pop_back();
    return _value;
  }

  static KValue executeEnqueue(const Token& token, const KValue& value,
                               const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Enqueue);
    }

    if (!value.isList()) {
      throw InvalidOperationError(
          token, "Expected a list for builtin `" + KiwiBuiltins.Enqueue + "`.");
    }

    value.getList()->elements.push_back(args.at(0));
    return KValue::createBoolean(true);
  }

  static KValue executeDequeue(const Token& token, const KValue& value,
                               const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Dequeue);
    }

    if (!value.isList()) {
      throw InvalidOperationError(
          token, "Expected a list for builtin `" + KiwiBuiltins.Dequeue + "`.");
    }

    auto& elements = value.getList()->elements;

    if (elements.empty()) {
      return {};
    }

    auto _value = elements.front();
    elements.erase(elements.begin());
    return _value;
  }

  static KValue executeShift(const Token& token, const KValue& value,
                             const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Shift);
    }

    if (!value.isList()) {
      throw InvalidOperationError(
          token, "Expected a list for builtin `" + KiwiBuiltins.Shift + "`.");
    }

    auto& elements = value.getList()->elements;

    if (elements.empty()) {
      return {};
    }

    auto _value = elements.front();
    elements.erase(elements.begin());
    return _value;
  }

  static KValue executeUnshift(const Token& token, const KValue& value,
                               const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Unshift);
    }

    if (!value.isList()) {
      throw InvalidOperationError(
          token, "Expected a list for builtin `" + KiwiBuiltins.Unshift + "`.");
    }

    auto& elements = value.getList()->elements;
    elements.insert(elements.begin(), args.at(0));
    return value;
  }

  static KValue executeConcat(const Token& token, const KValue& value,
                              const std::vector<KValue>& args) {
    if (args.size() != 1 || !args.at(0).isList()) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Concat);
    }

    if (!value.isList()) {
      throw InvalidOperationError(
          token, "Expected a list for builtin `" + KiwiBuiltins.Concat + "`.");
    }

    auto& elements = value.getList()->elements;
    const auto& concat = args.at(0).getList()->elements;
    elements.insert(elements.end(), concat.begin(), concat.end());
    return value;
  }

  static KValue executeInsert(const Token& token, const KValue& value,
                              const std::vector<KValue>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Insert);
    }

    if (!value.isList()) {
      throw InvalidOperationError(
          token, "Expected a list for builtin `" + KiwiBuiltins.Insert + "`.");
    }

    auto& elements = value.getList()->elements;
    size_t index = get_integer(token, args.at(1));

    if (index > elements.size()) {
      throw InvalidOperationError(
          token, "Index out of bounds for `insert` operation.");
    }

    elements.insert(elements.begin() + index, args.at(0));
    return value;
  }

  static KValue executeRemove(const Token& token, const KValue& value,
                              const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Remove);
    }

    if (value.isHashmap()) {
      auto& hash = value.getHashmap();
      hash->remove(args.at(0));
      return value;
    } else if (value.isList()) {
      auto& elements = value.getList()->elements;
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

  static KValue executeRemoveAt(const Token& token, const KValue& value,
                                const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.RemoveAt);
    }

    if (!value.isList()) {
      throw InvalidOperationError(token, "Expected a list for builtin `" +
                                             KiwiBuiltins.RemoveAt + "`.");
    }

    auto& elements = value.getList()->elements;
    size_t index = get_integer(token, args.at(0));

    if (index >= elements.size()) {
      throw InvalidOperationError(
          token, "Index out of bounds for `removeAt` operation.");
    }

    elements.erase(elements.begin() + index);
    return value;
  }

  static KValue executeRotate(const Token& token, const KValue& value,
                              const std::vector<KValue>& args) {
    if (args.size() != 1 || !args.at(0).isInteger()) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Rotate);
    }

    if (!value.isList()) {
      throw InvalidOperationError(
          token, "Expected a list for builtin `" + KiwiBuiltins.Rotate + "`.");
    }

    auto& elements = value.getList()->elements;
    auto rotation = args.at(0).getInteger();

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

  static KValue executeUnique(const Token& token, const KValue& value,
                              const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Unique);
    }

    if (!value.isList()) {
      throw InvalidOperationError(
          token, "Expected a list for builtin `" + KiwiBuiltins.Unique + "`.");
    }

    auto& elements = value.getList()->elements;
    std::unordered_set<k_value> seen;
    auto newEnd = std::remove_if(elements.begin(), elements.end(),
                                 [&seen](const KValue& item) {
                                   return !seen.insert(item.getValue()).second;
                                 });
    elements.erase(newEnd, elements.end());
    return value;
  }

  static KValue executeCount(const Token& token, const KValue& value,
                             const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Count);
    }

    if (value.isString()) {
      const auto& needle = get_string(token, args.at(0));
      const auto& haystack = value.getString();
      return KValue::createInteger(String::count(haystack, needle));
    } else if (value.isList()) {
      const auto& elements = value.getList()->elements;
      return KValue::createInteger(static_cast<k_int>(
          std::count(elements.begin(), elements.end(), args.at(0))));
    }

    throw InvalidOperationError(
        token,
        "Expected a list or string for builtin `" + KiwiBuiltins.Count + "`.");
  }

  static KValue executeFlatten(const Token& token, const KValue& value,
                               const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Flatten);
    }

    if (!value.isList()) {
      throw InvalidOperationError(
          token, "Expected a list for builtin `" + KiwiBuiltins.Flatten + "`.");
    }

    auto flattened = std::make_shared<List>();
    std::function<void(const KValue&)> flattenElement;
    flattenElement = [&flattened, &flattenElement](const KValue& element) {
      if (element.isList()) {
        for (const auto& subElement : element.getList()->elements) {
          flattenElement(subElement);
        }
      } else {
        flattened->elements.push_back(element);
      }
    };

    const auto& elements = value.getList()->elements;
    for (const auto& element : elements) {
      flattenElement(element);
    }
    return KValue::createList(flattened);
  }

  static KValue executeZip(const Token& token, const KValue& value,
                           const std::vector<KValue>& args) {
    if (args.size() != 1 || !args.at(0).isList()) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Zip);
    }

    if (!value.isList()) {
      throw InvalidOperationError(
          token, "Expected a list for builtin `" + KiwiBuiltins.Zip + "`.");
    }

    const auto& elements1 = value.getList()->elements;
    const auto& elements2 = args.at(0).getList()->elements;
    auto zipped = std::make_shared<List>();
    auto win_min = (elements1.size() < elements2.size()) ? elements1.size()
                                                         : elements2.size();
    for (size_t i = 0; i < win_min; ++i) {
      auto pair = std::make_shared<List>();
      pair->elements.push_back(elements1.at(i));
      pair->elements.push_back(elements2.at(i));
      zipped->elements.push_back(KValue::createList(pair));
    }

    return KValue::createList(zipped);
  }

  static KValue executeSlice(const Token& token, const KValue& value,
                             const std::vector<KValue>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Slice);
    }

    if (!value.isList()) {
      throw InvalidOperationError(
          token, "Expected a list for builtin `" + KiwiBuiltins.Slice + "`.");
    }

    auto& elements = value.getList()->elements;
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
    return KValue::createList(slicedList);
  }

  static KValue executeClear(const Token& token, const KValue& value,
                             const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, KiwiBuiltins.Clear);
    }

    if (value.isList()) {
      value.getList()->elements.clear();
      return value;
    } else if (value.isHashmap()) {
      auto& hash = value.getHashmap();
      hash->keys.clear();
      hash->kvp.clear();
      return value;
    }

    throw InvalidOperationError(
        token, "Invalid type for builtin `" + KiwiBuiltins.Empty + "`.");
  }
};

#endif