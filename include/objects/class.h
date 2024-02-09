#ifndef KIWI_OBJECTS_CLASS_H
#define KIWI_OBJECTS_CLASS_H

#include <map>
#include <string>
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
  std::map<std::string, Method> getMethods() const { return methods; }
  Method getMethod(const std::string& name) { return methods[name]; }
  std::string getClassName() const { return className; }
  std::string getBaseClassName() const { return baseClassName; }

 private:
  std::string className;
  std::string baseClassName;
  bool abstractMark = false;
  std::map<std::string, Method> methods;
};

#endif