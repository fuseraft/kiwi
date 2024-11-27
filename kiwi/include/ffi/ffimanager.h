#ifndef KIWI_FFI_FFIMANAGER_H
#define KIWI_FFI_FFIMANAGER_H

#include <cstring>
#include <dlfcn.h>
#include <ffi.h>
#include <iostream>
#include <memory>
#include <mutex>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

#include "parsing/tokens.h"
#include "tracing/error.h"
#include "typing/value.h"

// Function information structure
struct FunctionInfo {
  void* funcPtr;
  std::vector<k_string> parameters;
  k_string returnType;
  k_string libraryAlias;
};

struct Allocation {
  void* ptr;
  std::function<void(void*)> deleter;
};

class FFIManager {
 public:
  // Load a shared library and associate it with an alias
  void loadLibrary(const Token& token, const k_string& libAlias,
                   const k_string& path);

  // Attach a function from a library and store its signature
  bool attachFunction(const Token& token, const k_string& libAlias,
                      const k_string& funcAlias, const k_string& funcName,
                      const k_list& parameters, const k_string& returnType);

  // Invoke a function by its alias with the provided arguments
  k_value invokeFunction(const Token& token, const k_string& funcAlias,
                         const std::vector<k_value>& args);

  // Unload a library by its alias and clean up resources
  void unloadLibrary(const Token& token, const k_string& libAlias);

  // Clean up resources
  ~FFIManager();

 private:
  std::unordered_map<k_string, void*> libraryHandles;
  std::unordered_map<k_string, FunctionInfo> functionRegistry;
  std::mutex managerMutex;

  std::vector<k_string> parseSignature(const k_string& signature);

  ffi_type* getFFIType(const Token& token, const k_string& typeStr);

  void prepareArguments(const Token& token,
                        const std::vector<k_string>& paramTypes,
                        const std::vector<k_value>& args,
                        std::vector<ffi_type*>& argTypes,
                        std::vector<void*>& argValues,
                        std::vector<Allocation>& allocations);

  void* callFunction(const Token& token, void* funcPtr,
                     std::vector<ffi_type*>& argTypes, ffi_type* retType,
                     std::vector<void*>& argValues);

  k_value processReturnValue(const Token& token, void* retVal,
                             const k_string& retTypeStr);

  void cleanUpAllocations(std::vector<Allocation>& allocations);
};

void FFIManager::loadLibrary(const Token& token, const k_string& libAlias,
                             const k_string& path) {
  std::lock_guard<std::mutex> lock(managerMutex);

  void* handle = dlopen(path.c_str(), RTLD_LAZY);
  if (!handle) {
    throw FFIError(token, k_string("dlopen failed @ (alias=" + libAlias +
                                   ",path=" + path + "): ") +
                              dlerror());
  }

  libraryHandles[libAlias] = handle;
}

bool FFIManager::attachFunction(const Token& token, const k_string& libAlias,
                                const k_string& funcAlias,
                                const k_string& funcName,
                                const k_list& parameters,
                                const k_string& returnType) {
  std::lock_guard<std::mutex> lock(managerMutex);

  if (!libraryHandles.count(libAlias)) {
    throw FFIError(token, "The library alias is not loaded: " + libAlias);
  }

  void* handle = libraryHandles.at(libAlias);
  void* funcPtr = dlsym(handle, funcName.c_str());

  if (!funcPtr) {
    throw FFIError(token, k_string("dlsym failed @ (lib_alias=" + libAlias +
                                   ",func_alias=" + funcAlias +
                                   ",func_name=" + funcName + "): ") +
                              dlerror());
  }

  const auto& funcParameters = parameters->elements;
  std::vector<k_string> parameterTypes;
  parameterTypes.reserve(funcParameters.size());

  for (const auto& funcParam : funcParameters) {
    if (!std::holds_alternative<k_string>(funcParam)) {
      throw FFIError(
          token, "Invalid type string in function parameter information for `" +
                     funcAlias + "`.");
    }

    parameterTypes.push_back(std::get<k_string>(funcParam));
  }

  functionRegistry[funcAlias] = {funcPtr, parameterTypes, returnType, libAlias};

  return true;
}

