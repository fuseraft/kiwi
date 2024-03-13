#ifndef KIWI_BUILTINS_HTTPHANDLER_H
#define KIWI_BUILTINS_HTTPHANDLER_H

#include <vector>
#include "math/functions.h"
#include "net/httpclient.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/valuetype.h"

class HttpBuiltinHandler {
 public:
  static Value execute(const Token& term, const std::string& builtin,
                       const std::vector<Value>& args) {
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
    }

    throw UnknownBuiltinError(term, builtin);
  }

 private:
  static Value executeDeleteGetHeadOptions(const Token& term,
                                           const std::vector<Value>& args,
                                           const std::string& builtin) {
    if (args.size() < 1 || args.size() > 2) {
      throw BuiltinUnexpectedArgumentError(term, builtin);
    }

    std::string url = get_string(term, args.at(0));
    std::shared_ptr<List> headers = {};

    if (args.size() == 2) {
      if (!std::holds_alternative<std::shared_ptr<List>>(args.at(1))) {
        throw InvalidOperationError(
            term, "Expected a List type for HTTP header list.");
      }
      headers = std::get<std::shared_ptr<List>>(args.at(1));
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

  static Value executePatchPostPut(const Token& term,
                                   const std::vector<Value>& args,
                                   const std::string& builtin) {
    if (args.size() < 1 || args.size() > 3) {
      throw BuiltinUnexpectedArgumentError(term, builtin);
    }

    std::string url = get_string(term, args.at(0));
    std::string body = Serializer::serialize(args.at(1));
    std::shared_ptr<List> headers = {};

    if (args.size() == 3) {
      if (!std::holds_alternative<std::shared_ptr<List>>(args.at(2))) {
        throw InvalidOperationError(
            term, "Expected a List type for HTTP header list.");
      }
      headers = std::get<std::shared_ptr<List>>(args.at(2));
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
};

#endif