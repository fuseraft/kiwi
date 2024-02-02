#ifndef KIWI_OBJECTS_METHOD_H
#define KIWI_OBJECTS_METHOD_H

#include <map>
#include <string>
#include <variant>
#include <vector>
#include "parsing/tokens.h"

class Method {
 public:
  void addToken(Token& t) { code.push_back(t); }
  void addParameterToken(Token& t) { parameters.push_back(t); }
  void setName(const std::string& name) { _name = name; }
  void addParameterValue(
      std::string paramName,
      std::variant<int, double, bool, std::string, std::shared_ptr<List>>
          value) {
    parameterKVP[paramName] = value;
  }
  bool hasParameter(std::string& paramName) {
    return parameterKVP.find(paramName) != parameterKVP.end();
  }
  std::variant<int, double, bool, std::string, std::shared_ptr<List>>
  getParameterValue(std::string& paramName) {
    return parameterKVP[paramName];
  }

  std::vector<Token> getCode() const { return code; }
  std::vector<Token> getParameters() const { return parameters; }
  std::string getName() { return _name; }

 private:
  std::vector<Token> parameters;
  std::vector<Token> code;
  std::string _name;
  std::map<std::string,
           std::variant<int, double, bool, std::string, std::shared_ptr<List>>>
      parameterKVP;
};

#endif