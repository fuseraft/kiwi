#ifndef KIWI_BUILTINS_SYSHANDLER_H
#define KIWI_BUILTINS_SYSHANDLER_H

#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/value.h"
#include "util/sys.h"
#include "globals.h"

class SysBuiltinHandler {
 public:
  static k_value execute(const Token& token, const KName& builtin,
                         const std::vector<k_value>& args) {
    if (SAFEMODE) {
      return static_cast<k_int>(0);
    }

    switch (builtin) {
      case KName::Builtin_Sys_Exec:
        return executeExec(token, args);

      case KName::Builtin_Sys_ExecOut:
        return executeExecOut(token, args);

      case KName::Builtin_Sys_EffectiveUserId:
        return executeEffectiveUserId(token, args);

      default:
        break;
    }

    throw UnknownBuiltinError(token, "");
  }

 private:
  static k_value executeEffectiveUserId(const Token& token,
                                        const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, SysBuiltins.EffectiveUserId);
    }

    return static_cast<k_int>(Sys::getEffectiveUserId());
  }

  static k_value executeExec(const Token& token,
                             const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, SysBuiltins.Exec);
    }

    k_string command = get_string(token, args.at(0));
    return Sys::exec(command);
  }

  static k_value executeExecOut(const Token& token,
                                const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, SysBuiltins.Exec);
    }

    k_string command = get_string(token, args.at(0));
    return Sys::execOut(command);
  }
};

#endif