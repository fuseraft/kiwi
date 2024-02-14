#ifndef KIWI_BUILTINS_CONSOLE_H
#define KIWI_BUILTINS_CONSOLE_H

#include <cstdlib>
#include <string>
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/serializer.h"
#include "typing/valuetype.h"

class ConsoleBuiltinHandler {
 public:
  static Value execute(const Token& tokenTerm, const std::string& builtin,
                       const std::vector<Value>& args) {
    if (builtin == ConsoleBuiltins.Input) {
      return executeInput(tokenTerm, args);
    }

    throw UnknownBuiltinError(tokenTerm, builtin);
  }

 private:
  static Value executeInput(const Token& tokenTerm,
                            const std::vector<Value>& args) {
    if (args.size() > 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, ConsoleBuiltins.Input);
    }

    std::string userInput;
    if (args.size() == 1) {
      std::cout << Serializer::serialize(args.at(0));
    }
    std::getline(std::cin, userInput);

    return userInput;
  }
};

#endif