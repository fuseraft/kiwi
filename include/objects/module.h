#ifndef KIWI_OBJECTS_MODULE_H
#define KIWI_OBJECTS_MODULE_H

#include <string>
#include <vector>
#include "parsing/tokens.h"

class Module {
 public:
  void addToken(Token& t) { code.push_back(t); }
  void setName(const std::string& name) { _name = name; }

  std::vector<Token> getCode() const { return code; }
  std::string getName() { return _name; }

 private:
  std::vector<Token> code;
  std::string _name;
};

#endif