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
  static KValue execute(FFIManager& ffi, const Token& token,
                        const KName& builtin, const std::vector<KValue>& args) {
    if (SAFEMODE) {
      return {};
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
  static KValue executeAttach(FFIManager& ffi, const Token& token,
                              const std::vector<KValue>& args) {
    if (args.size() != 5) {
      throw BuiltinUnexpectedArgumentError(token, FFIBuiltins.Attach);
    }

    auto libAlias = get_string(token, args.at(0));
    auto funcAlias = get_string(token, args.at(1));
    auto ffiFuncName = get_string(token, args.at(2));
    if (!args.at(3).isList()) {
      throw InvalidOperationError(
          token, "Expected a list of parameter types for argument 4 of `" +
                     FFIBuiltins.Attach + "`.");
    }
    auto ffiParameterTypes = args.at(3).getList();
    auto ffiReturnType = get_string(token, args.at(4));

    return KValue::createBoolean(
        ffi.attachFunction(token, libAlias, funcAlias, ffiFuncName,
                           ffiParameterTypes, ffiReturnType));
  }

  static KValue executeInvoke(FFIManager& ffi, const Token& token,
                              const std::vector<KValue>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FFIBuiltins.Invoke);
    }

    auto funcAlias = get_string(token, args.at(0));

    if (!args.at(1).isList()) {
      throw InvalidOperationError(
          token, "Expected a list of parameters for argument 2 of `" +
                     FFIBuiltins.Invoke + "`.");
    }

    const auto& funcParams = args.at(1).getList()->elements;

    return ffi.invokeFunction(token, funcAlias, funcParams);
  }

  static KValue executeLoad(FFIManager& ffi, const Token& token,
                            const std::vector<KValue>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FFIBuiltins.Load);
    }

    auto libAlias = get_string(token, args.at(0));
    auto libPath = get_string(token, args.at(1));

    ffi.loadLibrary(token, libAlias, libPath);

    return {};
  }

  static KValue executeUnload(FFIManager& ffi, const Token& token,
                              const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FFIBuiltins.Unload);
    }

    auto libAlias = get_string(token, args.at(0));

    ffi.unloadLibrary(token, libAlias);

    return {};
  }
};

#endif