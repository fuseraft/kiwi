#ifndef KIWI_TYPING_VALUETYPE_H
#define KIWI_TYPING_VALUETYPE_H

#include <map>
#include <memory>
#include <variant>
#include "errors/error.h"

struct Hash;
struct List;

using Value = std::variant<int, double, bool, std::string,
                           std::shared_ptr<List>, std::shared_ptr<Hash>>;

struct List {
  std::vector<Value> elements;
};

struct Hash {
  std::map<std::string, Value> kvp;
};

enum class ValueType {
  None,
  Unknown,
  Integer,
  Double,
  Boolean,
  String,
  List,
  Hash
};

#endif