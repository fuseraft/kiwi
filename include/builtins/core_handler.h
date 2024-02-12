#ifndef KIWI_BUILTINS_COREHANDLER_H
#define KIWI_BUILTINS_COREHANDLER_H

#include <charconv>
#include <sstream>
#include <string>
#include <vector>
#include "errors/error.h"
#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/strings.h"
#include "parsing/tokens.h"
#include "system/fileio.h"
#include "system/time.h"
#include "typing/serializer.h"
#include "typing/valuetype.h"

class CoreBuiltinHandler {
 public:
  static Value execute(const Token& tokenTerm, const std::string& builtin,
                       const Value& value, const std::vector<Value>& args) {
    if (KiwiBuiltins.is_builtin(builtin)) {
      return executeKiwiBuiltin(tokenTerm, builtin, value, args);
    }

    throw UnknownBuiltinError(tokenTerm, builtin);
  }

 private:
  static Value executeKiwiBuiltin(const Token& tokenTerm,
                                  const std::string& builtin,
                                  const Value& value,
                                  const std::vector<Value>& args) {
    if (builtin == KiwiBuiltins.Chars) {
      return executeChars(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.IsA) {
      return executeIsA(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.Join) {
      return executeJoin(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.Split) {
      return executeSplit(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.LeftTrim) {
      return executeLeftTrim(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.RightTrim) {
      return executeRightTrim(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.Trim) {
      return executeTrim(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.Type) {
      return executeType(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.Size) {
      return executeSize(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.ToD) {
      return executeToDouble(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.ToI) {
      return executeToInteger(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.ToS) {
      return executeToString(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.BeginsWith) {
      return executeBeginsWith(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.Contains) {
      return executeContains(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.EndsWith) {
      return executeEndsWith(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.Replace) {
      return executeReplace(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.IndexOf) {
      return executeIndexOf(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.Upcase) {
      return executeUpcase(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.Downcase) {
      return executeDowncase(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.Keys) {
      return executeKeys(tokenTerm, value, args);
    }

    throw UnknownBuiltinError(tokenTerm, builtin);
  }

  static std::shared_ptr<List> executeChars(const Token& tokenTerm,
                                            const Value& value,
                                            const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.Chars);
    }

    auto newList = std::make_shared<List>();
    std::string stringValue = get_string(tokenTerm, value);
    for (char c : stringValue) {
      newList->elements.push_back(std::string(1, c));
    }
    return newList;
  }

  static std::string executeJoin(const Token& tokenTerm, const Value& value,
                                 const std::vector<Value>& args) {
    int argSize = args.size();

    if (argSize != 0 && argSize != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.Join);
    }

    if (!std::holds_alternative<std::shared_ptr<List>>(value)) {
      throw ConversionError(tokenTerm, "Cannot join a non-list type.");
    }

    auto list = std::get<std::shared_ptr<List>>(value);
    std::ostringstream sv;
    std::string joiner;

    if (argSize == 1) {
      joiner = get_string(tokenTerm, args.at(0));
    }

    for (auto it = list->elements.begin(); it != list->elements.end(); ++it) {
      if (it != list->elements.begin()) {
        sv << joiner;
      }
      sv << Serializer::serialize(*it);
    }

    return sv.str();
  }

  static int executeSize(const Token& tokenTerm, const Value& value,
                         const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.Size);
    }

    if (std::holds_alternative<std::string>(value)) {
      int size = std::get<std::string>(value).length();
      return size;
    } else if (std::holds_alternative<std::shared_ptr<List>>(value)) {
      auto list = std::get<std::shared_ptr<List>>(value);
      int size = list->elements.size();
      return size;
    } else if (std::holds_alternative<std::shared_ptr<Hash>>(value)) {
      auto hash = std::get<std::shared_ptr<Hash>>(value);
      int size = hash->kvp.size();
      return size;
    } else {
      throw ConversionError(tokenTerm);
    }
  }

  static double executeToDouble(const Token& tokenTerm, const Value& value,
                                const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.ToD);
    }

    if (std::holds_alternative<std::string>(value)) {
      std::string stringValue = std::get<std::string>(value);
      double doubleValue = 0;
      auto [ptr, ec] =
          std::from_chars(stringValue.data(),
                          stringValue.data() + stringValue.size(), doubleValue);

      if (ec == std::errc()) {
        return doubleValue;
      } else {
        throw ConversionError(
            tokenTerm, "Cannot convert non-numeric value to a double: `" +
                           stringValue + "`");
      }
    } else if (std::holds_alternative<int>(value)) {
      return static_cast<double>(std::get<int>(value));
    } else {
      throw ConversionError(tokenTerm,
                            "Cannot convert non-numeric value to a double.");
    }
  }

  static int executeToInteger(const Token& tokenTerm, const Value& value,
                              const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.ToI);
    }

    if (std::holds_alternative<std::string>(value)) {
      std::string stringValue = std::get<std::string>(value);
      int intValue = 0;
      auto [ptr, ec] =
          std::from_chars(stringValue.data(),
                          stringValue.data() + stringValue.size(), intValue);

      if (ec == std::errc()) {
        return intValue;
      } else {
        throw ConversionError(
            tokenTerm, "Cannot convert non-numeric value to an integer: `" +
                           stringValue + "`");
      }
    } else if (std::holds_alternative<double>(value)) {
      return static_cast<int>(std::get<double>(value));
    } else {
      throw ConversionError(tokenTerm,
                            "Cannot convert non-numeric value to an integer.");
    }
  }

  static std::string executeToString(const Token& tokenTerm, const Value& value,
                                     const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.ToS);
    }

    return Serializer::serialize(value);
  }

  static std::shared_ptr<List> executeSplit(const Token& tokenTerm,
                                            const Value& value,
                                            const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.Split);
    }

    std::string input = get_string(tokenTerm, value);
    auto delimiter = get_string(tokenTerm, args.at(0));
    auto newList = std::make_shared<List>();

    if (delimiter.empty()) {
      for (char c : input) {
        newList->elements.push_back(std::string(1, c));
      }
    } else {
      for (std::string token : Strings::split(input, delimiter)) {
        newList->elements.push_back(token);
      }
    }

    return newList;
  }

  static std::string executeLeftTrim(const Token& tokenTerm, const Value& value,
                                     const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.LeftTrim);
    }

    std::string input = get_string(tokenTerm, value);
    return Strings::ltrim(input);
  }

  static std::string executeRightTrim(const Token& tokenTerm,
                                      const Value& value,
                                      const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.RightTrim);
    }

    std::string input = get_string(tokenTerm, value);
    return Strings::rtrim(input);
  }

  static std::string executeTrim(const Token& tokenTerm, const Value& value,
                                 const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.Trim);
    }

    std::string input = get_string(tokenTerm, value);
    return Strings::trim(input);
  }

