#ifndef ASTRAL_BUILTINS_CONCURRENCY_H
#define ASTRAL_BUILTINS_CONCURRENCY_H

#include <cstdlib>
#include <string>
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/serializer.h"
#include "typing/value.h"
#include "globals.h"

class ConsoleBuiltinHandler {
 public:
  static k_value execute(const Token& term, const KName& builtin,
                       const std::vector<k_value>& args) {
    switch (builtin) {
      case KName::Builtin_Console_Input:
        return executeInput(term, args);

      case KName::Builtin_Console_Silent:
        return executeSilence(term, args);

      default:
        break;
    }

    throw UnknownBuiltinError(term, "");
  }

 private:
  static k_value executeInput(const Token& term, const std::vector<k_value>& args) {
    if (args.size() > 1) {
      throw BuiltinUnexpectedArgumentError(term, ConsoleBuiltins.Input);
    }

    k_string userInput;
    if (args.size() == 1) {
      std::cout << Serializer::serialize(args.at(0));
    }
    std::getline(std::cin, userInput);

    return userInput;
  }

  static k_value executeSilence(const Token& term,
                              const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, ConsoleBuiltins.Silent);
    }

    if (!std::holds_alternative<bool>(args.at(0))) {
      throw ConversionError(term, "Expected a Boolean value.");
    }

    SILENCE = std::get<bool>(args.at(0));
    return SILENCE;
  }
};

#endif