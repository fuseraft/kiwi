#ifndef KIWI_BUILTINS_CONCURRENCY_H
#define KIWI_BUILTINS_CONCURRENCY_H

#include <cstdlib>
#include <string>
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/serializer.h"
#include "typing/value.h"

class ConsoleBuiltinHandler {
 public:
  static k_value execute(const Token& term, const KName& builtin,
                         const std::vector<k_value>& args) {
    switch (builtin) {
      case KName::Builtin_Console_Input:
        return executeInput(term, args);

      default:
        break;
    }

    throw UnknownBuiltinError(term, "");
  }

 private:
  static k_value executeInput(const Token& term,
                              const std::vector<k_value>& args) {
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
};

#endif