#ifndef KIWI_OBJECTS_METHOD_H
#define KIWI_OBJECTS_METHOD_H

#include <vector>
#include "parsing/tokens.h"

class Method {
 public:
  void addToken(Token& t) { code.push_back(t); }
  void addParameterToken(Token& t) { parameters.push_back(t); }
  void setName(const std::string& name) { _name = name; }

  std::vector<Token> getCode() { return code; }
  std::vector<Token> getParameters() { return parameters; }
  std::string getName() { return _name; }

 private:
  std::vector<Token> parameters;
  std::vector<Token> code;
  std::string _name;
};

#endif