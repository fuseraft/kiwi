#ifndef ASTRAL_BUILTINS_ENCODER_H
#define ASTRAL_BUILTINS_ENCODER_H

#include <cstdlib>
#include <string>
#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/value.h"
#include "util/string.h"

class EncoderBuiltinHandler {
 public:
  static k_value execute(const Token& term, const KName& builtin,
                         const std::vector<k_value>& args) {
    switch (builtin) {
      case KName::Builtin_Encoder_Base64Decode:
        return executeBase64Decode(term, args);

      case KName::Builtin_Encoder_Base64Encode:
        return executeBase64Encode(term, args);

      case KName::Builtin_Encoder_UrlDecode:
        return executeUrlDecode(term, args);

      case KName::Builtin_Encoder_UrlEncode:
        return executeUrlEncode(term, args);

      default:
        break;
    }

    throw UnknownBuiltinError(term, "");
  }

 private:
  static k_value executeBase64Decode(const Token& term,
                                     const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, EncoderBuiltins.Base64Decode);
    }

    auto value = get_string(term, args.at(0));
    return String::base64Decode(value);
  }

  static k_value executeBase64Encode(const Token& term,
                                     const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, EncoderBuiltins.Base64Encode);
    }

    auto value = get_string(term, args.at(0));
    return String::base64Encode(value);
  }

  static k_value executeUrlDecode(const Token& term,
                                  const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, EncoderBuiltins.UrlDecode);
    }

    auto value = get_string(term, args.at(0));
    return String::urlDecode(value);
  }

  static k_value executeUrlEncode(const Token& term,
                                  const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, EncoderBuiltins.UrlEncode);
    }

    auto value = get_string(term, args.at(0));
    return String::urlEncode(value);
  }
};

#endif