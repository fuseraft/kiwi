#ifndef KIWI_INTERPBUILTIN_H
#define KIWI_INTERPBUILTIN_H

#include <charconv>
#include <sstream>
#include <string>
#include <vector>
#include "builtins/argv_handler.h"
#include "builtins/console_handler.h"
#include "builtins/core_handler.h"
#include "builtins/encoder_handler.h"
#include "builtins/env_handler.h"
#include "builtins/fileio_handler.h"
#include "builtins/logging_handler.h"
#include "builtins/math_handler.h"
#include "builtins/sys_handler.h"
#include "builtins/time_handler.h"
#include "builtins/http_handler.h"
#include "builtins/reflector_handler.h"
#include "tracing/error.h"
#include "parsing/builtins.h"
#include "typing/value.h"

class BuiltinDispatch {
 public:
  static k_value execute(
      const Token& term, const KName& builtin, const std::vector<k_value>& args,
      const std::unordered_map<k_string, k_string>& kiwiArgs) {
    if (FileIOBuiltIns.is_builtin(builtin)) {
      return FileIOBuiltinHandler::execute(term, builtin, args);
    } else if (TimeBuiltins.is_builtin(builtin)) {
      return TimeBuiltinHandler::execute(term, builtin, args);
    } else if (MathBuiltins.is_builtin(builtin)) {
      return MathBuiltinHandler::execute(term, builtin, args);
    } else if (EnvBuiltins.is_builtin(builtin)) {
      return EnvBuiltinHandler::execute(term, builtin, args);
    } else if (EncoderBuiltins.is_builtin(builtin)) {
      return EncoderBuiltinHandler::execute(term, builtin, args);
    } else if (ArgvBuiltins.is_builtin(builtin)) {
      return ArgvBuiltinHandler::execute(term, builtin, args, kiwiArgs);
    } else if (ConsoleBuiltins.is_builtin(builtin)) {
      return ConsoleBuiltinHandler::execute(term, builtin, args);
    } else if (SysBuiltins.is_builtin(builtin)) {
      return SysBuiltinHandler::execute(term, builtin, args);
    } else if (HttpBuiltins.is_builtin(builtin)) {
      return HttpBuiltinHandler::execute(term, builtin, args);
    } else if (LoggingBuiltins.is_builtin(builtin)) {
      return LoggingBuiltinHandler::execute(term, builtin, args);
    } else if (ReflectorBuiltins.is_builtin(builtin)) {
      return ReflectorBuiltinHandler::execute(term, builtin, args);
    }

    throw UnknownBuiltinError(term, term.getText());
  }

  static k_value execute(const Token& term, const KName& builtin,
                         const k_value& value,
                         const std::vector<k_value>& args) {
    if (KiwiBuiltins.is_builtin(builtin)) {
      return CoreBuiltinHandler::execute(term, builtin, value, args);
    }

    throw UnknownBuiltinError(term, term.getText());
  }
};

#endif
