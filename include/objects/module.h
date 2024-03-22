#ifndef ASTRAL_OBJECTS_MODULE_H
#define ASTRAL_OBJECTS_MODULE_H

#include <string>
#include <vector>
#include "parsing/tokens.h"
#include "typing/value.h"

class Module {
 public:
  void addToken(const Token& t) { code.push_back(t); }
  void setName(const k_string& name) { _name = name; }
  void setHome(const k_string& home) {
    _home = home;
    _hasHome = true;
  }

  const std::vector<Token>& getCode() const { return code; }
  const k_string& getName() { return _name; }
  const k_string& getHome() { return _home; }
  bool hasHome() const { return _hasHome; }

 private:
  std::vector<Token> code;
  k_string _name;
  k_string _home;
  bool _hasHome = false;
};

#endif