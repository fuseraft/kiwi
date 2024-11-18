#ifndef KIWI_FFI_FFIMANAGER_H
#define KIWI_FFI_FFIMANAGER_H

#include <dlfcn.h>
#include <string>
#include <unordered_map>
#include <vector>
#include <stdexcept>
#include <iostream>
#include <variant>
#include <memory>

#include "parsing/tokens.h"
#include "tracing/error.h"
#include "typing/value.h"
#include "marshaledarg.h"

// FFIManager class
class FFIManager {
 private:
  struct FFIEntry {
    void* functionPtr;
    k_string signature;
  };

  std::unordered_map<k_string, FFIEntry> ffiRegistry;
  std::unordered_map<k_string, void*> loadedLibraries;

  // Helper to parse signature and convert arguments
  std::vector<k_string> parseSignature(const k_string& signature) {
    std::vector<k_string> parts;
    std::istringstream stream(signature);
    k_string token;

    while (std::getline(stream, token, ',')) {
      parts.push_back(token);
    }

    return parts;
  }

 public:
  // Load a shared library as a package.
  void loadLibrary(const Token& token, const k_string& libPath,
                   const k_string& alias) {
    if (loadedLibraries.count(alias)) {
      throw FFIError(token, "Library alias already exists: " + alias);
    }

    void* handle = dlopen(libPath.c_str(), RTLD_LAZY);
    if (!handle) {
      throw std::runtime_error("Failed to load library: " +
                               k_string(dlerror()));
    }

    loadedLibraries[alias] = handle;
  }

  // Attach a function to an aliased package.
  void attachFunction(const Token& token, const k_string& kiwiName,
                      const k_string& ffiName, const k_string& signature,
                      const k_string& libraryAlias = "") {
    void* handle = nullptr;

    if (!libraryAlias.empty()) {
      if (!loadedLibraries.count(libraryAlias)) {
        throw FFIError(token, "Library alias not found: " + libraryAlias);
      }
      handle = loadedLibraries[libraryAlias];
    }

    void* functionPtr = dlsym(handle ? handle : RTLD_DEFAULT, ffiName.c_str());
    if (!functionPtr) {
      throw FFIError(token, "Function not found: " + ffiName);
    }

    ffiRegistry[kiwiName] = {functionPtr, signature};
  }

  k_value callFunction(const Token& token, const k_string& kiwiName,
                       const std::vector<k_value>& args) {
    if (!ffiRegistry.count(kiwiName)) {
      throw FFIError(token, "FFI function not registered: " + kiwiName);
    }

    const auto& entry = ffiRegistry[kiwiName];
    void* functionPtr = entry.functionPtr;
    std::vector<k_string> signatureParts = parseSignature(entry.signature);

    if (signatureParts.size() > 1 && signatureParts.size() - 1 != args.size()) {
      throw FFIError(token,
                     "Argument count mismatch for function: " + kiwiName);
    }

    // Dynamically prepare arguments
    std::vector<MarshaledArg>
        marshaledValues;  // Collect raw pointers to arguments
    std::vector<std::unique_ptr<char[]>> stringStorage;
    marshalParameters(args, signatureParts, token, kiwiName, marshaledValues,
                      stringStorage);

    return invokeFFI(signatureParts, functionPtr, marshaledValues, token);
  }

  k_value invokeFFI(std::vector<k_string>& signatureParts, void* functionPtr,
                    std::vector<MarshaledArg>& marshaledValues,
                    const Token& token) {
    auto invokeVariadic = [&](auto func,
                              const std::vector<MarshaledArg>& values) -> auto {
      switch (values.size()) {
        case 0:
          return func();
        case 1:
          return func(values[0].getValue());
        case 2:
          return func(values[0].getValue(), values[1].getValue());
        case 3:
          return func(values[0].getValue(), values[1].getValue(),
                      values[2].getValue());
        case 4:
          return func(values[0].getValue(), values[1].getValue(),
                      values[2].getValue(), values[3].getValue());
        default:
          throw FFIError(token, "Too many arguments for function call.");
      }
    };

    // Determine return type and invoke function
    const k_string& returnType = signatureParts.back();

    if (returnType == "int") {
      using FuncType = int (*)(...);
      auto func = reinterpret_cast<FuncType>(functionPtr);
      int result = invokeVariadic(func, marshaledValues);
      return static_cast<k_int>(result);
    } else if (returnType == "double") {
      using FuncType = double (*)(...);
      auto func = reinterpret_cast<FuncType>(functionPtr);
      double result = invokeVariadic(func, marshaledValues);
      return result;
    } else if (returnType == "pointer") {
      using FuncType = void* (*)(...);
      auto func = reinterpret_cast<FuncType>(functionPtr);
      void* result = invokeVariadic(func, marshaledValues);
      return k_pointer(result);
    } else if (returnType == "string") {
      using FuncType = const char* (*)(...);
      auto func = reinterpret_cast<FuncType>(functionPtr);
      const char* result = invokeVariadic(func, marshaledValues);

      // Convert to k_string, handle null pointers
      return k_string(result ? result : "");
    } else if (returnType == "void") {
      using FuncType = void (*)(...);
      auto func = reinterpret_cast<FuncType>(functionPtr);
      invokeVariadic(func, marshaledValues);

      // No value for void functions
      return {};
    }

    throw FFIError(token, "Unsupported return type: " + returnType);
  }

