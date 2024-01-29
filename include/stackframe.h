#ifndef KIWI_STACKFRAME_H
#define KIWI_STACKFRAME_H

#include <map>
#include <string>
#include <variant>
#include <vector>
#include "parsing/tokens.h"

struct CallStackFrame {
  std::vector<Token> tokens;  // The tokens of the current method or scope.
  size_t position = 0;        // Current position in the token stream.
  std::map<std::string, std::variant<int, double, bool, std::string>> variables;
  std::variant<int, double, bool, std::string> returnValue;

  CallStackFrame(const std::vector<Token>& tokens) : tokens(tokens) {}
};

#endif