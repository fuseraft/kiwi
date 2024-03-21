#ifndef KIWI_TYPING_VALUETYPE_H
#define KIWI_TYPING_VALUETYPE_H

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include "tracing/error.h"

struct Hash;
struct List;
struct Object;
struct LambdaRef;

typedef long long k_int;
typedef std::string k_string;

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

inline void hash_combine(std::size_t& seed, std::size_t hash);
std::size_t hashHash(const std::shared_ptr<Hash>& hash);
std::size_t hashList(const std::shared_ptr<List>& list);
std::size_t hashObject(const std::shared_ptr<Object>& object);

using Value = std::variant<k_int, double, bool, k_string,
                           std::shared_ptr<List>, std::shared_ptr<Hash>,
                           std::shared_ptr<Object>, std::shared_ptr<LambdaRef>>;

// Specialize a struct for hash computation for Value
namespace std {
template <>
struct hash<Value> {
  std::size_t operator()(const Value& v) const {
    switch (v.index()) {
      case 0:  // k_int
        return std::hash<k_int>()(std::get<k_int>(v));
      case 1:  // double
        return std::hash<double>()(std::get<double>(v));
      case 2:  // bool
        return std::hash<bool>()(std::get<bool>(v));
      case 3:  // k_string
        return std::hash<k_string>()(std::get<k_string>(v));
      case 4:  // std::shared_ptr<List>
        return hashList(std::get<std::shared_ptr<List>>(v));
      case 5:  // std::shared_ptr<Hash>
        return hashHash(std::get<std::shared_ptr<Hash>>(v));
      case 6:  // std::shared_ptr<Object>
        return hashObject(std::get<std::shared_ptr<Object>>(v));
      case 7:  // std::shared_ptr<LambdaRef>
        return false;
      default:
        // Fallback for unknown types
        return 0;
    }
  }
};
}  // namespace std

struct List {
  std::vector<Value> elements;

  List() {}
  List(const std::vector<Value>& values) : elements(values) {}
};

struct Hash {
  std::unordered_map<k_string, Value> kvp;
  std::vector<k_string> keys;

  int size() const { return keys.size(); }

  bool hasKey(const k_string& key) const {
    return kvp.find(key) != kvp.end();
  }

  void add(const k_string& key, Value value) {
    if (!hasKey(key)) {
      keys.push_back(key);
    }
    kvp[key] = value;
  }

  Value get(const k_string& key) { return kvp[key]; }

  void remove(const k_string& key) {
    kvp.erase(key);
    auto newEnd = std::remove(keys.begin(), keys.end(), key);
    keys.erase(newEnd, keys.end());
  }
};

struct Object {
  k_string identifier;
  k_string className;
  std::unordered_map<k_string, Value> instanceVariables;

  bool hasVariable(const k_string& name) const {
    return instanceVariables.find(name) != instanceVariables.end();
  }
};

struct LambdaRef {
  k_string identifier;

  LambdaRef(const k_string& identifier) : identifier(identifier) {}
};

