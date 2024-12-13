#ifndef KIWI_BUILTINS_ENCODER_H
#define KIWI_BUILTINS_ENCODER_H

#include <cstdlib>
#include <string>
#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/value.h"
#include "util/string.h"

class EncoderBuiltinHandler {
 public:
  static KValue execute(const Token& token, const KName& builtin,
                        const std::vector<KValue>& args) {
    switch (builtin) {
      case KName::Builtin_Encoder_Base64Decode:
        return executeBase64Decode(token, args);

      case KName::Builtin_Encoder_Base64Encode:
        return executeBase64Encode(token, args);

      case KName::Builtin_Encoder_UrlDecode:
        return executeUrlDecode(token, args);

      case KName::Builtin_Encoder_UrlEncode:
        return executeUrlEncode(token, args);

      default:
        break;
    }

    throw UnknownBuiltinError(token, "");
  }

 private:
  static KValue executeBase64Decode(const Token& token,
                                    const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, EncoderBuiltins.Base64Decode);
    }

    auto value = get_string(token, args.at(0));
    return KValue::createString(String::base64Decode(value));
  }

  static KValue executeBase64Encode(const Token& token,
                                    const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, EncoderBuiltins.Base64Encode);
    }

    auto value = get_string(token, args.at(0));
    return KValue::createString(String::base64Encode(value));
  }

  static KValue executeUrlDecode(const Token& token,
                                 const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, EncoderBuiltins.UrlDecode);
    }

    auto value = get_string(token, args.at(0));
    return KValue::createString(String::urlDecode(value));
  }

  static KValue executeUrlEncode(const Token& token,
                                 const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, EncoderBuiltins.UrlEncode);
    }

    auto value = get_string(token, args.at(0));
    return KValue::createString(String::urlEncode(value));
  }
};

#endif