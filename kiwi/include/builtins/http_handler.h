#ifndef KIWI_BUILTINS_HTTPHANDLER_H
#define KIWI_BUILTINS_HTTPHANDLER_H

#include <memory>
#include <vector>
#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/serializer.h"
#include "typing/value.h"
#include "web/httplib.h"

class HttpBuiltinHandler {
 public:
  static k_value execute(const Token& token, const KName& builtin,
                         const std::vector<k_value>& args) {
    if (SAFEMODE) {
      return static_cast<k_int>(0);
    }

    switch (builtin) {
      case KName::Builtin_WebClient_Delete:
      case KName::Builtin_WebClient_Get:
      case KName::Builtin_WebClient_Head:
      case KName::Builtin_WebClient_Options:
        return executeDeleteGetHeadOptions(token, args, builtin);

      case KName::Builtin_WebClient_Patch:
      case KName::Builtin_WebClient_Post:
      case KName::Builtin_WebClient_Put:
        return executePatchPostPut(token, args, builtin);

      default:
        break;
    }

    throw UnknownBuiltinError(token, token.getText());
  }

 private:
  static k_value executeDeleteGetHeadOptions(const Token& token,
                                             const std::vector<k_value>& args,
                                             const KName& builtin) {
    if (args.size() != 3) {
      throw BuiltinUnexpectedArgumentError(token, token.getText());
    }

    if (!std::holds_alternative<k_hashmap>(args.at(2))) {
      throw InvalidOperationError(token, "Expected a hashmap for headers.");
    }

    auto url = get_string(token, args.at(0));
    auto path = get_string(token, args.at(1));
    auto headers = std::get<k_hashmap>(args.at(2));

    switch (builtin) {
      case KName::Builtin_WebClient_Get:
        return executeGet(url, path, headers);
      case KName::Builtin_WebClient_Delete:
        return executeDelete(url, path, headers);
      case KName::Builtin_WebClient_Head:
        return executeHead(url, path, headers);
      case KName::Builtin_WebClient_Options:
        return executeOptions(url, path, headers);

      default:
        break;
    }

    throw UnknownBuiltinError(token, token.getText());
  }

  static k_value executePatchPostPut(const Token& token,
                                     const std::vector<k_value>& args,
                                     const KName& builtin) {
    if (args.size() != 5) {
      throw BuiltinUnexpectedArgumentError(token, token.getText());
    }

    if (!std::holds_alternative<k_hashmap>(args.at(4))) {
      throw InvalidOperationError(token, "Expected a hashmap for headers.");
    }

    auto url = get_string(token, args.at(0));
    auto path = get_string(token, args.at(1));
    auto body = Serializer::serialize(args.at(2));
    auto contentType = get_string(token, args.at(3));
    auto headers = std::get<k_hashmap>(args.at(4));

    switch (builtin) {
      case KName::Builtin_WebClient_Post:
        return executePost(url, path, body, contentType, headers);
      case KName::Builtin_WebClient_Put:
        return executePost(url, path, body, contentType, headers);
      case KName::Builtin_WebClient_Patch:
        return executePost(url, path, body, contentType, headers);
      default:
        break;
    }

    throw UnknownBuiltinError(token, token.getText());
  }

  static k_value executeGet(const k_string& url, const k_string& path,
                            const k_hashmap& headers) {
    httplib::Client cli(url);
    auto res = cli.Get(path, getHeaders(headers));
    return getResponseHash(res);
  }

  static k_value executeDelete(const k_string& url, const k_string& path,
                               const k_hashmap& headers) {
    httplib::Client cli(url);
    auto res = cli.Delete(path, getHeaders(headers));
    return getResponseHash(res);
  }

  static k_value executeHead(const k_string& url, const k_string& path,
                             const k_hashmap& headers) {
    httplib::Client cli(url);
    auto res = cli.Head(path, getHeaders(headers));
    return getResponseHash(res);
  }

  static k_value executeOptions(const k_string& url, const k_string& path,
                                const k_hashmap& headers) {
    httplib::Client cli(url);
    auto res = cli.Options(path, getHeaders(headers));
    return getResponseHash(res);
  }

  static k_value executePost(const k_string& url, const k_string& path,
                             const k_string& body, const k_string& contentType,
                             const k_hashmap& headers) {
    httplib::Client cli(url);
    auto res = cli.Post(path, getHeaders(headers), body, contentType);
    return getResponseHash(res);
  }

  static k_value executePut(const k_string& url, const k_string& path,
                            const k_string& body, const k_string& contentType,
                            const k_hashmap& headers) {
    httplib::Client cli(url);
    auto res = cli.Put(path, getHeaders(headers), body, contentType);
    return getResponseHash(res);
  }

  static k_value executePatch(const k_string& url, const k_string& path,
                              const k_string& body, const k_string& contentType,
                              const k_hashmap& headers) {
    httplib::Client cli(url);
    auto res = cli.Patch(path, getHeaders(headers), body, contentType);
    return getResponseHash(res);
  }

  static httplib::Headers getHeaders(const k_hashmap& headersHash) {
    httplib::Headers headers;

    for (const auto& key : headersHash->keys) {
      const auto& value = headersHash->kvp[key];
      headers.insert(
          {Serializer::serialize(key), Serializer::serialize(value)});
    }

    return headers;
  }

  static k_value getResponseHash(const httplib::Result& res) {
    auto resHash = std::make_shared<Hashmap>();

    if (res) {
      resHash->add("status", static_cast<k_int>(res->status));
      resHash->add("body", res->body);

      auto headersHash = std::make_shared<Hashmap>();
      for (const auto& pair : res->headers) {
        headersHash->add(pair.first, pair.second);
      }

      resHash->add("headers", headersHash);
    } else {
      resHash->add("status", static_cast<k_int>(0));
      resHash->add("body", "Request failed or no response received");
      resHash->add("headers", std::make_shared<Hashmap>());
    }

    return resHash;
  }
};

#endif