  static std::string executeType(const Token& tokenTerm, const Value& value,
                                 const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.Type);
    }

    return Serializer::get_value_type_string(value);
  }

  static std::shared_ptr<List> executeKeys(const Token& tokenTerm,
                                           const Value& value,
                                           const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.Keys);
    }

    if (!std::holds_alternative<std::shared_ptr<Hash>>(value)) {
      throw InvalidOperationError(
          tokenTerm, "Attempted to retrieve keys from non-Hash type.");
    }

    auto hash = std::get<std::shared_ptr<Hash>>(value);

    return Serializer::get_hash_keys_list(hash);
  }

  static bool executeBeginsWith(const Token& tokenTerm, const Value& value,
                                const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.BeginsWith);
    }

    auto str = get_string(tokenTerm, value);
    auto search = get_string(tokenTerm, args.at(0));
    return Strings::begins_with(str, search);
  }

  static bool executeContains(const Token& tokenTerm, const Value& value,
                              const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.Contains);
    }

    auto str = get_string(tokenTerm, value);
    auto search = get_string(tokenTerm, args.at(0));
    return Strings::contains(str, search);
  }

  static bool executeEndsWith(const Token& tokenTerm, const Value& value,
                              const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.Contains);
    }

    auto str = get_string(tokenTerm, value);
    auto search = get_string(tokenTerm, args.at(0));
    return Strings::ends_with(str, search);
  }

  static bool executeIsA(const Token& tokenTerm, const Value& value,
                         const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.IsA);
    }

    auto typeName = get_string(tokenTerm, args.at(0));
    if (!TypeNames.is_typename(typeName)) {
      if (std::holds_alternative<std::shared_ptr<Object>>(value)) {
        std::shared_ptr<Object> object =
            std::get<std::shared_ptr<Object>>(value);
        return object->className == typeName;
      }
      throw InvalidTypeNameError(tokenTerm, typeName);
    }

    return (typeName == TypeNames.Boolean &&
            std::holds_alternative<bool>(value)) ||
           (typeName == TypeNames.Double &&
            std::holds_alternative<double>(value)) ||
           (typeName == TypeNames.Hash &&
            std::holds_alternative<std::shared_ptr<Hash>>(value)) ||
           (typeName == TypeNames.Integer &&
            std::holds_alternative<int>(value)) ||
           (typeName == TypeNames.List &&
            std::holds_alternative<std::shared_ptr<List>>(value)) ||
           (typeName == TypeNames.Object &&
            std::holds_alternative<std::shared_ptr<Object>>(value)) ||
           (typeName == TypeNames.Lambda &&
            std::holds_alternative<std::shared_ptr<LambdaRef>>(value)) ||
           (typeName == TypeNames.String &&
            std::holds_alternative<std::string>(value));
  }

  static std::string executeReplace(const Token& tokenTerm, const Value& value,
                                    const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.Replace);
    }

    auto str = get_string(tokenTerm, value);
    auto search = get_string(tokenTerm, args.at(0));
    auto replacement = get_string(tokenTerm, args.at(1));
    return Strings::replace(str, search, replacement);
  }

  static int executeIndexOf(const Token& tokenTerm, const Value& value,
                            const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.IndexOf);
    }

    auto str = get_string(tokenTerm, value);
    auto search = get_string(tokenTerm, args.at(0));
    return Strings::index_of(str, search);
  }

  static std::string executeUpcase(const Token& tokenTerm, const Value& value,
                                   const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.Upcase);
    }

    auto str = get_string(tokenTerm, value);
    return Strings::upcase(str);
  }

  static std::string executeDowncase(const Token& tokenTerm, const Value& value,
                                     const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.Downcase);
    }

    auto str = get_string(tokenTerm, value);
    return Strings::downcase(str);
  }
};

#endif