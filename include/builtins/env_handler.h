#ifndef KIWI_BUILTINS_ENV_H
#define KIWI_BUILTINS_ENV_H

#include <cstdlib>
#include <string>
#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/valuetype.h"

class EnvBuiltinHandler {
 public:
  static Value execute(const Token& tokenTerm, const std::string& builtin,
                       const std::vector<Value>& args) {
    if (builtin == EnvBuiltins.GetEnvironmentVariable) {
      return executeGetEnvironmentVariable(tokenTerm, args);
    } else if (builtin == EnvBuiltins.SetEnvironmentVariable) {
      return executeSetEnvironmentVariable(tokenTerm, args);
    }

    throw UnknownBuiltinError(tokenTerm, builtin);
  }

 private:
  static Value executeGetEnvironmentVariable(const Token& tokenTerm,
                                             const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           EnvBuiltins.GetEnvironmentVariable);
    }

    std::string varName = get_string(tokenTerm, args.at(0));
    const char* varValue = std::getenv(varName.c_str());

    if (varValue != nullptr) {
      return std::string(varValue);
    }

    // If it's not there, just return an empty string for now.
    return "";
  }

  static Value executeSetEnvironmentVariable(const Token& tokenTerm,
                                             const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           EnvBuiltins.SetEnvironmentVariable);
    }

    std::string varName = get_string(tokenTerm, args.at(0));
    std::string varValue = get_string(tokenTerm, args.at(1));

    return setenv(varName.c_str(), varValue.c_str(), 1) == 0;
  }
};

#endif