inline void hash_combine(std::size_t& seed, std::size_t hash) {
  seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

std::size_t hashList(const std::shared_ptr<List>& list) {
  std::size_t seed = 0;
  for (const auto& elem : list->elements) {
    hash_combine(seed, std::hash<Value>()(elem));
  }
  return seed;
}

std::size_t hashHash(const std::shared_ptr<Hash>& hash) {
  std::size_t seed = 0;
  for (const auto& pair : hash->kvp) {
    hash_combine(seed, std::hash<k_string>()(pair.first));
    hash_combine(seed, std::hash<Value>()(pair.second));
  }
  return seed;
}

std::size_t hashObject(const std::shared_ptr<Object>& object) {
  auto seed = std::hash<k_string>()(object->className);
  for (const auto& pair : object->instanceVariables) {
    hash_combine(seed, std::hash<k_string>()(pair.first));
    hash_combine(seed, std::hash<Value>()(pair.second));
  }
  return seed;
}

struct ValueComparator {
  bool operator()(const Value& lhs, const Value& rhs) const {
    if (lhs.index() != rhs.index()) {
      return lhs.index() < rhs.index();
    }

    switch (lhs.index()) {
      case 0:  // k_int
        return *std::get_if<k_int>(&lhs) < *std::get_if<k_int>(&rhs);
      case 1:  // double
        return *std::get_if<double>(&lhs) < *std::get_if<double>(&rhs);
      case 2:  // bool
        return *std::get_if<bool>(&lhs) < *std::get_if<bool>(&rhs);
      case 3:  // k_string
        return *std::get_if<k_string>(&lhs) <
               *std::get_if<k_string>(&rhs);
      default:
        auto lhs_hash = std::hash<Value>()(lhs);
        auto rhs_hash = std::hash<Value>()(rhs);
        if (lhs_hash != rhs_hash) {
          return lhs_hash < rhs_hash;
        }
        // Probably could handle this better.
        return &lhs < &rhs;
    }
  }
};

void sortList(List& list) {
  std::sort(list.elements.begin(), list.elements.end(), ValueComparator());
}

bool same_value(const Value& v1, const Value& v2) {
  if (v1.index() != v2.index()) {
    return false;
  }

  switch (v1.index()) {
    case 0:  // k_int
      return *std::get_if<k_int>(&v1) == *std::get_if<k_int>(&v2);
    case 1:  // double
      return *std::get_if<double>(&v1) == *std::get_if<double>(&v2);
    case 2:  // bool
      return *std::get_if<bool>(&v1) == *std::get_if<bool>(&v2);
    case 3:  // k_string
      return *std::get_if<k_string>(&v1) == *std::get_if<k_string>(&v2);
    default:
      return std::hash<Value>()(v1) == std::hash<Value>()(v2);
  }
}

bool lt_value(const Value& lhs, const Value& rhs) {
  if (lhs.index() != rhs.index()) {
    return lhs.index() < rhs.index();
  }

  switch (lhs.index()) {
    case 0:  // k_int
      return std::get<k_int>(lhs) < std::get<k_int>(rhs);
    case 1:  // double
      return std::get<double>(lhs) < std::get<double>(rhs);
    case 2:  // bool
      return std::get<bool>(lhs) < std::get<bool>(rhs);
    case 3:  // k_string
      return std::get<k_string>(lhs) < std::get<k_string>(rhs);
    case 4:  // std::shared_ptr<List>
      return hashList(std::get<std::shared_ptr<List>>(lhs)) <
             hashList(std::get<std::shared_ptr<List>>(rhs));
    case 5:  // std::shared_ptr<Hash>
      return hashHash(std::get<std::shared_ptr<Hash>>(lhs)) <
             hashHash(std::get<std::shared_ptr<Hash>>(rhs));
    case 6:  // std::shared_ptr<Object>
      return hashObject(std::get<std::shared_ptr<Object>>(lhs)) <
             hashObject(std::get<std::shared_ptr<Object>>(rhs));
    default:
      return false;
  }
}

bool gt_value(const Value& lhs, const Value& rhs) {
  if (lhs.index() != rhs.index()) {
    return lhs.index() < rhs.index();
  }

  switch (lhs.index()) {
    case 0:  // k_int
      return std::get<k_int>(lhs) > std::get<k_int>(rhs);
    case 1:  // double
      return std::get<double>(lhs) > std::get<double>(rhs);
    case 2:  // bool
      return std::get<bool>(lhs) > std::get<bool>(rhs);
    case 3:  // k_string
      return std::get<k_string>(lhs) > std::get<k_string>(rhs);
    case 4:  // std::shared_ptr<List>
      return hashList(std::get<std::shared_ptr<List>>(lhs)) >
             hashList(std::get<std::shared_ptr<List>>(rhs));
    case 5:  // std::shared_ptr<Hash>
      return hashHash(std::get<std::shared_ptr<Hash>>(lhs)) >
             hashHash(std::get<std::shared_ptr<Hash>>(rhs));
    case 6:  // std::shared_ptr<Object>
      return hashObject(std::get<std::shared_ptr<Object>>(lhs)) >
             hashObject(std::get<std::shared_ptr<Object>>(rhs));
    default:
      return false;
  }
}

Value sum_listvalue(std::shared_ptr<List> list) {
  double sum = 0;
  bool hasDouble = false;

  for (const auto& val : list->elements) {
    if (std::holds_alternative<k_int>(val)) {
      sum += std::get<k_int>(val);
    } else if (std::holds_alternative<double>(val)) {
      sum += std::get<double>(val);
      hasDouble = true;
    }
  }

  if (hasDouble) {
    return sum;
  } else {
    return static_cast<k_int>(sum);
  }
}

Value min_listvalue(std::shared_ptr<List> list) {
  Value minValue = list->elements[0];
  for (const auto& val : list->elements) {
    if (lt_value(val, minValue)) {
      minValue = val;
    }
  }
  return minValue;
}

Value max_listvalue(std::shared_ptr<List> list) {
  Value maxValue = list->elements[0];
  for (const auto& val : list->elements) {
    if (gt_value(val, maxValue)) {
      maxValue = val;
    }
  }
  return maxValue;
}

Value indexof_listvalue(const std::shared_ptr<List>& list, const Value& value) {
  const auto& elements = list->elements;
  if (elements.empty()) {
    return static_cast<k_int>(-1);
  }

  for (size_t i = 0; i < elements.size(); ++i) {
    if (same_value(elements.at(i), value)) {
      return static_cast<k_int>(i);
    }
  }
  return static_cast<k_int>(-1);
}

Value lastindexof_listvalue(const std::shared_ptr<List>& list,
                            const Value& value) {
  const auto& elements = list->elements;
  if (elements.empty()) {
    return static_cast<k_int>(-1);
  }

  for (size_t i = elements.size(); i-- > 0;) {
    if (same_value(elements.at(i), value)) {
      return static_cast<k_int>(i);
    }
  }

  return static_cast<k_int>(-1);
}

#endif