#ifndef KIWI_OBJECTS_CLASS_H
#define KIWI_OBJECTS_CLASS_H

#include <algorithm>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "parsing/tokens.h"
#include "method.h"

class Class {
 public:
  Class() {}

  void setAbstract() { abstractMark = true; }
  bool isAbstract() const { return abstractMark; }

  void addMethod(Method& method) { methods[method.getName()] = method; }
  bool hasMethod(const std::string& name) const {
    return methods.find(name) != methods.end();
  }

  void setBaseClassName(const std::string& name) { baseClassName = name; }
  void setClassName(const std::string& name) { className = name; }

  std::unordered_map<std::string, Method> getMethods() const { return methods; }
  const Method& getMethod(const std::string& name) { return methods[name]; }

  const std::string& getClassName() const { return className; }
  // std::string getBaseClassName() const { return baseClassName; }

  void addPrivateVariable(const Parameter& param) {
    if (hasPrivateVariable(param.getName())) {
      return;
    }

    privateVariableNames.insert(param.getName());
  }

  bool hasPrivateVariable(const std::string& name) const {
    return privateVariableNames.find(name) != privateVariableNames.end();
  }

 private:
  std::string className;
  std::string baseClassName;
  bool abstractMark = false;
  std::unordered_map<std::string, Method> methods;
  std::unordered_set<std::string> privateVariableNames;
};

#endif