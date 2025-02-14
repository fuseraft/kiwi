#ifndef KIWI_BUILTINS_CONSOLE_H
#define KIWI_BUILTINS_CONSOLE_H

#include <cstdlib>
#include <string>
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/serializer.h"
#include "typing/value.h"

class ConsoleBuiltinHandler {
 public:
  static KValue execute(const Token& token, const KName& builtin,
                        const std::vector<KValue>& args) {
    switch (builtin) {
      case KName::Builtin_Console_Input:
        return executeInput(token, args);

      default:
        break;
    }

    throw UnknownBuiltinError(token, "");
  }

 private:
  static KValue executeInput(const Token& token,
                             const std::vector<KValue>& args) {
    if (args.size() > 1) {
      throw BuiltinUnexpectedArgumentError(token, ConsoleBuiltins.Input);
    }

    k_string userInput;
    if (args.size() == 1) {
      std::cout << Serializer::serialize(args.at(0));
    }
    std::getline(std::cin, userInput);

    return KValue::createString(userInput);
  }
};

#endif