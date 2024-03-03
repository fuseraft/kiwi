#ifndef KIWI_TYPING_VALUETYPE_H
#define KIWI_TYPING_VALUETYPE_H

#include <memory>
#include <string>
#include <unordered_map>
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

using Value = std::variant<long long, double, bool, std::string,
                           std::shared_ptr<List>, std::shared_ptr<Hash>,
                           std::shared_ptr<Object>, std::shared_ptr<LambdaRef>>;

struct List {
  std::vector<Value> elements;
};

struct Hash {
  std::unordered_map<std::string, Value> kvp;
  std::vector<std::string> keys;

  int size() const { return keys.size(); }

  bool hasKey(const std::string& key) const {
    return kvp.find(key) != kvp.end();
  }

  void add(const std::string& key, Value value) {
    if (!hasKey(key)) {
      keys.push_back(key);
    }
    kvp[key] = value;
  }

  Value get(const std::string& key) { return kvp[key]; }

  void remove(const std::string& key) {
    kvp.erase(key);
    auto newEnd = std::remove(keys.begin(), keys.end(), key);
    keys.erase(newEnd, keys.end());
  }
};

struct Object {
  std::string identifier;
  std::string className;
  std::unordered_map<std::string, Value> instanceVariables;
};

struct LambdaRef {
  std::string identifier;

  LambdaRef(const std::string& identifier) : identifier(identifier) {}
};

#endif