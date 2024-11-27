#ifndef KIWI_BUILTINS_FFIHANDLER_H
#define KIWI_BUILTINS_FFIHANDLER_H

#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/value.h"
#include "ffi/ffimanager.h"
#include "globals.h"

class FFIBuiltinHandler {
 public:
  static k_value execute(FFIManager& ffi, const Token& token,
                         const KName& builtin,
                         const std::vector<k_value>& args) {
    if (SAFEMODE) {
      return static_cast<k_int>(0);
    }
    switch (builtin) {
      case KName::Builtin_FFI_Attach:
        return executeAttach(ffi, token, args);
      case KName::Builtin_FFI_Invoke:
        return executeInvoke(ffi, token, args);
      case KName::Builtin_FFI_Load:
        return executeLoad(ffi, token, args);
      case KName::Builtin_FFI_Unload:
        return executeUnload(ffi, token, args);
      default:
        break;
    }
    throw InvalidOperationError(token, "Come back later.");
  }

 private:
  static k_value executeAttach(FFIManager& ffi, const Token& token,
                               const std::vector<k_value>& args) {
    if (args.size() != 5) {
      throw BuiltinUnexpectedArgumentError(token, FFIBuiltins.Attach);
    }

    auto libAlias = get_string(token, args.at(0));
    auto funcAlias = get_string(token, args.at(1));
    auto ffiFuncName = get_string(token, args.at(2));
    if (!std::holds_alternative<k_list>(args.at(3))) {
      throw InvalidOperationError(
          token, "Expected a list of parameter types for argument 4 of `" +
                     FFIBuiltins.Attach + "`.");
    }
    auto ffiParameterTypes = std::get<k_list>(args.at(3));
    auto ffiReturnType = get_string(token, args.at(4));

    return ffi.attachFunction(token, libAlias, funcAlias, ffiFuncName,
                              ffiParameterTypes, ffiReturnType);
  }

  static k_value executeInvoke(FFIManager& ffi, const Token& token,
                               const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FFIBuiltins.Invoke);
    }

    auto funcAlias = get_string(token, args.at(0));

    if (!std::holds_alternative<k_list>(args.at(1))) {
      throw InvalidOperationError(
          token, "Expected a list of parameters for argument 2 of `" +
                     FFIBuiltins.Invoke + "`.");
    }

    const auto& funcParams = std::get<k_list>(args.at(1))->elements;

    return ffi.invokeFunction(token, funcAlias, funcParams);
  }

  static k_value executeLoad(FFIManager& ffi, const Token& token,
                             const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FFIBuiltins.Load);
    }

    auto libAlias = get_string(token, args.at(0));
    auto libPath = get_string(token, args.at(1));

    ffi.loadLibrary(token, libAlias, libPath);

    return {};
  }

  static k_value executeUnload(FFIManager& ffi, const Token& token,
                               const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FFIBuiltins.Unload);
    }

    auto libAlias = get_string(token, args.at(0));

    ffi.unloadLibrary(token, libAlias);

    return {};
  }
};

#endif