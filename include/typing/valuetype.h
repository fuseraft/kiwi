#ifndef KIWI_TYPING_VALUETYPE_H
#define KIWI_TYPING_VALUETYPE_H

#include <map>
#include <memory>
#include <string>
#include <variant>
#include <vector>
#include "errors/error.h"

struct Hash;
struct List;
struct Object;
struct LambdaRef;

enum class ValueType {
  None,
  Integer,
  Double,
  Boolean,
  String,
  List,
  Hash,
  Object,
  Lambda
};

using Value = std::variant<int, double, bool, std::string,
                           std::shared_ptr<List>, std::shared_ptr<Hash>,
                           std::shared_ptr<Object>, std::shared_ptr<LambdaRef>>;

struct List {
  std::vector<Value> elements;
};

struct Hash {
  std::map<std::string, Value> kvp;
};

struct Object {
  std::string identifier;
  std::string className;
  std::map<std::string, Value> instanceVariables;
};

struct LambdaRef {
  std::string identifier;

  LambdaRef(const std::string& identifier) : identifier(identifier) {}
};

#endif