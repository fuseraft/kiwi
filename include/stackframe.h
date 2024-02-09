#ifndef KIWI_STACKFRAME_H
#define KIWI_STACKFRAME_H

#include <memory>
#include <map>
#include <string>
#include <vector>
#include "errors/error.h"
#include "errors/state.h"
#include "parsing/tokens.h"
#include "typing/valuetype.h"

struct CallStackFrame {
  std::vector<Token> tokens;  // The tokens of the current method or scope.
  size_t position = 0;        // Current position in the token stream.
  std::map<std::string, Value> variables;
  Value returnValue;
  ErrorState errorState;
  std::shared_ptr<Object> objectContext;

  bool returnFlag = false;
  bool subFrame = false;
  bool loopBreak = false;
  bool loopContinue = false;
  bool inTry = false;
  bool inObject = false;

  CallStackFrame(const std::vector<Token>& tokens) : tokens(tokens) {}

  // Methods to control the flags
  void setBreak() { loopBreak = true; }
  void clearBreak() { loopBreak = false; }
  bool isBreakSet() const { return loopBreak; }

  void setTry() { inTry = true; }
  void clearTry() { inTry = false; }
  bool isInTry() const { return inTry; }

  void setErrorState(const KiwiError& e) { errorState.setError(e); }
  bool isErrorStateSet() const { return errorState.isErrorSet(); }
  ErrorState getErrorState() const { return errorState; }
  std::string getErrorMessage() const { return errorState.error.getMessage(); }
  void clearErrorState() { errorState.clearError(); }

  void setObjectContext(const std::shared_ptr<Object>& object) {
    objectContext = object;
    inObject = true;
  }
  bool inObjectContext() const { return inObject; }
  std::shared_ptr<Object>& getObjectContext() { return objectContext; }

  void setContinue() { loopContinue = true; }
  void clearContinue() { loopContinue = false; }
  bool isContinueSet() const { return loopContinue; }

  void setSubFrame() { subFrame = true; }
  void setReturnFlag() { returnFlag = true; }
  bool isSubFrame() const { return subFrame; }
  bool isReturnFlagSet() const { return returnFlag; }
};

#endif