#ifndef KIWI_TYPING_VALUETYPE_H
#define KIWI_TYPING_VALUETYPE_H

#include <map>
#include <memory>
#include <variant>
#include "errors/error.h"

struct Hash;
struct List;
struct Object;

using Value =
    std::variant<int, double, bool, std::string, std::shared_ptr<List>,
                 std::shared_ptr<Hash>, std::shared_ptr<Object>>;

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

enum class ValueType {
  None,
  Unknown,
  Integer,
  Double,
  Boolean,
  String,
  List,
  Hash,
  Object
};

#endif