k_value FFIManager::invokeFunction(const Token& token,
                                   const k_string& funcAlias,
                                   const std::vector<k_value>& args) {
  std::lock_guard<std::mutex> lock(managerMutex);

  if (!functionRegistry.count(funcAlias)) {
    throw FFIError(token, "Function alias is not registered: " + funcAlias);
  }

  FunctionInfo& funcInfo = functionRegistry.at(funcAlias);
  std::vector<k_string> paramTypes = funcInfo.parameters;

  k_string retTypeStr = funcInfo.returnType;

  if (paramTypes.size() != args.size()) {
    throw FFIError(
        token,
        "Argument count does not match function signature @ (func_alias=" +
            funcAlias + ")");
  }

  // Prepare ffi_type arrays and argument values
  std::vector<ffi_type*> argTypes;
  std::vector<void*> argValues;
  std::vector<Allocation>
      allocations;  // Keep track of allocated memory for cleanup

  prepareArguments(token, paramTypes, args, argTypes, argValues, allocations);

  ffi_type* retType = getFFIType(token, retTypeStr);

  // Invoke the function
  void* retVal =
      callFunction(token, funcInfo.funcPtr, argTypes, retType, argValues);

  // Process the return value
  k_value result = processReturnValue(token, retVal, retTypeStr);

  // Cleanup
  cleanUpAllocations(allocations);
  if (retVal && retType != &ffi_type_void) {
    free(retVal);
  }

  return result;
}

void FFIManager::unloadLibrary(const Token& token, const k_string& libAlias) {
  std::lock_guard<std::mutex> lock(managerMutex);

  // Close the library handle
  auto it = libraryHandles.find(libAlias);
  if (it != libraryHandles.end()) {
    if (dlclose(it->second) != 0) {
      throw FFIError(token, k_string("dlclose failed for alias: ") + libAlias +
                                ": " + dlerror());
    }
    libraryHandles.erase(it);
  } else {
    throw FFIError(token, "Library alias not found: " + libAlias);
  }

  // Remove any functions associated with this library
  for (auto funcIt = functionRegistry.begin();
       funcIt != functionRegistry.end();) {
    if (funcIt->second.libraryAlias == libAlias) {
      funcIt = functionRegistry.erase(funcIt);
    } else {
      ++funcIt;
    }
  }
}

std::vector<k_string> FFIManager::parseSignature(const k_string& signature) {
  std::vector<k_string> types;
  std::stringstream ss(signature);
  k_string type;
  while (std::getline(ss, type, ',')) {
    // Trim whitespace
    type.erase(0, type.find_first_not_of(" \t"));
    type.erase(type.find_last_not_of(" \t") + 1);
    types.push_back(type);
  }
  return types;
}

ffi_type* FFIManager::getFFIType(const Token& token, const k_string& typeStr) {
  if (typeStr == "int") {
    return &ffi_type_sint;
  } else if (typeStr == "size_t") {
    return &ffi_type_uint32;
  } else if (typeStr == "double") {
    return &ffi_type_double;
  } else if (typeStr == "bool") {
    return &ffi_type_uint8;
  } else if (typeStr == "pointer") {
    return &ffi_type_pointer;
  } else if (typeStr == "string") {
    return &ffi_type_pointer;  // char*
  } else if (typeStr == "string[]") {
    return &ffi_type_pointer;
  } else if (typeStr == "void") {
    return &ffi_type_void;
  }

  throw FFIError(token, "Unsupported type: " + typeStr);
}

