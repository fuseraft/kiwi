#ifndef KIWI_INTERPBUILTIN_H
#define KIWI_INTERPBUILTIN_H

#include <charconv>
#include <sstream>
#include <string>
#include <vector>
#include "builtins/core_handler.h"
#include "builtins/http_handler.h"
#include "builtins/fileio_handler.h"
#include "builtins/math_handler.h"
#include "builtins/time_handler.h"
#include "errors/error.h"
#include "parsing/builtins.h"
#include "typing/valuetype.h"

class BuiltinInterpreter {
 public:
  static Value execute(const Token& tokenTerm, const std::string& builtin,
                       const std::vector<Value>& args) {
    if (FileIOBuiltIns.is_builtin(builtin)) {
      return FileIOBuiltinHandler::execute(tokenTerm, builtin, args);
    } else if (TimeBuiltins.is_builtin(builtin)) {
      return TimeBuiltinHandler::execute(tokenTerm, builtin, args);
    } else if (MathBuiltins.is_builtin(builtin)) {
      return MathBuiltinHandler::execute(tokenTerm, builtin, args);
    } else if (HttpBuiltins.is_builtin(builtin)) {
      return HttpBuiltinHandler::execute(tokenTerm, builtin, args);
    }

    throw UnknownBuiltinError(tokenTerm, builtin);
  }

  static Value execute(const Token& tokenTerm, const std::string& builtin,
                       const Value& value, const std::vector<Value>& args) {
    if (KiwiBuiltins.is_builtin(builtin)) {
      return CoreBuiltinHandler::execute(tokenTerm, builtin, value, args);
    }

    throw UnknownBuiltinError(tokenTerm, builtin);
  }
};

#endif
