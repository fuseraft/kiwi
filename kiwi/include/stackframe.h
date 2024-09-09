#ifndef KIWI_STACKFRAME_H
#define KIWI_STACKFRAME_H

#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include "parsing/tokens.h"
#include "tracing/error.h"
#include "tracing/state.h"
#include "typing/value.h"

enum class FrameFlags : uint16_t {
  None = 0,
  Return = 1 << 0,
  SubFrame = 1 << 1,
  InLoop = 1 << 2,
  Break = 1 << 3,
  Next = 1 << 4,
  InTry = 1 << 5,
  InObject = 1 << 6,
};

inline FrameFlags operator|(FrameFlags a, FrameFlags b) {
  return static_cast<FrameFlags>(
      static_cast<std::underlying_type<FrameFlags>::type>(a) |
      static_cast<std::underlying_type<FrameFlags>::type>(b));
}
inline FrameFlags operator&(FrameFlags a, FrameFlags b) {
  return static_cast<FrameFlags>(
      static_cast<std::underlying_type<FrameFlags>::type>(a) &
      static_cast<std::underlying_type<FrameFlags>::type>(b));
}
inline FrameFlags operator~(FrameFlags a) {
  return static_cast<FrameFlags>(
      ~static_cast<std::underlying_type_t<FrameFlags>>(a));
}

struct CallStackFrame {
  std::unordered_map<k_string, k_value> variables;
  k_value returnValue;
  k_object objectContext;
  FrameFlags flags = FrameFlags::None;

  CallStackFrame() {}
  ~CallStackFrame() { variables.clear(); }

  bool hasVariable(const k_string& name) const {
    return variables.find(name) != variables.end();
  }

  void setObjectContext(const k_object& object) {
    objectContext = object;
    setFlag(FrameFlags::InObject);
  }
  bool inObjectContext() const { return isFlagSet(FrameFlags::InObject); }
  k_object& getObjectContext() { return objectContext; }

  void setFlag(FrameFlags flag) { flags = flags | flag; }
  void clearFlag(FrameFlags flag) { flags = flags & ~flag; }
  bool isFlagSet(FrameFlags flag) const { return (flags & flag) == flag; }
};

#endif