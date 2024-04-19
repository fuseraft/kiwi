#ifndef ASTRAL_OBJECTS_CLASS_H
#define ASTRAL_OBJECTS_CLASS_H

#include <algorithm>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include "parsing/tokens.h"
#include "typing/value.h"
#include "method.h"

class Class {
 public:
  Class() {}

  void setAbstract() { abstractMark = true; }
  bool isAbstract() const { return abstractMark; }

  void addMethod(const Method& method) { methods[method.getName()] = method; }
  bool hasMethod(const k_string& name) const {
    return methods.find(name) != methods.end();
  }

  void setBaseClassName(const k_string& name) { baseClassName = name; }
  void setClassName(const k_string& name) { className = name; }

  std::unordered_map<k_string, Method> getMethods() const { return methods; }
  const Method& getMethod(const k_string& name) { return methods[name]; }

  const k_string& getClassName() const { return className; }
  const k_string getBaseClassName() const { return baseClassName; }

  void addPrivateVariable(const Parameter& param) {
    if (hasPrivateVariable(param.getName())) {
      return;
    }

    privateVariableNames.insert(param.getName());
  }

  bool hasPrivateVariable(const k_string& name) const {
    return privateVariableNames.find(name) != privateVariableNames.end();
  }

 private:
  k_string className;
  k_string baseClassName;
  bool abstractMark = false;
  std::unordered_map<k_string, Method> methods;
  std::unordered_set<k_string> privateVariableNames;
};

#endif