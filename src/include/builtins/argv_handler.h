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
#include "globals.h"

class ArgvBuiltinHandler {
 public:
  static KValue execute(const Token& token, const KName& builtin,
                        const std::vector<KValue>& args,
                        const std::unordered_map<k_string, k_string>& cliArgs) {
    if (SAFEMODE) {
      return {};
    }

    switch (builtin) {
      case KName::Builtin_Argv_GetArgv:
        return executeGetArgv(token, args, cliArgs);

      case KName::Builtin_Argv_GetXarg:
        return executeGetXarg(token, args, cliArgs);

      default:
        break;
    }

    throw UnknownBuiltinError(token, "");
  }

 private:
  static KValue executeGetArgv(
      const Token& token, const std::vector<KValue>& args,
      const std::unordered_map<k_string, k_string>& cliArgs) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, ArgvBuiltins.GetArgv);
    }

    auto argv = std::make_shared<List>();
    auto& elements = argv->elements;

    for (const auto& pair : cliArgs) {
      if (String::beginsWith(pair.first, "argv_")) {
        elements.emplace_back(KValue::createString(pair.second));
      }
    }

    return KValue::createList(argv);
  }

  static KValue executeGetXarg(
      const Token& token, const std::vector<KValue>& args,
      const std::unordered_map<k_string, k_string>& cliArgs) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, ArgvBuiltins.GetXarg);
    }

    k_string xargName = get_string(token, args.at(0));

    for (const auto& pair : cliArgs) {
      if (pair.first == xargName) {
        return KValue::createString(pair.second);
      }
    }

    return KValue::emptyString();
  }
};

#endif