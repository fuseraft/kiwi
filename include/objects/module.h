#ifndef KIWI_OBJECTS_MODULE_H
#define KIWI_OBJECTS_MODULE_H

#include <string>
#include <vector>
#include "parsing/tokens.h"

class Module {
 public:
  void addToken(const Token& t) { code.push_back(t); }
  void setName(std::string name) { _name = name; }
  void setHome(std::string home) {
    _home = home;
    _hasHome = true;
  }

  const std::vector<Token>& getCode() const { return code; }
  const std::string& getName() { return _name; }
  const std::string& getHome() { return _home; }
  bool hasHome() const { return _hasHome; }

 private:
  std::vector<Token> code;
  std::string _name;
  std::string _home;
  bool _hasHome = false;
};

#endif