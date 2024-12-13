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
  static KValue execute(const Token& token, const KName& builtin,
                        const std::vector<KValue>& args) {
    if (SAFEMODE) {
      return {};
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
  static KValue executeEffectiveUserId(const Token& token,
                                       const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, SysBuiltins.EffectiveUserId);
    }

    return KValue::createInteger(Sys::getEffectiveUserId());
  }

  static KValue executeExec(const Token& token,
                            const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, SysBuiltins.Exec);
    }

    auto command = get_string(token, args.at(0));
    return KValue::createInteger(Sys::exec(command));
  }

  static KValue executeExecOut(const Token& token,
                               const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, SysBuiltins.Exec);
    }

    auto command = get_string(token, args.at(0));
    return KValue::createString(Sys::execOut(command));
  }
};

#endif