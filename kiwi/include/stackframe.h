#ifndef KIWI_STACKFRAME_H
#define KIWI_STACKFRAME_H

#include <memory>
#include <string>
#include <type_traits>
#include <unordered_map>
#include "objects/method.h"
#include "parsing/tokens.h"
#include "tracing/error.h"
#include "tracing/state.h"
#include "typing/value.h"

enum class FrameFlags : uint8_t {
  None = 0,
  ReturnFlag = 1 << 0,
  SubFrame = 1 << 1,
  LoopBreak = 1 << 2,
  LoopContinue = 1 << 3,
  InTry = 1 << 4,
  InObject = 1 << 5,
  InLoop = 1 << 6,
  InCall = 1 << 7
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
  std::unordered_map<k_string, Method> lambdas;
  std::vector<k_string> aliases;
  k_value returnValue;
  ErrorState errorState;
  k_object objectContext;
  FrameFlags flags = FrameFlags::None;

  CallStackFrame() {}
  ~CallStackFrame() {
    variables.clear();
    lambdas.clear();
    aliases.clear();
  }

  void assignLambda(const k_string& name, const Method& method) {
    lambdas[name] = std::move(method);
  }
  bool hasAssignedLambda(const k_string& name) const {
    return lambdas.find(name) != lambdas.end();
  }
  Method& getAssignedLambda(const k_string& name) { return lambdas[name]; }

  bool hasVariable(const k_string& name) const {
    return variables.find(name) != variables.end();
  }

  void setErrorState(const ErrorState& e) { errorState = e; }
  void setErrorState(const KiwiError& e) { errorState.setError(e); }
  bool isErrorStateSet() const { return errorState.isErrorSet(); }
  k_string getErrorMessage() const { return errorState.error.getMessage(); }
  ErrorState getErrorState() const { return errorState; }
  void clearErrorState() { errorState.clearError(); }

  void setObjectContext(const k_object& object) {
    objectContext = object;
    setFlag(FrameFlags::InObject);
  }
  bool inObjectContext() const { return isFlagSet(FrameFlags::InObject); }
  k_object& getObjectContext() { return objectContext; }

  void setFlag(FrameFlags flag) { flags = flags | flag; }
  void clearFlag(FrameFlags flag) { flags = flags & ~flag; }
  bool isFlagSet(FrameFlags flag) const { return (flags & flag) == flag; }
  bool isLoopControlFlagSet() const {
    return isFlagSet(FrameFlags::LoopBreak) ||
           isFlagSet(FrameFlags::LoopContinue);
  }
};

#endif