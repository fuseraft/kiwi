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
  static Value execute(const Token& tokenTerm, const std::string& builtin,
                       const std::vector<Value>& args) {
    if (builtin == HttpBuiltins.Get) {
      return executeDeleteGetHeadOptions(tokenTerm, args, builtin);
    } else if (builtin == HttpBuiltins.Post) {
      return executePatchPostPut(tokenTerm, args, builtin);
    } else if (builtin == HttpBuiltins.Put) {
      return executePatchPostPut(tokenTerm, args, builtin);
    } else if (builtin == HttpBuiltins.Patch) {
      return executePatchPostPut(tokenTerm, args, builtin);
    } else if (builtin == HttpBuiltins.Delete) {
      return executeDeleteGetHeadOptions(tokenTerm, args, builtin);
    } else if (builtin == HttpBuiltins.Head) {
      return executeDeleteGetHeadOptions(tokenTerm, args, builtin);
    } else if (builtin == HttpBuiltins.Options) {
      return executeDeleteGetHeadOptions(tokenTerm, args, builtin);
    }

    throw UnknownBuiltinError(tokenTerm, builtin);
  }

 private:
  static Value executeDeleteGetHeadOptions(const Token& tokenTerm,
                                           const std::vector<Value>& args,
                                           const std::string& builtin) {
    if (args.size() < 1 || args.size() > 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, builtin);
    }

    std::string url = get_string(tokenTerm, args.at(0));
    std::shared_ptr<List> headers = {};

    if (args.size() == 2) {
      if (!std::holds_alternative<std::shared_ptr<List>>(args.at(1))) {
        throw InvalidOperationError(
            tokenTerm, "Expected a List type for HTTP header list.");
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

    throw UnknownBuiltinError(tokenTerm, builtin);
  }

  static Value executePatchPostPut(const Token& tokenTerm,
                                   const std::vector<Value>& args,
                                   const std::string& builtin) {
    if (args.size() < 1 || args.size() > 3) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, builtin);
    }

    std::string url = get_string(tokenTerm, args.at(0));
    std::string body = Serializer::serialize(args.at(1));
    std::shared_ptr<List> headers = {};

    if (args.size() == 3) {
      if (!std::holds_alternative<std::shared_ptr<List>>(args.at(2))) {
        throw InvalidOperationError(
            tokenTerm, "Expected a List type for HTTP header list.");
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

    throw UnknownBuiltinError(tokenTerm, builtin);
  }
};

#endif