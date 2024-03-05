#ifndef KIWI_BUILTINS_SYSHANDLER_H
#define KIWI_BUILTINS_SYSHANDLER_H

#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/valuetype.h"
#include "system/sys.h"

class SysBuiltinHandler {
 public:
  static Value execute(const Token& tokenTerm, const SubTokenType& builtin,
                       const std::vector<Value>& args) {
    if (builtin == SubTokenType::Builtin_Sys_Exec) {
      return executeExec(tokenTerm, args);
    } else if (builtin == SubTokenType::Builtin_Sys_ExecOut) {
      return executeExecOut(tokenTerm, args);
    } else if (builtin == SubTokenType::Builtin_Sys_EffectiveUserId) {
      return executeEffectiveUserId(tokenTerm, args);
    }

    throw UnknownBuiltinError(tokenTerm, "");
  }

 private:
  static Value executeEffectiveUserId(const Token& tokenTerm,
                                      const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           SysBuiltins.EffectiveUserId);
    }

    return Sys::getEffectiveUserId();
  }

  static Value executeExec(const Token& tokenTerm,
                           const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, SysBuiltins.Exec);
    }

    std::string command = get_string(tokenTerm, args.at(0));
    return Sys::exec(command);
  }

  static Value executeExecOut(const Token& tokenTerm,
                              const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, SysBuiltins.Exec);
    }

    std::string command = get_string(tokenTerm, args.at(0));
    return Sys::execOut(command);
  }
};

#endif