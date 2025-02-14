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
  static KValue execute(const Token& token, const KName& builtin,
                        const std::vector<KValue>& args) {
    if (SAFEMODE) {
      return {};
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
  static KValue executeDeleteGetHeadOptions(const Token& token,
                                            const std::vector<KValue>& args,
                                            const KName& builtin) {
    if (args.size() != 3) {
      throw BuiltinUnexpectedArgumentError(token, token.getText());
    }

    if (!args.at(2).isHashmap()) {
      throw InvalidOperationError(token, "Expected a hashmap for headers.");
    }

    auto url = get_string(token, args.at(0));
    auto path = get_string(token, args.at(1));
    auto headers = args.at(2).getHashmap();

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

  static KValue executePatchPostPut(const Token& token,
                                    const std::vector<KValue>& args,
                                    const KName& builtin) {
    if (args.size() != 5) {
      throw BuiltinUnexpectedArgumentError(token, token.getText());
    }

    if (!args.at(4).isHashmap()) {
      throw InvalidOperationError(token, "Expected a hashmap for headers.");
    }

    auto url = get_string(token, args.at(0));
    auto path = get_string(token, args.at(1));
    auto body = Serializer::serialize(args.at(2));
    auto contentType = get_string(token, args.at(3));
    auto headers = args.at(4).getHashmap();

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

  static KValue executeGet(const k_string& url, const k_string& path,
                           const k_hashmap& headers) {
    httplib::Client cli(url);
    auto res = cli.Get(path, getHeaders(headers));
    return getResponseHash(res);
  }

  static KValue executeDelete(const k_string& url, const k_string& path,
                              const k_hashmap& headers) {
    httplib::Client cli(url);
    auto res = cli.Delete(path, getHeaders(headers));
    return getResponseHash(res);
  }

  static KValue executeHead(const k_string& url, const k_string& path,
                            const k_hashmap& headers) {
    httplib::Client cli(url);
    auto res = cli.Head(path, getHeaders(headers));
    return getResponseHash(res);
  }

  static KValue executeOptions(const k_string& url, const k_string& path,
                               const k_hashmap& headers) {
    httplib::Client cli(url);
    auto res = cli.Options(path, getHeaders(headers));
    return getResponseHash(res);
  }

  static KValue executePost(const k_string& url, const k_string& path,
                            const k_string& body, const k_string& contentType,
                            const k_hashmap& headers) {
    httplib::Client cli(url);
    auto res = cli.Post(path, getHeaders(headers), body, contentType);
    return getResponseHash(res);
  }

  static KValue executePut(const k_string& url, const k_string& path,
                           const k_string& body, const k_string& contentType,
                           const k_hashmap& headers) {
    httplib::Client cli(url);
    auto res = cli.Put(path, getHeaders(headers), body, contentType);
    return getResponseHash(res);
  }

  static KValue executePatch(const k_string& url, const k_string& path,
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

  static KValue getResponseHash(const httplib::Result& res) {
    auto resHash = std::make_shared<Hashmap>();

    if (res) {
      resHash->add(KValue::createString("status"),
                   KValue::createInteger(res->status));
      resHash->add(KValue::createString("body"),
                   KValue::createString(res->body));

      auto headersHash = std::make_shared<Hashmap>();
      for (const auto& pair : res->headers) {
        headersHash->add(KValue::createString(pair.first),
                         KValue::createString(pair.second));
      }

      resHash->add(KValue::createString("headers"),
                   KValue::createHashmap(headersHash));
    } else {
      resHash->add(KValue::createString("status"), {});
      resHash->add(
          KValue::createString("body"),
          KValue::createString("Request failed or no response received"));
      resHash->add(KValue::createString("headers"),
                   KValue::createHashmap(std::make_shared<Hashmap>()));
    }

    return KValue::createHashmap(resHash);
  }
};

#endif