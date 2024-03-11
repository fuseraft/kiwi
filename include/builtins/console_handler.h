#ifndef KIWI_BUILTINS_CONSOLE_H
#define KIWI_BUILTINS_CONSOLE_H

#include <cstdlib>
#include <string>
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/serializer.h"
#include "typing/valuetype.h"
#include "globals.h"

class ConsoleBuiltinHandler {
 public:
  static Value execute(const Token& term, const SubTokenType& builtin,
                       const std::vector<Value>& args) {
    if (builtin == SubTokenType::Builtin_Console_Input) {
      return executeInput(term, args);
    } else if (builtin == SubTokenType::Builtin_Console_Silent) {
      return executeSilence(term, args);
    }

    throw UnknownBuiltinError(term, "");
  }

 private:
  static Value executeInput(const Token& term, const std::vector<Value>& args) {
    if (args.size() > 1) {
      throw BuiltinUnexpectedArgumentError(term, ConsoleBuiltins.Input);
    }

    std::string userInput;
    if (args.size() == 1) {
      std::cout << Serializer::serialize(args.at(0));
    }
    std::getline(std::cin, userInput);

    return userInput;
  }

  static Value executeSilence(const Token& term,
                              const std::vector<Value>& args) {
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