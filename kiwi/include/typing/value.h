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
struct ClassRef;
struct Null;

typedef long long k_int;
typedef std::string k_string;

using k_hash = std::shared_ptr<Hash>;
using k_list = std::shared_ptr<List>;
using k_object = std::shared_ptr<Object>;
using k_lambda = std::shared_ptr<LambdaRef>;
using k_class = std::shared_ptr<ClassRef>;
using k_null = std::shared_ptr<Null>;

inline void hash_combine(std::size_t& seed, std::size_t hash);
std::size_t hash_hash(const k_hash& hash);
std::size_t hash_list(const k_list& list);
std::size_t hash_object(const k_object& object);

using k_value = std::variant<k_int, double, bool, k_string, k_list, k_hash,
                             k_object, k_lambda, k_null, k_class>;

// Specialize a struct for hash computation for k_value
namespace std {
template <>
struct hash<k_value> {
  std::size_t operator()(const k_value& v) const {
    switch (v.index()) {
      case 0:  // k_int
        return std::hash<k_int>()(std::get<k_int>(v));
      case 1:  // double
        return std::hash<double>()(std::get<double>(v));
      case 2:  // bool
        return std::hash<bool>()(std::get<bool>(v));
      case 3:  // k_string
        return std::hash<k_string>()(std::get<k_string>(v));
      case 4:  // k_list
        return hash_list(std::get<k_list>(v));
      case 5:  // k_hash
        return hash_hash(std::get<k_hash>(v));
      case 6:  // k_object
        return hash_object(std::get<k_object>(v));
      case 7:  // k_lambda
      case 8:  // k_null
      case 9:  // k_class
        return false;
      default:
        // Fallback for unknown types
        return 0;
    }
  }
};
}  // namespace std

struct Null {};

struct List {
  std::vector<k_value> elements;

  List() {}
  List(const std::vector<k_value>& values) : elements(values) {}
};

struct Hash {
  std::unordered_map<k_value, k_value> kvp;
  std::vector<k_value> keys;

  int size() const { return keys.size(); }

  bool hasKey(const k_value& key) const { return kvp.find(key) != kvp.end(); }

  void add(const k_value& key, k_value value) {
    if (!hasKey(key)) {
      keys.emplace_back(key);
    }
    kvp[key] = value;
  }

  k_value get(const k_value& key) { return kvp[key]; }

  void remove(const k_value& key) {
    kvp.erase(key);
    auto newEnd = std::remove(keys.begin(), keys.end(), key);
    keys.erase(newEnd, keys.end());
  }

  void merge(const k_hash& other) {
    for (const auto& key : other->keys) {
      add(key, other->kvp[key]);
    }
  }
};

struct Object {
  k_string identifier;
  k_string className;
  std::unordered_map<k_string, k_value> instanceVariables;

  bool hasVariable(const k_string& name) const {
    return instanceVariables.find(name) != instanceVariables.end();
  }
};

struct LambdaRef {
  k_string identifier;

  LambdaRef(const k_string& identifier) : identifier(identifier) {}
};

struct ClassRef {
  k_string identifier;

  ClassRef(const k_string& identifier) : identifier(identifier) {}
};

inline void hash_combine(std::size_t& seed, std::size_t hash) {
  seed ^= hash + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

struct SliceIndex {
  k_value indexOrStart = static_cast<k_int>(0);
  k_value stopIndex = static_cast<k_int>(0);
  k_value stepValue = static_cast<k_int>(0);
  bool isSlice = false;
};

std::size_t hash_list(const k_list& list) {
  std::size_t seed = 0;
  for (const auto& elem : list->elements) {
    hash_combine(seed, std::hash<k_value>()(elem));
  }
  return seed;
}

std::size_t hash_hash(const k_hash& hash) {
  std::size_t seed = 0;
  for (const auto& pair : hash->kvp) {
    hash_combine(seed, std::hash<k_value>()(pair.first));
    hash_combine(seed, std::hash<k_value>()(pair.second));
  }
  return seed;
}

std::size_t hash_object(const k_object& object) {
  auto seed = std::hash<k_string>()(object->className);
  for (const auto& pair : object->instanceVariables) {
    hash_combine(seed, std::hash<k_string>()(pair.first));
    hash_combine(seed, std::hash<k_value>()(pair.second));
  }
  return seed;
}

struct ValueComparator {
  bool operator()(const k_value& lhs, const k_value& rhs) const {
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
        return *std::get_if<k_string>(&lhs) < *std::get_if<k_string>(&rhs);
      default:
        auto lhs_hash = std::hash<k_value>()(lhs);
        auto rhs_hash = std::hash<k_value>()(rhs);
        if (lhs_hash != rhs_hash) {
          return lhs_hash < rhs_hash;
        }
        // Probably could handle this better.
        return &lhs < &rhs;
    }
  }
};

