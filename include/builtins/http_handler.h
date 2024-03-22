#ifndef ASTRAL_BUILTINS_HTTPHANDLER_H
#define ASTRAL_BUILTINS_HTTPHANDLER_H

#include <vector>
#include "math/functions.h"
#include "net/httpclient.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/value.h"

class HttpBuiltinHandler {
 public:
  static k_value execute(const Token& term, const k_string& builtin,
                       const std::vector<k_value>& args) {
    #ifdef EXPERIMENTAL_FEATURES
    switch (builtin) {
      case HttpBuiltins.Delete:
      case HttpBuiltins.Get:
      case HttpBuiltins.Head:
      case HttpBuiltins.Options:
        return executeDeleteGetHeadOptions(term, args, builtin);

      case HttpBuiltins.Patch:
      case HttpBuiltins.Post:
      case HttpBuiltins.Put:
        return executePatchPostPut(term, args, builtin);

      default:
        break;
    }*/

    throw UnknownBuiltinError(term, builtin);
    #endif
  }

 private:
  #ifdef EXPERIMENTAL_FEATURES
  static k_value executeDeleteGetHeadOptions(const Token& term,
                                           const std::vector<k_value>& args,
                                           const k_string& builtin) {
    if (args.size() < 1 || args.size() > 2) {
      throw BuiltinUnexpectedArgumentError(term, builtin);
    }

    k_string url = get_string(term, args.at(0));
    k_list headers = {};

    if (args.size() == 2) {
      if (!std::holds_alternative<k_list>(args.at(1))) {
        throw InvalidOperationError(
            term, "Expected a List type for HTTP header list.");
      }
      headers = std::get<k_list>(args.at(1));
    }

    if (builtin == HttpBuiltins.Get) {
      return HttpClient::getInstance().get(url, headers);
    } else if (builtin == HttpBuiltins.Delete) {
      return HttpClient::getInstance().del(url, headers);
    } else if (builtin == HttpBuiltins.Head) {
      return HttpClient::getInstance().head(url, headers);
    } else if (builtin == HttpBuiltins.Options) {
      return HttpClient::getInstance().options(url, headers);
    }

    throw UnknownBuiltinError(term, builtin);
  }

  static k_value executePatchPostPut(const Token& term,
                                   const std::vector<k_value>& args,
                                   const k_string& builtin) {
    if (args.size() < 1 || args.size() > 3) {
      throw BuiltinUnexpectedArgumentError(term, builtin);
    }

    k_string url = get_string(term, args.at(0));
    k_string body = Serializer::serialize(args.at(1));
    k_list headers = {};

    if (args.size() == 3) {
      if (!std::holds_alternative<k_list>(args.at(2))) {
        throw InvalidOperationError(
            term, "Expected a List type for HTTP header list.");
      }
      headers = std::get<k_list>(args.at(2));
    }

    if (builtin == HttpBuiltins.Post) {
      return HttpClient::getInstance().post(url, body, headers);
    } else if (builtin == HttpBuiltins.Put) {
      return HttpClient::getInstance().put(url, body, headers);
    } else if (builtin == HttpBuiltins.Patch) {
      return HttpClient::getInstance().patch(url, body, headers);
    }

    throw UnknownBuiltinError(term, builtin);
  }
  #endif
};

#endif