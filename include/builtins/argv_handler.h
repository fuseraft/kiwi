#ifndef KIWI_BUILTINS_ARGV_H
#define KIWI_BUILTINS_ARGV_H

#include <cstdlib>
#include <string>
#include <unordered_map>
#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/value.h"
#include "util/string.h"

class ArgvBuiltinHandler {
 public:
  static Value execute(
      const Token& term, const KName& builtin,
      const std::vector<Value>& args,
      const std::unordered_map<std::string, std::string>& kiwiArgs) {
    switch (builtin) {
      case KName::Builtin_Argv_GetArgv:
        return executeGetArgv(term, args, kiwiArgs);

      case KName::Builtin_Argv_GetXarg:
        return executeGetXarg(term, args, kiwiArgs);

      default:
        break;
    }

    throw UnknownBuiltinError(term, "");
  }

 private:
  static Value executeGetArgv(
      const Token& term, const std::vector<Value>& args,
      const std::unordered_map<std::string, std::string>& kiwiArgs) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, ArgvBuiltins.GetArgv);
    }

    std::shared_ptr<List> argv = std::make_shared<List>();

    for (const auto& pair : kiwiArgs) {
      if (String::beginsWith(pair.first, "argv_")) {
        argv->elements.push_back(pair.second);
      }
    }

    return argv;
  }

  static Value executeGetXarg(
      const Token& term, const std::vector<Value>& args,
      const std::unordered_map<std::string, std::string>& kiwiArgs) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, ArgvBuiltins.GetXarg);
    }

    std::string xargName = get_string(term, args.at(0));

    for (const auto& pair : kiwiArgs) {
      if (pair.first == xargName) {
        return pair.second;
      }
    }

    return "";
  }
};

#endif