#ifndef KIWI_INTERPBUILTIN_H
#define KIWI_INTERPBUILTIN_H

#include <charconv>
#include <sstream>
#include <string>
#include <vector>
#include "builtins/argv_handler.h"
#include "builtins/console_handler.h"
#include "builtins/core_handler.h"
#include "builtins/env_handler.h"
#include "builtins/fileio_handler.h"
#include "builtins/math_handler.h"
#include "builtins/sys_handler.h"
#include "builtins/time_handler.h"
#ifdef EXPERIMENTAL_FEATURES
#include "builtins/http_handler.h"
#include "builtins/odbc_handler.h"
#endif
#include "tracing/error.h"
#include "parsing/builtins.h"
#include "typing/value.h"

class BuiltinInterpreter {
 public:
  static k_value execute(
      const Token& term, const KName& builtin, const std::vector<k_value>& args,
      const std::unordered_map<k_string, k_string>& astralArgs) {
    if (FileIOBuiltIns.is_builtin(builtin)) {
      return FileIOBuiltinHandler::execute(term, builtin, args);
    } else if (TimeBuiltins.is_builtin(builtin)) {
      return TimeBuiltinHandler::execute(term, builtin, args);
    } else if (MathBuiltins.is_builtin(builtin)) {
      return MathBuiltinHandler::execute(term, builtin, args);
    } else if (EnvBuiltins.is_builtin(builtin)) {
      return EnvBuiltinHandler::execute(term, builtin, args);
    } else if (ArgvBuiltins.is_builtin(builtin)) {
      return ArgvBuiltinHandler::execute(term, builtin, args, astralArgs);
    } else if (ConsoleBuiltins.is_builtin(builtin)) {
      return ConsoleBuiltinHandler::execute(term, builtin, args);
    } else if (SysBuiltins.is_builtin(builtin)) {
      return SysBuiltinHandler::execute(term, builtin, args);
    } else {
#ifdef EXPERIMENTAL_FEATURES
      if (HttpBuiltins.is_builtin(builtin)) {
        return HttpBuiltinHandler::execute(term, builtin, args);
      } else if (OdbcBuiltins.is_builtin(builtin)) {
        return OdbcBuiltinHandler::execute(term, builtin, args);
      }
#endif
    }

    throw UnknownBuiltinError(term, "");
  }

  static k_value execute(const Token& term, const KName& builtin,
                       const k_value& value, const std::vector<k_value>& args) {
    if (KiwiBuiltins.is_builtin(builtin)) {
      return CoreBuiltinHandler::execute(term, builtin, value, args);
    }

    throw UnknownBuiltinError(term, "");
  }
};

#endif
