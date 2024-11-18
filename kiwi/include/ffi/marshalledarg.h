#ifndef KIWI_FFI_MARSHALLEDARG_H
#define KIWI_FFI_MARSHALLEDARG_H

struct MarshaledArg {
  enum class ArgType { Int, Double, Pointer } type;
  void* value;

  MarshaledArg(ArgType t, void* v) : type(t), value(v) {}

  ~MarshaledArg() {
    switch (type) {
      case ArgType::Int:
        delete static_cast<int*>(value);
        break;
      case ArgType::Double:
        delete static_cast<double*>(value);
        break;
      case ArgType::Pointer:
        // Do not delete raw pointers passed to the FFI call
        break;
    }
  }
};

#endif