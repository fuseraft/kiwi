#ifndef KIWI_TYPING_VALUETYPE_H
#define KIWI_TYPING_VALUETYPE_H

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <sstream>
#include <unordered_map>
#include <variant>
#include <vector>
#include "tracing/error.h"

enum KValueType {
  INTEGER = 0,
  FLOAT = 1,
  BOOLEAN = 2,
  STRING = 3,
  LIST = 4,
  HASHMAP = 5,
  OBJECT = 6,
  LAMBDA = 7,
  NONE = 8,
  STRUCT = 9,
  POINTER = 10,
  UNSET = 20
};

struct KValue;
struct Hashmap;
struct List;
struct Object;
struct LambdaRef;
struct StructRef;
struct Null;

typedef long long k_int;
typedef std::string k_string;

using k_hashmap = std::shared_ptr<Hashmap>;
using k_list = std::shared_ptr<List>;
using k_object = std::shared_ptr<Object>;
using k_lambda = std::shared_ptr<LambdaRef>;
using k_struct = std::shared_ptr<StructRef>;
using k_null = std::shared_ptr<Null>;

struct k_pointer {
  void* ptr;

  k_pointer() : ptr(nullptr) {}
  explicit k_pointer(void* p) : ptr(p) {}

  template <typename T>
  T* as() const {
    return static_cast<T*>(ptr);
  }

  static std::string serialize(const k_pointer& kp) {
    if (kp.isNull()) {
      return "nullptr";
    }
    std::stringstream ss;
    ss << kp.ptr;
    return ss.str();
  }

  bool isNull() const { return ptr == nullptr; }

  bool operator==(const k_pointer& other) const { return ptr == other.ptr; }
  bool operator!=(const k_pointer& other) const { return ptr != other.ptr; }
};

inline void hash_combine(std::size_t& seed, std::size_t hash);
std::size_t hash_hash(const k_hashmap& hash);
std::size_t hash_list(const k_list& list);
std::size_t hash_object(const k_object& object);

using k_value = std::variant<k_int, double, bool, k_string, k_list, k_hashmap,
                             k_object, k_lambda, k_null, k_struct, k_pointer>;