  void marshalParameters(const std::vector<k_value>& args,
                         std::vector<k_string>& signatureParts,
                         const Token& token, const k_string& kiwiName,
                         std::vector<MarshaledArg>& marshaledValues,
                         std::vector<std::unique_ptr<char[]>>& stringStorage) {
    for (size_t i = 0; i < args.size(); ++i) {
      const k_string& type = signatureParts[i];
      if (type == "int") {
        if (!std::holds_alternative<k_int>(args[i])) {
          throw FFIError(token, "Expected an integer in argument " +
                                    std::to_string(1 + i) + " of `" + kiwiName +
                                    "`.");
        }

        marshaledValues.push_back(
            MarshaledArg(ArgType::Int, new int(std::get<k_int>(args[i]))));
      } else if (type == "double") {
        if (!std::holds_alternative<double>(args[i])) {
          throw FFIError(token, "Expected a double in argument " +
                                    std::to_string(1 + i) + " of `" + kiwiName +
                                    "`.");
        }

        marshaledValues.push_back(MarshaledArg(
            ArgType::Double, new double(std::get<double>(args[i]))));
      } else if (type == "pointer") {
        if (!std::holds_alternative<k_pointer>(args[i])) {
          throw FFIError(token, "Expected a pointer in argument " +
                                    std::to_string(1 + i) + " of `" + kiwiName +
                                    "`.");
        }

        marshaledValues.push_back(
            MarshaledArg(ArgType::Pointer, std::get<k_pointer>(args[i]).ptr));
      } else if (type == "size_t") {
        if (!std::holds_alternative<k_int>(args[i])) {
          throw FFIError(token, "Expected an integer in argument " +
                                    std::to_string(1 + i) + " of `" + kiwiName +
                                    "`.");
        }

        marshaledValues.push_back(MarshaledArg(
            ArgType::SizeT,
            new size_t(static_cast<size_t>(std::get<k_int>(args[i])))));
      } else if (type == "string") {
        if (!std::holds_alternative<k_string>(args[i])) {
          throw FFIError(token, "Expected a string in argument " +
                                    std::to_string(1 + i) + " of `" + kiwiName +
                                    "`.");
        }

        const auto& str = std::get<k_string>(args[i]);
        stringStorage.emplace_back(new char[str.size() + 1]);
        std::strcpy(stringStorage.back().get(), str.c_str());
        marshaledValues.push_back(
            MarshaledArg(ArgType::String, stringStorage.back().release()));
      } else if (type == "string[]") {
        if (!std::holds_alternative<k_list>(args[i])) {
          throw FFIError(token, "Expected a list of strings in argument " +
                                    std::to_string(1 + i) + " of `" + kiwiName +
                                    "`.");
        }

        const auto& inputList = std::get<k_list>(args[i])->elements;
        std::vector<k_string> inputStrings;
        inputStrings.reserve(inputList.size());

        for (const auto& e : inputList) {
          if (!std::holds_alternative<k_string>(e)) {
            throw FFIError(token, "Expected a list of strings in argument " +
                                      std::to_string(1 + i) + " of `" +
                                      kiwiName + "`.");
          }

          inputStrings.emplace_back(std::get<k_string>(e));
        }

        marshaledValues.push_back(
            MarshaledArg::createStringArray(inputStrings));
      } else {
        throw FFIError(token,
                       "Unsupported argument type in signature: " + type);
      }
    }
  }

  void unloadLibrary(const Token& token, const k_string& name) {
    if (!loadedLibraries.count(name)) {
      throw FFIError(token, "Library alias not found: " + name);
    }

    for (auto& [alias, handle] : loadedLibraries) {
      if (name != alias) {
        continue;
      }

      dlclose(handle);
    }

    loadedLibraries.erase(name);
  }

  // Unload all libraries
  void unloadLibraries() {
    for (auto& [alias, handle] : loadedLibraries) {
      dlclose(handle);
    }

    loadedLibraries.clear();
  }

  ~FFIManager() { unloadLibraries(); }
};

#endif