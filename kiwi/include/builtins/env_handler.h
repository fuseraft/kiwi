#ifndef KIWI_BUILTINS_ENV_H
#define KIWI_BUILTINS_ENV_H

#include <cstdlib>
#include <string>
#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/value.h"
#include "util/file.h"
#include "globals.h"

class EnvBuiltinHandler {
 public:
  static k_value execute(const Token& token, const KName& builtin,
                         const std::vector<k_value>& args) {
    if (SAFEMODE) {
      return static_cast<k_int>(0);
    }

    switch (builtin) {
      case KName::Builtin_Env_GetEnvironmentVariable:
        return executeGetEnvironmentVariable(token, args);

      case KName::Builtin_Env_SetEnvironmentVariable:
        return executeSetEnvironmentVariable(token, args);

      case KName::Builtin_Env_UnsetEnvironmentVariable:
        return executeUnsetEnvironmentVariable(token, args);

      case KName::Builtin_Env_Kiwi:
        return executeKiwi(token, args);

      case KName::Builtin_Env_KiwiLib:
        return executeKiwiLib(token, args);

      default:
        break;
    }

    throw UnknownBuiltinError(token, "");
  }

 private:
  static k_value executeKiwi(const Token& token,
                             const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, EnvBuiltins.Kiwi);
    }

    return File::getExecutablePath().string();
  }

  static k_value executeKiwiLib(const Token& token,
                                const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, EnvBuiltins.KiwiLib);
    }

    return File::getLibraryPath(token);
  }

  static k_value executeGetEnvironmentVariable(
      const Token& token, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           EnvBuiltins.GetEnvironmentVariable);
    }

    k_string varName = get_string(token, args.at(0));
    const char* varValue = std::getenv(varName.c_str());

    if (varValue != nullptr) {
      return k_string(varValue);
    }

    // If it's not there, just return an empty string for now.
    return "";
  }

  static k_value executeSetEnvironmentVariable(
      const Token& token, const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token,
                                           EnvBuiltins.SetEnvironmentVariable);
    }

    k_string varName = get_string(token, args.at(0));
    k_string varValue = get_string(token, args.at(1));

    return setenv(varName.c_str(), varValue.c_str(), 1) == 0;
  }

  static k_value executeUnsetEnvironmentVariable(
      const Token& token, const std::vector<k_value>& args) {

    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(
          token, EnvBuiltins.UnsetEnvironmentVariable);
    }

    k_string var = get_string(token, args.at(0));

    return unsetenv(var.c_str()) == 0;
  }
};

#endif