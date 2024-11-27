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
#include "builtins/ffi_handler.h"
#include "builtins/fileio_handler.h"
#include "builtins/logging_handler.h"
#include "builtins/math_handler.h"
#include "builtins/net_handler.h"
#include "builtins/sys_handler.h"
#include "builtins/task_handler.h"
#include "builtins/time_handler.h"
#include "builtins/http_handler.h"
#include "tracing/error.h"
#include "parsing/builtins.h"
#include "typing/value.h"

class BuiltinDispatch {
 public:
  static k_value execute(FFIManager& ffi, const Token& token,
                         const KName& builtin,
                         const std::vector<k_value>& args) {
    return FFIBuiltinHandler::execute(ffi, token, builtin, args);
  }

  static k_value execute(SocketManager& sockmgr, const Token& token,
                         const KName& builtin,
                         const std::vector<k_value>& args) {
    return NetBuiltinHandler::execute(sockmgr, token, builtin, args);
  }

  static k_value execute(TaskManager& taskmgr, const Token& token,
                         const KName& builtin,
                         const std::vector<k_value>& args) {
    return TaskBuiltinHandler::execute(taskmgr, token, builtin, args);
  }

  static k_value execute(
      const Token& token, const KName& builtin,
      const std::vector<k_value>& args,
      const std::unordered_map<k_string, k_string>& kiwiArgs) {
    if (FileIOBuiltIns.is_builtin(builtin)) {
      return FileIOBuiltinHandler::execute(token, builtin, args);
    } else if (TimeBuiltins.is_builtin(builtin)) {
      return TimeBuiltinHandler::execute(token, builtin, args);
    } else if (MathBuiltins.is_builtin(builtin)) {
      return MathBuiltinHandler::execute(token, builtin, args);
    } else if (EnvBuiltins.is_builtin(builtin)) {
      return EnvBuiltinHandler::execute(token, builtin, args);
    } else if (EncoderBuiltins.is_builtin(builtin)) {
      return EncoderBuiltinHandler::execute(token, builtin, args);
    } else if (ArgvBuiltins.is_builtin(builtin)) {
      return ArgvBuiltinHandler::execute(token, builtin, args, kiwiArgs);
    } else if (ConsoleBuiltins.is_builtin(builtin)) {
      return ConsoleBuiltinHandler::execute(token, builtin, args);
    } else if (SysBuiltins.is_builtin(builtin)) {
      return SysBuiltinHandler::execute(token, builtin, args);
    } else if (HttpBuiltins.is_builtin(builtin)) {
      return HttpBuiltinHandler::execute(token, builtin, args);
    } else if (LoggingBuiltins.is_builtin(builtin)) {
      return LoggingBuiltinHandler::execute(token, builtin, args);
    }

    throw UnknownBuiltinError(token, token.getText());
  }

  static k_value execute(const Token& token, const KName& builtin,
                         const k_value& value,
                         const std::vector<k_value>& args) {
    if (KiwiBuiltins.is_builtin(builtin)) {
      return CoreBuiltinHandler::execute(token, builtin, value, args);
    }

    throw UnknownBuiltinError(token, token.getText());
  }
};

#endif
