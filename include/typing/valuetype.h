#ifndef KIWI_TYPING_VALUETYPE_H
#define KIWI_TYPING_VALUETYPE_H

#include <algorithm>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include "errors/error.h"
#include "k_int.h"

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

using Value = std::variant<k_int, double, bool, std::string,
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

// Comparator for Value types
struct ValueComparator {
  bool operator()(const Value& lhs, const Value& rhs) const {
    // Compare based on the type index first
    if (lhs.index() != rhs.index()) {
      return lhs.index() < rhs.index();
    }

    // If types are the same, compare based on values (if comparable)
    switch (lhs.index()) {
      case 0:  // k_int
        return *std::get_if<k_int>(&lhs) < *std::get_if<k_int>(&rhs);
      case 1:  // double
        return *std::get_if<double>(&lhs) < *std::get_if<double>(&rhs);
      case 2:  // bool
        return *std::get_if<bool>(&lhs) < *std::get_if<bool>(&rhs);
      case 3:  // std::string
        return *std::get_if<std::string>(&lhs) <
               *std::get_if<std::string>(&rhs);
      // Add more cases for other primitive types
      default:
        // For non-primitive types, just compare the pointers (not meaningful, placeholder)
        return &lhs < &rhs;
    }
  }
};

// Sorting function for List
void sortList(List& list) {
  std::sort(list.elements.begin(), list.elements.end(), ValueComparator());
}

#endif