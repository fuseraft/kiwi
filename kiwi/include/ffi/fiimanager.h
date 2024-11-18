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
#include "marshalledarg.h"

// FFIManager class
class FFIManager {
 private:
  struct FFIEntry {
    void* functionPtr;
    std::string signature;
  };

  std::unordered_map<std::string, FFIEntry> ffiRegistry;
  std::unordered_map<std::string, void*> loadedLibraries;

  // Helper to parse signature and convert arguments
  std::vector<std::string> parseSignature(const std::string& signature) {
    std::vector<std::string> parts;
    std::istringstream stream(signature);
    std::string token;

    while (std::getline(stream, token, ',')) {
      parts.push_back(token);
    }

    return parts;
  }

  template <typename T>
  T convertArgument(const k_value& arg) {
    if constexpr (std::is_same_v<T, k_int>) {
      return std::get<k_int>(arg);
    } else if constexpr (std::is_same_v<T, double>) {
      return std::get<double>(arg);
    } else if constexpr (std::is_same_v<T, k_pointer>) {
      return std::get<k_pointer>(arg);
    } else {
      throw std::runtime_error("Unsupported argument type");
    }
  }

 public:
  // Load a shared library as a package.
  void loadLibrary(const Token& token, const std::string& libPath,
                   const std::string& alias) {
    if (loadedLibraries.count(alias)) {
      throw FFIError(token, "Library alias already exists: " + alias);
    }

    void* handle = dlopen(libPath.c_str(), RTLD_LAZY);
    if (!handle) {
      throw std::runtime_error("Failed to load library: " +
                               std::string(dlerror()));
    }

    loadedLibraries[alias] = handle;
  }

  // Attach a function to an aliased package.
  void attachFunction(const Token& token, const std::string& kiwiName,
                      const std::string& ffiName, const std::string& signature,
                      const std::string& libraryAlias = "") {
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

  // Call a registered FFI function dynamically
  k_value callFunction(const Token& token, const std::string& kiwiName,
                       const std::vector<k_value>& args) {
    if (!ffiRegistry.count(kiwiName)) {
      throw FFIError(token, "FFI function not registered: " + kiwiName);
    }

    const auto& entry = ffiRegistry[kiwiName];
    void* functionPtr = entry.functionPtr;
    std::vector<std::string> signatureParts = parseSignature(entry.signature);

    if (signatureParts.size() - 1 != args.size()) {
      throw FFIError(token,
                     "Argument count mismatch for function: " + kiwiName);
    }

    // Dynamically prepare arguments
    std::vector<std::unique_ptr<MarshaledArg>> marshaledArgs;
    for (size_t i = 0; i < args.size(); ++i) {
      const std::string& type = signatureParts[i];
      if (type == "int") {
        marshaledArgs.emplace_back(new MarshaledArg(
            MarshaledArg::ArgType::Int, new int(std::get<k_int>(args[i]))));
      } else if (type == "double") {
        marshaledArgs.emplace_back(
            new MarshaledArg(MarshaledArg::ArgType::Double,
                             new double(std::get<double>(args[i]))));
      } else if (type == "pointer") {
        marshaledArgs.emplace_back(new MarshaledArg(
            MarshaledArg::ArgType::Pointer, std::get<k_pointer>(args[i]).ptr));
      } else {
        throw FFIError(token,
                       "Unsupported argument type in signature: " + type);
      }
    }

    // Determine return type
    const std::string& returnType = signatureParts.back();

    // Invoke function
    if (returnType == "int") {
      using FuncType = int (*)(...);
      int result = reinterpret_cast<FuncType>(functionPtr)(
          *static_cast<int*>(marshaledArgs[0]->value));
      return static_cast<k_int>(result);
    } else if (returnType == "double") {
      using FuncType = double (*)(...);
      double result = reinterpret_cast<FuncType>(functionPtr)(
          *static_cast<double*>(marshaledArgs[0]->value));
      return result;
    } else if (returnType == "pointer") {
      using FuncType = void* (*)(...);
      void* result =
          reinterpret_cast<FuncType>(functionPtr)(marshaledArgs[0]->value);
      return k_pointer(result);
    } else {
      throw FFIError(token, "Unsupported return type: " + returnType);
    }
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