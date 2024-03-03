#ifndef KIWI_OBJECTS_METHOD_H
#define KIWI_OBJECTS_METHOD_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "parsing/tokens.h"
#include "typing/valuetype.h"

enum class MethodFlags : uint8_t {
  None = 0,
  Abstract = 1 << 0,
  Private = 1 << 1,
  Static = 1 << 2,
  Override = 1 << 3,
  Ctor = 1 << 4,
  Lambda = 1 << 5,
};

inline MethodFlags operator|(MethodFlags a, MethodFlags b) {
  return static_cast<MethodFlags>(
      static_cast<std::underlying_type<MethodFlags>::type>(a) |
      static_cast<std::underlying_type<MethodFlags>::type>(b));
}
inline MethodFlags operator&(MethodFlags a, MethodFlags b) {
  return static_cast<MethodFlags>(
      static_cast<std::underlying_type<MethodFlags>::type>(a) &
      static_cast<std::underlying_type<MethodFlags>::type>(b));
}
inline MethodFlags operator~(MethodFlags a) {
  return static_cast<MethodFlags>(
      ~static_cast<std::underlying_type_t<MethodFlags>>(a));
}

class Parameter {
 public:
  Parameter() {}
  Parameter(const std::string& name) { _name = name; }
  Parameter(const std::string& name, const Value& value) {
    _name = name;
    _value = value;
    _hasDefaultValue = true;
  }
  ~Parameter() { _value = 0; }

  std::string getName() const { return _name; }
  Value getValue() { return _value; }
  bool hasDefaultValue() const { return _hasDefaultValue; }

 private:
  std::string _name;
  bool _hasDefaultValue = false;
  Value _value;
};

class Method {
 public:
  void addParameterValue(std::string paramName, Value value) {
    parameterKVP[paramName] = value;
  }

  bool hasParameter(const std::string& paramName) const {
    return parameterKVP.find(paramName) != parameterKVP.end();
  }

  Value getParameterValue(const std::string& paramName) {
    return parameterKVP[paramName];
  }

  void addToken(Token t) { code.push_back(t); }

  const std::vector<Token>& getCode() const { return code; }

  std::string getName() const { return _name; }

  void setName(const std::string& name) { _name = name; }

  void setFlag(MethodFlags flag) { flags = flags | flag; }

  void clearFlag(MethodFlags flag) { flags = flags & ~flag; }

  bool isFlagSet(MethodFlags flag) const { return (flags & flag) == flag; }

  void addParameter(const Parameter& param) {
    auto paramName = param.getName();
    parameters.push_back(paramName);
    _params[paramName] = std::move(param);
  }

  void addParameter(const std::string& name) {
    Parameter param(name);
    addParameter(param);
  }

  void addParameter(const std::string& name, const Value& value) {
    Parameter param(name, value);
    addParameter(param);
  }

  int getParameterCount() const { return parameters.size(); }

  const std::vector<std::string>& getParameters() const { return parameters; }

  const Parameter& getParameter(const std::string& name) {
    return _params[name];
  }

 private:
  std::vector<std::string> parameters;
  std::vector<Token> code;
  std::string _name;
  std::unordered_map<std::string, Value> parameterKVP;
  MethodFlags flags = MethodFlags::None;

  std::unordered_map<std::string, Parameter> _params;
};

#endif