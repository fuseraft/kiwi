#ifndef KIWI_BUILTINS_ARGV_H
#define KIWI_BUILTINS_ARGV_H

#include <cstdlib>
#include <string>
#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/strings.h"
#include "parsing/tokens.h"
#include "typing/valuetype.h"

class ArgvBuiltinHandler {
 public:
  static Value execute(const Token& tokenTerm, const std::string& builtin,
                       const std::vector<Value>& args,
                       const std::map<std::string, std::string>& kiwiArgs) {
    if (builtin == ArgvBuiltins.GetArgv) {
      return executeGetArgv(tokenTerm, args, kiwiArgs);
    } else if (builtin == ArgvBuiltins.GetXarg) {
      return executeGetXarg(tokenTerm, args, kiwiArgs);
    }

    throw UnknownBuiltinError(tokenTerm, builtin);
  }

 private:
  static Value executeGetArgv(
      const Token& tokenTerm, const std::vector<Value>& args,
      const std::map<std::string, std::string>& kiwiArgs) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, ArgvBuiltins.GetArgv);
    }

    std::shared_ptr<List> argv = std::make_shared<List>();

    for (const auto& pair : kiwiArgs) {
      if (Strings::begins_with(pair.first, "argv_")) {
        argv->elements.push_back(pair.second);
      }
    }

    return argv;
  }

  static Value executeGetXarg(
      const Token& tokenTerm, const std::vector<Value>& args,
      const std::map<std::string, std::string>& kiwiArgs) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, ArgvBuiltins.GetXarg);
    }

    std::string xargName = get_string(tokenTerm, args.at(0));

    for (const auto& pair : kiwiArgs) {
      if (pair.first == xargName) {
        return pair.second;
      }
    }

    return "";
  }
};

#endif