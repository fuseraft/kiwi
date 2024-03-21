#ifndef KIWI_BUILTINS_ENV_H
#define KIWI_BUILTINS_ENV_H

#include <cstdlib>
#include <string>
#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/value.h"

class EnvBuiltinHandler {
 public:
  static Value execute(const Token& term, const KName& builtin,
                       const std::vector<Value>& args) {
    switch (builtin) {
      case KName::Builtin_Env_GetEnvironmentVariable:
        return executeGetEnvironmentVariable(term, args);

      case KName::Builtin_Env_SetEnvironmentVariable:
        return executeSetEnvironmentVariable(term, args);

      default:
        break;
    }

    throw UnknownBuiltinError(term, "");
  }

 private:
  static Value executeGetEnvironmentVariable(const Token& term,
                                             const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term,
                                           EnvBuiltins.GetEnvironmentVariable);
    }

    std::string varName = get_string(term, args.at(0));
    const char* varValue = std::getenv(varName.c_str());

    if (varValue != nullptr) {
      return std::string(varValue);
    }

    // If it's not there, just return an empty string for now.
    return "";
  }

  static Value executeSetEnvironmentVariable(const Token& term,
                                             const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term,
                                           EnvBuiltins.SetEnvironmentVariable);
    }

    std::string varName = get_string(term, args.at(0));
    std::string varValue = get_string(term, args.at(1));

#ifdef _WIN64
    _putenv_s(varName.c_str(), varValue.c_str());
    return true;
#else
    return setenv(varName.c_str(), varValue.c_str(), 1) == 0;
#endif
  }
};

#endif