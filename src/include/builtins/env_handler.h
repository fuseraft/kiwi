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
  static KValue execute(const Token& token, const KName& builtin,
                        const std::vector<KValue>& args) {
    if (SAFEMODE) {
      return {};
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
  static KValue executeKiwi(const Token& token,
                            const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, EnvBuiltins.Kiwi);
    }

    return KValue::createString(File::getExecutablePath().string());
  }

  static KValue executeKiwiLib(const Token& token,
                               const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, EnvBuiltins.KiwiLib);
    }

    return KValue::createString(File::getLibraryPath(token));
  }

  static KValue executeGetEnvironmentVariable(const Token& token,
                                              const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           EnvBuiltins.GetEnvironmentVariable);
    }

    k_string varName = get_string(token, args.at(0));
    const char* varValue = std::getenv(varName.c_str());

    if (varValue != nullptr) {
      return KValue::createString(k_string(varValue));
    }

    // If it's not there, just return an empty string for now.
    return KValue::emptyString();
  }

  static KValue executeSetEnvironmentVariable(const Token& token,
                                              const std::vector<KValue>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token,
                                           EnvBuiltins.SetEnvironmentVariable);
    }

    k_string varName = get_string(token, args.at(0));
    k_string varValue = get_string(token, args.at(1));

    return KValue::createBoolean(setenv(varName.c_str(), varValue.c_str(), 1) ==
                                 0);
  }

  static KValue executeUnsetEnvironmentVariable(
      const Token& token, const std::vector<KValue>& args) {

    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(
          token, EnvBuiltins.UnsetEnvironmentVariable);
    }

    k_string var = get_string(token, args.at(0));

    return KValue::createBoolean(unsetenv(var.c_str()) == 0);
  }
};

#endif