#ifndef KIWI_FFI_MARSHALEDARG_H
#define KIWI_FFI_MARSHALEDARG_H

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
#include <utility>

// Enumeration for argument types
enum class ArgType { Int, SizeT, Double, Pointer, String, StringArray };

// MarshaledArg class to manage FFI arguments safely
class MarshaledArg {
 public:
  template <typename T>
  MarshaledArg(ArgType t, T* v)
      : type(t), value(static_cast<void*>(v), [](void* ptr) {
          if (ptr) {
            delete static_cast<T*>(ptr);
          }
        }) {}

  // Constructor for raw pointers (ArgType::Pointer)
  MarshaledArg(ArgType t, void* v) : type(t), value(v, [](void*) {}) {}

  // Constructor for string arrays (string[])
  static MarshaledArg createStringArray(
      const std::vector<std::string>& strings) {
    const char** argvArray = new const char*[strings.size() + 1];
    for (size_t i = 0; i < strings.size(); ++i) {
      argvArray[i] = strings[i].c_str();  // No allocation, just references
    }
    argvArray[strings.size()] = nullptr;  // Null-terminate the array

    return MarshaledArg(ArgType::StringArray, argvArray, [](void* ptr) {
      delete[] static_cast<const char**>(ptr);
    });
  }

  // Move constructor
  MarshaledArg(MarshaledArg&& other) noexcept
      : type(other.type), value(std::move(other.value)) {}

  // Move assignment
  MarshaledArg& operator=(MarshaledArg&& other) noexcept {
    if (this != &other) {
      type = other.type;
      value = std::move(other.value);
    }
    return *this;
  }

  // Deleted copy constructor and assignment operator
  MarshaledArg(const MarshaledArg&) = delete;
  MarshaledArg& operator=(const MarshaledArg&) = delete;

  // Getters
  ArgType getType() const { return type; }
  void* getValue() const { return value.get(); }

 private:
  // Private constructor for specialized use cases
  MarshaledArg(ArgType t, void* v, void (*deleter)(void*))
      : type(t), value(v, deleter) {}

  ArgType type;
  std::unique_ptr<void, void (*)(void*)> value;
};

#endif