void sort_list(List& list) {
  std::sort(list.elements.begin(), list.elements.end(), ValueComparator());
}

k_value clone_value(const k_value& original);
k_hash clone_hash(const k_hash& original);
k_list clone_list(const k_list& original);

k_list clone_list(const k_list& original) {
  k_list clone = std::make_shared<List>();
  auto& cloneElements = clone->elements;
  auto& elements = original->elements;
  cloneElements.reserve(elements.size());
  for (const auto& element : elements) {
    cloneElements.push_back(clone_value(element));
  }
  return clone;
}

k_hash clone_hash(const k_hash& original) {
  k_hash clone = std::make_shared<Hash>();
  auto& keys = original->keys;
  for (const auto& key : keys) {
    clone->add(key, clone_value(original->get(key)));
  }
  return clone;
}

k_value clone_value(const k_value& original) {
  switch (original.index()) {
    case 0:  // k_int
      return std::get<k_int>(original);
    case 1:  // double
      return std::get<double>(original);
    case 2:  // bool
      return std::get<bool>(original);
    case 3:  // k_string
      return std::get<k_string>(original);
    case 4:  // k_list
      return clone_list(std::get<k_list>(original));
    case 5:  // k_hash
      return clone_hash(std::get<k_hash>(original));
    case 6:  // k_object
      return std::make_shared<Object>(*std::get<k_object>(original));
    case 7:  // k_lambda
      return std::make_shared<LambdaRef>(*std::get<k_lambda>(original));
    case 8:  // k_null
      return std::make_shared<Null>(*std::get<k_null>(original));
    case 9:  // k_class
      return std::make_shared<ClassRef>(*std::get<k_class>(original));
    default:
      throw std::runtime_error("Unsupported type for cloning");
  }
}

bool same_value(const k_value& v1, const k_value& v2) {
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
      return std::hash<k_value>()(v1) == std::hash<k_value>()(v2);
  }
}

bool lt_value(const k_value& lhs, const k_value& rhs) {
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
    case 4:  // k_list
      return hash_list(std::get<k_list>(lhs)) <
             hash_list(std::get<k_list>(rhs));
    case 5:  // k_hash
      return hash_hash(std::get<k_hash>(lhs)) <
             hash_hash(std::get<k_hash>(rhs));
    case 6:  // k_object
      return hash_object(std::get<k_object>(lhs)) <
             hash_object(std::get<k_object>(rhs));
    default:
      return false;
  }
}

bool gt_value(const k_value& lhs, const k_value& rhs) {
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
    case 4:  // k_list
      return hash_list(std::get<k_list>(lhs)) >
             hash_list(std::get<k_list>(rhs));
    case 5:  // k_hash
      return hash_hash(std::get<k_hash>(lhs)) >
             hash_hash(std::get<k_hash>(rhs));
    case 6:  // k_object
      return hash_object(std::get<k_object>(lhs)) >
             hash_object(std::get<k_object>(rhs));
    default:
      return false;
  }
}

k_value sum_listvalue(k_list list) {
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

k_value min_listvalue(k_list list) {
  const auto& elements = list->elements;

  if (elements.empty()) {
    return {};
  }

  auto minValue = elements.at(0);
  for (const auto& val : elements) {
    if (lt_value(val, minValue)) {
      minValue = val;
    }
  }

  return minValue;
}

k_value max_listvalue(k_list list) {
  const auto& elements = list->elements;

  if (elements.empty()) {
    return {};
  }

  auto maxValue = elements.at(0);
  for (const auto& val : elements) {
    if (gt_value(val, maxValue)) {
      maxValue = val;
    }
  }

  return maxValue;
}

k_value indexof_listvalue(const k_list& list, const k_value& value) {
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

k_value lastindexof_listvalue(const k_list& list, const k_value& value) {
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