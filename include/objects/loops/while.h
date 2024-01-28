#ifndef KIWI_OBJECTS_LOOPS_WHILE_H
#define KIWI_OBJECTS_LOOPS_WHILE_H

#include <vector>
#include "parsing/tokens.h"

class WhileLoop {
 public:
  void addConditionToken(Token& t) { condition.push_back(t); }

  void addToken(Token& t) { code.push_back(t); }

  std::vector<Token> getCondition() { return condition; }

  std::vector<Token> getCode() { return code; }

 private:
  std::vector<Token> condition;
  std::vector<Token> code;
};

#endif