void FFIManager::prepareArguments(const Token& token,
                                  const std::vector<k_string>& paramTypes,
                                  const std::vector<k_value>& args,
                                  std::vector<ffi_type*>& argTypes,
                                  std::vector<void*>& argValues,
                                  std::vector<Allocation>& allocations) {
  try {
    for (size_t i = 0; i < paramTypes.size(); ++i) {
      const k_string& typeStr = paramTypes[i];
      const k_value& arg = args[i];

      ffi_type* ffiType = getFFIType(token, typeStr);
      argTypes.push_back(ffiType);

      void* argValue = nullptr;

      try {
        if (typeStr == "int") {
          int* value = new int(static_cast<int>(std::get<k_int>(arg)));
          argValue = value;
          allocations.push_back({value, [](void* p) {
                                   delete static_cast<int*>(p);
                                 }});
        } else if (typeStr == "size_t") {
          size_t* value = new size_t(static_cast<size_t>(std::get<k_int>(arg)));
          argValue = value;
          allocations.push_back({value, [](void* p) {
                                   delete static_cast<size_t*>(p);
                                 }});
        } else if (typeStr == "double") {
          double* value = new double(std::get<double>(arg));
          argValue = value;
          allocations.push_back({value, [](void* p) {
                                   delete static_cast<double*>(p);
                                 }});
        } else if (typeStr == "bool") {
          bool* value = new bool(std::get<bool>(arg));
          argValue = value;
          allocations.push_back({value, [](void* p) {
                                   delete static_cast<bool*>(p);
                                 }});
        } else if (typeStr == "pointer") {
          void** value = new void*(std::get<k_pointer>(arg).ptr);
          argValue = value;
          allocations.push_back({value, [](void* p) {
                                   delete static_cast<void**>(p);
                                 }});
        } else if (typeStr == "string") {
          const k_string& str = std::get<k_string>(arg);
          char* cstr = new char[str.size() + 1];
          std::strcpy(cstr, str.c_str());
          argValue = &cstr;
          allocations.push_back({cstr, [](void* p) {
                                   delete[] static_cast<char*>(p);
                                 }});
        } else if (typeStr == "string[]") {
          const std::vector<k_value>& strArray =
              std::get<k_list>(arg)->elements;

          size_t arraySize = strArray.size();
          char** cstrArray =
              new char*[arraySize + 1];  // +1 for NULL terminator if needed

          for (size_t j = 0; j < arraySize; ++j) {
            const k_string& str = std::get<k_string>(strArray[j]);
            char* cstr = new char[str.size() + 1];
            std::strcpy(cstr, str.c_str());
            cstrArray[j] = cstr;

            allocations.push_back({cstr, [](void* p) {
                                     delete[] static_cast<char*>(p);
                                   }});
          }

          cstrArray[arraySize] = nullptr;

          allocations.push_back({cstrArray, [](void* p) {
                                   delete[] static_cast<char**>(p);
                                 }});

          argValue = &cstrArray;
        } else {
          throw FFIError(token, "Unsupported parameter type: " + typeStr);
        }
      } catch (const std::bad_variant_access&) {
        throw FFIError(token, "Argument type mismatch at position " +
                                  std::to_string(i) + " for parameter type '" +
                                  typeStr + "'");
      } catch (const std::exception& ex) {
        throw FFIError(token, "Error preparing argument at position " +
                                  std::to_string(i) + ": " + ex.what());
      }

      argValues.push_back(argValue);
    }
  } catch (...) {
    cleanUpAllocations(allocations);
    throw;
  }
}

void* FFIManager::callFunction(const Token& token, void* funcPtr,
                               std::vector<ffi_type*>& argTypes,
                               ffi_type* retType,
                               std::vector<void*>& argValues) {
  ffi_cif cif;
  if (ffi_prep_cif(&cif, FFI_DEFAULT_ABI,
                   static_cast<unsigned int>(argTypes.size()), retType,
                   argTypes.data()) != FFI_OK) {
    throw FFIError(token, "ffi_prep_cif failed");
  }

  void* retVal = nullptr;
  if (retType != &ffi_type_void) {
    retVal = malloc(retType->size);
    if (!retVal) {
      throw FFIError(token, "Memory allocation failed for return value");
    }
  }

  ffi_call(&cif, FFI_FN(funcPtr), retVal, argValues.data());

  return retVal;
}

k_value FFIManager::processReturnValue(const Token& token, void* retVal,
                                       const k_string& retTypeStr) {
  if (retTypeStr == "void") {
    return {};
  }

  if (!retVal) {
    throw FFIError(token, "Return value is null for type: " + retTypeStr);
  }

  if (retTypeStr == "int") {
    int value = *static_cast<int*>(retVal);
    return static_cast<k_int>(value);
  } else if (retTypeStr == "size_t") {
    size_t value = *static_cast<size_t*>(retVal);
    return static_cast<k_int>(value);
  } else if (retTypeStr == "double") {
    double value = *static_cast<double*>(retVal);
    return value;
  } else if (retTypeStr == "bool") {
    bool value = *static_cast<bool*>(retVal);
    return value;
  } else if (retTypeStr == "pointer") {
    void* value = *static_cast<void**>(retVal);
    return k_pointer(value);
  } else if (retTypeStr == "string") {
    char* value = *static_cast<char**>(retVal);
    if (!value) {
      throw FFIError(token, "String return value is null");
    }
    return k_string(value);
  } else if (retTypeStr == "string[]") {
    char** cstrArray = *static_cast<char***>(retVal);
    std::vector<k_value> strArray;

    if (!cstrArray) {
      throw FFIError(token, "Returned string array is null");
    }

    size_t index = 0;
    while (cstrArray[index] != nullptr) {
      char* cstr = cstrArray[index];
      k_string str(cstr);
      strArray.push_back(str);
      ++index;
    }

    return std::make_shared<List>(strArray);
  } else {
    throw FFIError(token, "Unsupported return type: " + retTypeStr);
  }
}

void FFIManager::cleanUpAllocations(std::vector<Allocation>& allocations) {
  for (const auto& alloc : allocations) {
    alloc.deleter(alloc.ptr);
  }
}

FFIManager::~FFIManager() {
  // Close all loaded libraries
  for (auto& pair : libraryHandles) {
    dlclose(pair.second);
  }
  libraryHandles.clear();
  functionRegistry.clear();
}

#endif