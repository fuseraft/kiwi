#ifndef KIWI_STACKFRAME_H
#define KIWI_STACKFRAME_H

#include <map>
#include <string>
#include <vector>
#include "parsing/tokens.h"
#include "typing/valuetype.h"

struct CallStackFrame {
  std::vector<Token> tokens;  // The tokens of the current method or scope.
  size_t position = 0;        // Current position in the token stream.
  std::map<std::string, Value> variables;
  Value returnValue;
  bool returnFlag = false;
  bool subFrame = false;
  bool loopBreak = false;
  bool loopContinue = false;

  CallStackFrame(const std::vector<Token>& tokens) : tokens(tokens) {}

  // Methods to control the flags
  void setBreak() { loopBreak = true; }
  void clearBreak() { loopBreak = false; }
  bool isBreakSet() const { return loopBreak; }

  void setContinue() { loopContinue = true; }
  void clearContinue() { loopContinue = false; }
  bool isContinueSet() const { return loopContinue; }

  void setSubFrame() { subFrame = true; }
  void setReturnFlag() { returnFlag = true; }
  bool isSubFrame() const { return subFrame; }
  bool isReturnFlagSet() const { return returnFlag; }
};

#endif