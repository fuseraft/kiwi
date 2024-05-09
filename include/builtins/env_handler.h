#ifndef KIWI_BUILTINS_ENV_H
#define KIWI_BUILTINS_ENV_H

#include <cstdlib>
#include <string>
#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/value.h"
#include "util/file.h"

class EnvBuiltinHandler {
 public:
  static k_value execute(const Token& term, const KName& builtin,
                         const std::vector<k_value>& args) {
    switch (builtin) {
      case KName::Builtin_Env_GetEnvironmentVariable:
        return executeGetEnvironmentVariable(term, args);

      case KName::Builtin_Env_SetEnvironmentVariable:
        return executeSetEnvironmentVariable(term, args);

      case KName::Builtin_Env_UnsetEnvironmentVariable:
        return executeUnsetEnvironmentVariable(term, args);

      case KName::Builtin_Env_Kiwi:
        return executeKiwi(term, args);

      case KName::Builtin_Env_KiwiLib:
        return executeKiwiLib(term, args);

      default:
        break;
    }

    throw UnknownBuiltinError(term, "");
  }

 private:
  static k_value executeKiwi(const Token& term,
                             const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, EnvBuiltins.Kiwi);
    }

    return File::getExecutablePath();
  }

  static k_value executeKiwiLib(const Token& term,
                                const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, EnvBuiltins.KiwiLib);
    }

    return File::getLibraryPath();
  }

  static k_value executeGetEnvironmentVariable(
      const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term,
                                           EnvBuiltins.GetEnvironmentVariable);
    }

    k_string varName = get_string(term, args.at(0));
    const char* varValue = std::getenv(varName.c_str());

    if (varValue != nullptr) {
      return k_string(varValue);
    }

    // If it's not there, just return an empty string for now.
    return "";
  }

  static k_value executeSetEnvironmentVariable(
      const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term,
                                           EnvBuiltins.SetEnvironmentVariable);
    }

    k_string varName = get_string(term, args.at(0));
    k_string varValue = get_string(term, args.at(1));

#ifdef _WIN64
    _putenv_s(varName.c_str(), varValue.c_str());
    return true;
#else
    return setenv(varName.c_str(), varValue.c_str(), 1) == 0;
#endif
  }

  static k_value executeUnsetEnvironmentVariable(
      const Token& term, const std::vector<k_value>& args) {

    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(
          term, EnvBuiltins.UnsetEnvironmentVariable);
    }

    k_string var = get_string(term, args.at(0));

#ifdef _WIN64
    return _putenv_s(var.c_str(), "") == 0;
#else
    return unsetenv(var.c_str()) == 0;
#endif
  }
};

#endif