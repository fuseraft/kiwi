#ifndef ASTRAL_BUILTINS_ARGV_H
#define ASTRAL_BUILTINS_ARGV_H

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
  static k_value execute(
      const Token& term, const KName& builtin, const std::vector<k_value>& args,
      const std::unordered_map<k_string, k_string>& astralArgs) {
    switch (builtin) {
      case KName::Builtin_Argv_GetArgv:
        return executeGetArgv(term, args, astralArgs);

      case KName::Builtin_Argv_GetXarg:
        return executeGetXarg(term, args, astralArgs);

      default:
        break;
    }

    throw UnknownBuiltinError(term, "");
  }

 private:
  static k_value executeGetArgv(
      const Token& term, const std::vector<k_value>& args,
      const std::unordered_map<k_string, k_string>& astralArgs) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, ArgvBuiltins.GetArgv);
    }

    auto argv = std::make_shared<List>();
    auto& elements = argv->elements;

    for (const auto& pair : astralArgs) {
      if (String::beginsWith(pair.first, "argv_")) {
        elements.emplace_back(pair.second);
      }
    }

    return argv;
  }

  static k_value executeGetXarg(
      const Token& term, const std::vector<k_value>& args,
      const std::unordered_map<k_string, k_string>& astralArgs) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, ArgvBuiltins.GetXarg);
    }

    k_string xargName = get_string(term, args.at(0));

    for (const auto& pair : astralArgs) {
      if (pair.first == xargName) {
        return pair.second;
      }
    }

    return "";
  }
};

#endif