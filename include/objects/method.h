#ifndef KIWI_OBJECTS_METHOD_H
#define KIWI_OBJECTS_METHOD_H

#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include "parsing/tokens.h"
#include "typing/value.h"

enum class MethodFlags : uint8_t {
  None = 0,
  Abstract = 1 << 0,
  Private = 1 << 1,
  Static = 1 << 2,
  Override = 1 << 3,
  Ctor = 1 << 4,
  Lambda = 1 << 5,
  Async = 1 << 6
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
  Parameter(const k_string& name) : _name(name) {}
  Parameter(const k_string& name, const k_value& value)
      : _name(name), _value(value) {
    _hasDefaultValue = true;
  }
  ~Parameter() { _value = static_cast<k_int>(0); }

  k_string getName() const { return _name; }
  k_value getValue() { return _value; }
  bool hasDefaultValue() const { return _hasDefaultValue; }

 private:
  k_string _name;
  bool _hasDefaultValue = false;
  k_value _value;
};

class Method {
 public:
  void addParameterValue(const k_string& paramName, k_value value) {
    parameterKVP[paramName] = value;
  }

  bool hasParameter(const k_string& paramName) const {
    return parameterKVP.find(paramName) != parameterKVP.end();
  }

  bool hasParameters() const { return parameters.size() > 0; }

  k_value getParameterValue(const k_string& paramName) {
    return parameterKVP[paramName];
  }

  void addToken(const Token& t) { code.emplace_back(t); }

  const std::vector<Token>& getCode() const { return code; }

  k_string getName() const { return _name; }

  void setName(const k_string& name) { _name = name; }

  void setFlag(MethodFlags flag) { flags = flags | flag; }

  void clearFlag(MethodFlags flag) { flags = flags & ~flag; }

  bool isFlagSet(MethodFlags flag) const { return (flags & flag) == flag; }

  void addParameter(const Parameter& param) {
    auto paramName = param.getName();
    parameters.emplace_back(paramName);
    _params[paramName] = std::move(param);
  }

  void addParameter(const k_string& name) {
    Parameter param(name);
    addParameter(param);
  }

  void addParameter(const k_string& name, const k_value& value) {
    Parameter param(name, value);
    addParameter(param);
  }

  int getParameterCount() const { return parameters.size(); }

  const std::vector<k_string>& getParameters() const { return parameters; }

  const Parameter& getParameter(const k_string& name) { return _params[name]; }

 private:
  std::vector<k_string> parameters;
  std::vector<Token> code;
  k_string _name;
  std::unordered_map<k_string, k_value> parameterKVP;
  MethodFlags flags = MethodFlags::None;

  std::unordered_map<k_string, Parameter> _params;
};

#endif