// Specialize a struct for hash computation for k_value
namespace std {
template <>
struct hash<k_value> {
  std::size_t operator()(const k_value& v) const {
    switch (v.index()) {
      case KValueType::INTEGER:
        return std::hash<k_int>()(std::get<k_int>(v));
      case KValueType::FLOAT:
        return std::hash<double>()(std::get<double>(v));
      case KValueType::BOOLEAN:
        return std::hash<bool>()(std::get<bool>(v));
      case KValueType::STRING:
        return std::hash<k_string>()(std::get<k_string>(v));
      case KValueType::LIST:
        return hash_list(std::get<k_list>(v));
      case KValueType::HASHMAP:
        return hash_hash(std::get<k_hashmap>(v));
      case KValueType::OBJECT:
        return hash_object(std::get<k_object>(v));
      case KValueType::LAMBDA:
      case KValueType::NONE:
      case KValueType::STRUCT:
        return false;
      case KValueType::POINTER:
        return std::hash<void*>()(std::get<k_pointer>(v).ptr);
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

struct Hashmap {
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

  void merge(const k_hashmap& other) {
    for (const auto& key : other->keys) {
      add(key, other->kvp[key]);
    }
  }
};

struct Object {
  k_string identifier;
  k_string structName;
  std::unordered_map<k_string, k_value> instanceVariables;

  bool hasVariable(const k_string& name) const {
    return instanceVariables.find(name) != instanceVariables.end();
  }
};

struct LambdaRef {
  k_string identifier;

  LambdaRef(const k_string& identifier) : identifier(identifier) {}
};

struct StructRef {
  k_string identifier;

  StructRef(const k_string& identifier) : identifier(identifier) {}
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

std::size_t hash_hash(const k_hashmap& hash) {
  std::size_t seed = 0;
  for (const auto& pair : hash->kvp) {
    hash_combine(seed, std::hash<k_value>()(pair.first));
    hash_combine(seed, std::hash<k_value>()(pair.second));
  }
  return seed;
}

std::size_t hash_object(const k_object& object) {
  auto seed = std::hash<k_string>()(object->structName);
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
      case KValueType::INTEGER:
        return *std::get_if<k_int>(&lhs) < *std::get_if<k_int>(&rhs);
      case KValueType::FLOAT:
        return *std::get_if<double>(&lhs) < *std::get_if<double>(&rhs);
      case KValueType::BOOLEAN:
        return *std::get_if<bool>(&lhs) < *std::get_if<bool>(&rhs);
      case KValueType::STRING:
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
k_hashmap clone_hash(const k_hashmap& original);
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

k_hashmap clone_hash(const k_hashmap& original) {
  k_hashmap clone = std::make_shared<Hashmap>();
  auto& keys = original->keys;
  for (const auto& key : keys) {
    clone->add(key, clone_value(original->get(key)));
  }
  return clone;
}

k_value clone_value(const k_value& original) {
  switch (original.index()) {
    case KValueType::INTEGER:
      return std::get<k_int>(original);
    case KValueType::FLOAT:
      return std::get<double>(original);
    case KValueType::BOOLEAN:
      return std::get<bool>(original);
    case KValueType::STRING:
      return std::get<k_string>(original);
    case KValueType::LIST:
      return clone_list(std::get<k_list>(original));
    case KValueType::HASHMAP:
      return clone_hash(std::get<k_hashmap>(original));
    case KValueType::OBJECT:
      return std::make_shared<Object>(*std::get<k_object>(original));
    case KValueType::LAMBDA:
      return std::make_shared<LambdaRef>(*std::get<k_lambda>(original));
    case KValueType::NONE:
      return std::make_shared<Null>(*std::get<k_null>(original));
    case KValueType::STRUCT:
      return std::make_shared<StructRef>(*std::get<k_struct>(original));
    default:
      throw std::runtime_error("Unsupported type for cloning");
  }
}

bool same_value(const k_value& v1, const k_value& v2) {
  if (v1.index() != v2.index()) {
    return false;
  }

  switch (v1.index()) {
    case KValueType::INTEGER:
      return *std::get_if<k_int>(&v1) == *std::get_if<k_int>(&v2);
    case KValueType::FLOAT:
      return *std::get_if<double>(&v1) == *std::get_if<double>(&v2);
    case KValueType::BOOLEAN:
      return *std::get_if<bool>(&v1) == *std::get_if<bool>(&v2);
    case KValueType::STRING:
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
    case KValueType::INTEGER:
      return std::get<k_int>(lhs) < std::get<k_int>(rhs);
    case KValueType::FLOAT:
      return std::get<double>(lhs) < std::get<double>(rhs);
    case KValueType::BOOLEAN:
      return std::get<bool>(lhs) < std::get<bool>(rhs);
    case KValueType::STRING:
      return std::get<k_string>(lhs) < std::get<k_string>(rhs);
    case KValueType::LIST:
      return hash_list(std::get<k_list>(lhs)) <
             hash_list(std::get<k_list>(rhs));
    case KValueType::HASHMAP:
      return hash_hash(std::get<k_hashmap>(lhs)) <
             hash_hash(std::get<k_hashmap>(rhs));
    case KValueType::OBJECT:
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
    case KValueType::INTEGER:
      return std::get<k_int>(lhs) > std::get<k_int>(rhs);
    case KValueType::FLOAT:
      return std::get<double>(lhs) > std::get<double>(rhs);
    case KValueType::BOOLEAN:
      return std::get<bool>(lhs) > std::get<bool>(rhs);
    case KValueType::STRING:
      return std::get<k_string>(lhs) > std::get<k_string>(rhs);
    case KValueType::LIST:
      return hash_list(std::get<k_list>(lhs)) >
             hash_list(std::get<k_list>(rhs));
    case KValueType::HASHMAP:
      return hash_hash(std::get<k_hashmap>(lhs)) >
             hash_hash(std::get<k_hashmap>(rhs));
    case KValueType::OBJECT:
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

// ==========================================
// KValue and KValueType
// ==========================================

struct KValue {
 public:
  KValue() {}
  KValue(const k_value& value, const KValueType& type) : _value(value), _type(type) {}

  static KValue createInteger(const k_value& value) {
    return { value, KValueType::INTEGER };
  }
  static KValue createFloat(const k_value& value) {
    return { value, KValueType::FLOAT };
  }
  static KValue createBoolean(const k_value& value) {
    return { value, KValueType::BOOLEAN };
  }
  static KValue createString(const k_value& value) {
    return { value, KValueType::STRING };
  }
  static KValue createList(const k_value& value) {
    return { value, KValueType::LIST };
  }
  static KValue createHashmap(const k_value& value) {
    return { value, KValueType::HASHMAP };
  }
  static KValue createObject(const k_value& value) {
    return { value, KValueType::OBJECT };
  }
  static KValue createLambda(const k_value& value) {
    return { value, KValueType::LAMBDA };
  }
  static KValue createNull(const k_value& value) {
    return { value, KValueType::NONE };
  }
  static KValue createStruct(const k_value& value) {
    return { value, KValueType::STRUCT };
  }
  static KValue createPointer(const k_value& value) {
    return { value, KValueType::POINTER };
  }

  const k_int getInteger() const {
    return std::get<k_int>(_value);
  }
  const double getFloat() const {
    return std::get<double>(_value);
  }
  const bool getBoolean() const {
    return std::get<bool>(_value);
  }
  const k_string getString() const {
    return std::get<k_string>(_value);
  }
  const k_list getList() const {
    return std::get<k_list>(_value);
  }
  const k_hashmap getHashmap() const {
    return std::get<k_hashmap>(_value);
  }
  const k_object getObject() const {
    return std::get<k_object>(_value);
  }
  const k_lambda getLambda() const {
    return std::get<k_lambda>(_value);
  }
  const k_null getNull() const {
    return std::get<k_null>(_value);
  }
  const k_struct getStruct() const {
    return std::get<k_struct>(_value);
  }
  const k_pointer getPointer() const {
    return std::get<k_pointer>(_value);
  }

  const bool isInteger() const {
    return _type == KValueType::INTEGER;
  }
  const bool isFloat() const {
    return _type == KValueType::FLOAT;
  }
  const bool isBoolean() const {
    return _type == KValueType::BOOLEAN;
  }
  const bool isString() const {
    return _type == KValueType::STRING;
  }
  const bool isList() const {
    return _type == KValueType::LIST;
  }
  const bool isHashmap() const {
    return _type == KValueType::HASHMAP;
  }
  const bool isObject() const {
    return _type == KValueType::OBJECT;
  }
  const bool isLambda() const {
    return _type == KValueType::LAMBDA;
  }
  const bool isNull() const {
    return _type == KValueType::NONE;
  }
  const bool isStruct() const {
    return _type == KValueType::STRUCT;
  }
  const bool isPointer() const {
    return _type == KValueType::POINTER;
  }

  void setValue(const KValue& value) {
    _value = value._value;
    _type = value._type;
  }
  void setValue(const k_int& value) {
    _value = value;
    _type = KValueType::INTEGER;
  }
  void setValue(const double& value) {
    _value = value;
    _type = KValueType::FLOAT;
  }
  void setValue(const bool& value) {
    _value = value;
    _type = KValueType::BOOLEAN;
  }
  void setValue(const k_string& value) {
    _value = value;
    _type = KValueType::STRING;
  }
  void setValue(const k_list& value) {
    _value = value;
    _type = KValueType::LIST;
  }
  void setValue(const k_hashmap& value) {
    _value = value;
    _type = KValueType::HASHMAP;
  }
  void setValue(const k_object& value) {
    _value = value;
    _type = KValueType::OBJECT;
  }
  void setValue(const k_lambda& value) {
    _value = value;
    _type = KValueType::LAMBDA;
  }
  void setValue(const k_null& value) {
    _value = value;
    _type = KValueType::NONE;
  }
  void setValue(const k_struct& value) {
    _value = value;
    _type = KValueType::STRUCT;
  }
  void setValue(const k_pointer& value) {
    _value = value;
    _type = KValueType::POINTER;
  }
  
 private:
  k_value _value = {};
  KValueType _type = KValueType::UNSET;
};

#endif