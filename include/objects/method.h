#ifndef KIWI_OBJECTS_METHOD_H
#define KIWI_OBJECTS_METHOD_H

#include <map>
#include <string>
#include <vector>
#include "parsing/tokens.h"
#include "typing/valuetype.h"

class Method {
 public:
  void addToken(Token t) { code.push_back(t); }
  void addParameterName(const std::string& s) { parameters.push_back(s); }
  void setName(const std::string& name) { _name = name; }
  void addParameterValue(std::string paramName, Value value) {
    parameterKVP[paramName] = value;
  }
  bool hasParameter(const std::string& paramName) const {
    return parameterKVP.find(paramName) != parameterKVP.end();
  }
  Value getParameterValue(const std::string& paramName) {
    return parameterKVP[paramName];
  }

  const std::vector<Token>& getCode() const { return code; }
  const std::vector<std::string>& getParameters() const { return parameters; }
  int getParameterCount() const { return parameters.size(); }
  std::string getName() const { return _name; }
  void setAbstract() { abstractMark = true; }
  bool isAbstract() const { return abstractMark; }
  void setPrivate() { privateMark = true; }
  bool isPrivate() const { return privateMark; }
  void setStatic() { staticMark = true; }
  bool isStatic() const { return staticMark; }
  void setOverride() { overrideMark = true; }
  bool isOverride() const { return overrideMark; }
  void setCtor() { ctorMark = true; }
  bool isCtor() const { return ctorMark; }

 private:
  std::vector<std::string> parameters;
  std::vector<Token> code;
  std::string _name;
  std::map<std::string, Value> parameterKVP;
  bool abstractMark = false;
  bool privateMark = false;
  bool overrideMark = false;
  bool staticMark = false;
  bool ctorMark = false;
};

#endif