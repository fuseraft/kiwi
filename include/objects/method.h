#ifndef KIWI_OBJECTS_METHOD_H
#define KIWI_OBJECTS_METHOD_H

#include <map>
#include <string>
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
  void setFlag(MethodFlags flag) { flags = flags | flag; }
  void clearFlag(MethodFlags flag) { flags = flags & ~flag; }
  bool isFlagSet(MethodFlags flag) const { return (flags & flag) == flag; }

 private:
  std::vector<std::string> parameters;
  std::vector<Token> code;
  std::string _name;
  std::map<std::string, Value> parameterKVP;
  MethodFlags flags = MethodFlags::None;
};

#endif