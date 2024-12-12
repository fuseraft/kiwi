#ifndef KIWI_MATH_FUNCTIONS_H
#define KIWI_MATH_FUNCTIONS_H

#include <climits>
#include <cmath>
#include <limits>
#include <memory>
#include <sstream>
#include "parsing/tokens.h"
#include "tracing/error.h"
#include "typing/value.h"
#include "rng.h"

static k_string get_string(
    const Token& token, const KValue& arg,
    const k_string& message = "Expected a string value.") {
  if (!arg.isString()) {
    throw ConversionError(token, message);
  }
  return arg.getString();
}

static k_int get_integer(
    const Token& token, const KValue& arg,
    const k_string& message = "Expected an integer value.") {
  if (arg.isFloat()) {
    return static_cast<k_int>(arg.getFloat());
  }
  if (!arg.isInteger()) {
    throw ConversionError(token, message);
  }
  return arg.getInteger();
}

static double get_float(
    const Token& token, const KValue& arg,
    const k_string& message = "Expected an integer or float value.") {
  if (arg.isInteger()) {
    return static_cast<double>(arg.getInteger());
  } else if (arg.isFloat()) {
    return arg.getFloat();
  }

  throw ConversionError(token, message);
}

/*static double get_double(
    const Token& token, const KValue& arg,
    const k_string& message = "Expected an integer or double value.") {
  if (arg.isInteger()) {
    return static_cast<double>(arg.getInteger());
  } else if (arg.isFloat()) {
    return arg.getFloat();
  }

  throw ConversionError(token, message);
}*/

static k_string to_string_value(const KValue& val) {
  return Serializer::serialize(val);
}

struct {
  bool is_zero(const Token& token, const KValue& v) {
    if (v.isFloat()) {
      return v.getFloat() == 0.0;
    } else if (v.isInteger()) {
      return v.getInteger() == 0;
    }

    throw ConversionError(token,
                          "Cannot check non-numeric value for zero value.");
  }

  KValue do_addition(const Token& token, const KValue& left,
                     const KValue& right) {
    const auto& leftIsInt = left.isInteger();
    const auto& rightIsInt = right.isInteger();
    const auto& leftIsFloat = left.isFloat();
    const auto& rightIsFloat = right.isFloat();
    const auto& leftIsString = left.isString();
    const auto& rightIsString = right.isString();

    if (leftIsInt && rightIsInt) {
      return KValue::createInteger(left.getInteger() + right.getInteger());
    } else if (leftIsFloat && rightIsFloat) {
      return KValue::createFloat(left.getFloat() + right.getFloat());
    } else if ((leftIsInt && rightIsFloat) || (leftIsFloat && rightIsInt)) {
      double l =
          leftIsInt ? static_cast<double>(left.getInteger()) : left.getFloat();
      double r = rightIsInt ? static_cast<double>(right.getInteger())
                            : right.getFloat();
      return KValue::createFloat(l + r);
    } else if (leftIsString && rightIsString) {
      return KValue::createString(left.getString() + right.getString());
    } else if (leftIsString) {
      return KValue::createString(left.getString() + to_string_value(right));
    } else if (left.isList()) {
      auto listCopy = left.getList();

      if (right.isList()) {
        const auto& rhs = right.getList()->elements;
        listCopy->elements.insert(listCopy->elements.end(), rhs.begin(),
                                  rhs.end());
      } else {
        listCopy->elements.emplace_back(right);
      }

      return KValue::createList(listCopy);
    } else if (rightIsString) {
      return KValue::createString(to_string_value(left) + right.getString());
    }

    throw ConversionError(token, "Conversion error in addition.");
  }

  KValue do_subtraction(const Token& token, const KValue& left,
                        const KValue& right) {
    if (left.isInteger() && right.isInteger()) {
      return KValue::createInteger(left.getInteger() - right.getInteger());
    } else if (left.isFloat() && right.isFloat()) {
      return KValue::createFloat(left.getFloat() - right.getFloat());
    } else if (left.isInteger() && right.isFloat()) {
      return KValue::createFloat(static_cast<double>(left.getInteger()) -
                                 right.getFloat());
    } else if (left.isFloat() && right.isInteger()) {
      return KValue::createFloat(left.getFloat() -
                                 static_cast<double>(right.getInteger()));
    } else if (left.isList() && !right.isList()) {
      std::vector<KValue> listValues;
      const auto& leftList = left.getList()->elements;
      bool found = false;

      for (const auto& item : leftList) {
        if (!found && same_value(item.getValue(), right.getValue())) {
          found = true;
          continue;
        }
        listValues.emplace_back(item);
      }

      return KValue::createList(std::make_shared<List>(listValues));
    } else if (left.isList() && right.isList()) {
      std::vector<KValue> listValues;
      const auto& leftList = left.getList()->elements;
      const auto& rightList = right.getList()->elements;

      for (const auto& item : leftList) {
        bool found = false;

        for (const auto& ritem : rightList) {
          if (same_value(item.getValue(), ritem.getValue())) {
            found = true;
            break;
          }
        }

        if (!found) {
          listValues.emplace_back(item);
        }
      }
      return KValue::createList(std::make_shared<List>(listValues));
    }

    throw ConversionError(token, "Conversion error in subtraction.");
  }

  KValue do_exponentiation(const Token& token, const KValue& left,
                           const KValue& right) {
    if (left.isInteger() && right.isInteger()) {
      return KValue::createInteger(
          static_cast<k_int>(pow(left.getInteger(), right.getInteger())));
    } else if (left.isFloat() && right.isFloat()) {
      return KValue::createFloat(pow(left.getFloat(), right.getFloat()));
    } else if (left.isInteger() && right.isFloat()) {
      return KValue::createFloat(
          pow(static_cast<double>(left.getInteger()), right.getFloat()));
    } else if (left.isFloat() && right.isInteger()) {
      return KValue::createFloat(
          pow(left.getFloat(), static_cast<double>(right.getInteger())));
    }

    throw ConversionError(token, "Conversion error in exponentiation.");
  }

  KValue do_modulus(const Token& token, const KValue& left,
                    const KValue& right) {
    if (left.isInteger() && right.isInteger()) {
      auto rhs = right.getInteger();
      if (rhs == 0) {
        throw DivideByZeroError(token);
      }
      return KValue::createInteger(left.getInteger() % right.getInteger());
    } else if (left.isFloat() && right.isFloat()) {
      double rhs = right.getFloat();
      if (rhs == 0.0) {
        throw DivideByZeroError(token);
      }
      return KValue::createFloat(fmod(left.getFloat(), rhs));
    } else if (left.isInteger() && right.isFloat()) {
      double rhs = right.getFloat();
      if (rhs == 0.0) {
        throw DivideByZeroError(token);
      }
      return KValue::createFloat(fmod(left.getInteger(), rhs));
    } else if (left.isFloat() && right.isInteger()) {
      double rhs = static_cast<double>(right.getInteger());
      if (rhs == 0) {
        throw DivideByZeroError(token);
      }
      return KValue::createFloat(fmod(left.getFloat(), rhs));
    }

    throw ConversionError(token, "Conversion error in modulus.");
  }

  KValue do_division(const Token& token, const KValue& left,
                     const KValue& right) {
    if (left.isInteger() && right.isInteger()) {
      auto rhs = right.getInteger();
      if (rhs == 0) {
        throw DivideByZeroError(token);
      }
      return KValue::createInteger(left.getInteger() / rhs);
    } else if (left.isFloat() && right.isFloat()) {
      double rhs = right.getFloat();
      if (rhs == 0.0) {
        throw DivideByZeroError(token);
      }
      return KValue::createFloat(left.getFloat() / rhs);
    } else if (left.isInteger() && right.isFloat()) {
      double rhs = right.getFloat();
      if (rhs == 0.0) {
        throw DivideByZeroError(token);
      }
      return KValue::createFloat(static_cast<double>(left.getInteger()) / rhs);
    } else if (left.isFloat() && right.isInteger()) {
      double rhs = static_cast<double>(right.getInteger());
      if (rhs == 0.0) {
        throw DivideByZeroError(token);
      }
      return KValue::createFloat(left.getFloat() / rhs);
    }

    throw ConversionError(token, "Conversion error in division.");
  }

  KValue do_multiplication(const Token& token, const KValue& left,
                           const KValue& right) {
    if (left.isInteger() && right.isInteger()) {
      return KValue::createInteger(left.getInteger() * right.getInteger());
    } else if (left.isFloat() && right.isFloat()) {
      return KValue::createFloat(left.getFloat() * right.getFloat());
    } else if (left.isInteger() && right.isFloat()) {
      return KValue::createFloat(static_cast<double>(left.getInteger()) *
                                 right.getFloat());
    } else if (left.isFloat() && right.isInteger()) {
      return KValue::createFloat(left.getFloat() *
                                 static_cast<double>(right.getInteger()));
    } else if (left.isString() && right.isInteger()) {
      return do_string_multiplication(left, right);
    } else if (left.isList() && right.isInteger()) {
      return do_list_multiplication(token, left, right);
    }

    throw ConversionError(token, "Conversion error in multiplication.");
  }

  KValue do_list_multiplication(const Token& token, const KValue& left,
                                const KValue& right) {
    const auto& list = left.getList();
    auto multiplier = right.getInteger();

    if (multiplier < 1) {
      throw SyntaxError(token,
                        "List multiplier must be a positive non-zero integer.");
    }

    if (list->elements.size() == 0) {
      throw SyntaxError(token, "Cannot multiply an empty list.");
    }

    auto newList = std::make_shared<List>();
    auto& elements = newList->elements;
    elements.reserve(list->elements.size() * multiplier);

    for (int i = 0; i < multiplier; ++i) {
      for (const auto& item : list->elements) {
        elements.emplace_back(clone_value(item));
      }
    }

    return KValue::createList(newList);
  }

  KValue do_string_multiplication(const KValue& left, const KValue& right) {
    auto string = left.getString();
    auto multiplier = right.getInteger();

    if (multiplier <= 0) {
      return KValue::createString(k_string());
    }

    k_string build;
    build.reserve(string.size() * multiplier);

    for (int i = 0; i < multiplier; ++i) {
      build.append(string);
    }

    return KValue::createString(build);
  }

  bool is_truthy(const KValue& value) {
    switch (value.getType()) {
      case KValueType::_INTEGER:
        return value.getInteger() != static_cast<k_int>(0);

      case KValueType::_FLOAT:
        return value.getFloat() != static_cast<double>(0);

      case KValueType::_BOOLEAN:
        return value.getBoolean();

      case KValueType::_STRING:
        return !value.getString().empty();

      case KValueType::_LIST:
        return !value.getList()->elements.empty();

      case KValueType::_HASHMAP:
        return value.getHashmap()->size() > 0;

      case KValueType::_OBJECT:
        return true;

      case KValueType::_LAMBDA:
        return true;

      case KValueType::_NONE:
        return false;

      default:
        return false;
    }
  }

  bool do_eq_comparison(const KValue& left, const KValue& right) {
    return same_value(left.getValue(), right.getValue());
  }

  KValue do_neq_comparison(const KValue& left, const KValue& right) {
    return KValue::createBoolean(
        !same_value(left.getValue(), right.getValue()));
  }

  KValue do_lt_comparison(const KValue& left, const KValue& right) {
    return KValue::createBoolean(lt_value(left.getValue(), right.getValue()));
  }

  KValue do_lte_comparison(const KValue& left, const KValue& right) {
    const auto& leftValue = left.getValue();
    const auto& rightValue = right.getValue();
    return KValue::createBoolean(lt_value(leftValue, rightValue) ||
                                 same_value(leftValue, rightValue));
  }

  KValue do_gt_comparison(const KValue& left, const KValue& right) {
    return KValue::createBoolean(gt_value(left.getValue(), right.getValue()));
  }

  KValue do_gte_comparison(const KValue& left, const KValue& right) {
    const auto& leftValue = left.getValue();
    const auto& rightValue = right.getValue();
    return KValue::createBoolean(gt_value(leftValue, rightValue) ||
                                 same_value(leftValue, rightValue));
  }

  KValue do_bitwise_and(const Token& token, const KValue& left,
                        const KValue& right) {
    if (left.isInteger()) {
      auto lhs = left.getInteger();
      
      if (right.isInteger()) {
        return KValue::createInteger(lhs & right.getInteger());
      } else if (right.isFloat()) {
        return KValue::createInteger(lhs & static_cast<k_int>(right.getFloat()));
      } else if (right.isBoolean()) {
        k_int rhs = right.getBoolean() ? 1 : 0;
        return KValue::createInteger(lhs & rhs);
      }
    }

    throw ConversionError(token, "Conversion error in bitwise & operation.");
  }

  KValue do_bitwise_or(const Token& token, const KValue& left,
                        const KValue& right) {
    if (left.isInteger()) {
      auto lhs = left.getInteger();

      if (right.isInteger()) {
        return KValue::createInteger(lhs | right.getInteger());
      } else if (right.isFloat()) {
        return KValue::createInteger(lhs | static_cast<k_int>(right.getFloat()));
      } else if (right.isBoolean()) {
        k_int rhs = right.getBoolean() ? 1 : 0;
        return KValue::createInteger(lhs | rhs);
      }
    }

    throw ConversionError(token, "Conversion error in bitwise | operation.");
  }

  KValue do_bitwise_xor(const Token& token, const KValue& left,
                         const KValue& right) {
    if (left.isInteger()) {
      auto lhs = left.getInteger();
      
      if (right.isInteger()) {
        return KValue::createInteger(lhs ^ right.getInteger());
      } else if (right.isFloat()) {
        return KValue::createInteger(lhs ^ static_cast<k_int>(right.getFloat()));
      } else if (right.isBoolean()) {
        k_int rhs = right.getBoolean() ? 1 : 0;
        return KValue::createInteger(lhs ^ rhs);
      }
    }

    throw ConversionError(token, "Conversion error in bitwise ^ operation.");
  }

  KValue do_bitwise_not(const Token& token, const KValue& left) {
    if (left.isInteger()) {
      return KValue::createInteger(~(left.getInteger()));
    } else if (left.isFloat()) {
      return KValue::createInteger(~(static_cast<k_int>(left.getFloat())));
    } else if (left.isBoolean()) {
      return KValue::createInteger(~(static_cast<k_int>(left.getBoolean() ? 1 : 0)));
    }

    throw ConversionError(token, "Conversion error in bitwise ~ operation.");
  }

  KValue do_bitwise_lshift(const Token& token, const KValue& left,
                           const KValue& right) {
    if (left.isInteger() && right.isInteger()) {
      return KValue::createInteger(left.getInteger() << right.getInteger());
    }

    throw ConversionError(token, "Conversion error in bitwise << operation.");
  }

  KValue do_bitwise_urshift(const Token& token, KValue left, KValue right) {
    if (!left.isInteger() || !right.isInteger()) {
      throw ConversionError(token,
                            "Conversion error in bitwise >>> operation.");
    }

    auto a = left.getInteger();
    auto b = right.getInteger();

    if (b >= static_cast<k_int>(32)) {
      return {};
    }

    unsigned int a_int = static_cast<unsigned int>(a);

    return KValue::createInteger(static_cast<k_int>(a_int >> b));
  }

  KValue do_bitwise_rshift(const Token& token, const KValue& left,
                           const KValue& right) {
    if (left.isInteger() && right.isInteger()) {
      return KValue::createInteger(left.getInteger() >> right.getInteger());
    }

    throw ConversionError(token, "Conversion error in bitwise >> operation.");
  }

  KValue do_negation(const Token& token, const KValue& right) {
    if (right.isInteger()) {
      return KValue::createInteger(-right.getInteger());
    } else if (right.isFloat()) {
      return KValue::createFloat(-right.getFloat());
    } else {
      throw ConversionError(token,
                            "Unary minus applied to a non-numeric value.");
    }
  }

  KValue do_logical_not(const KValue& right) {
    if (right.isBoolean()) {
      return KValue::createBoolean(!right.getBoolean());
    } else if (right.isNull()) {
      return KValue::createBoolean(true);
    } else if (right.isInteger()) {
      return KValue::createBoolean(right.getInteger() == 0 ? true : false);
    } else if (right.isFloat()) {
      return KValue::createBoolean(right.getFloat() == 0.0);
    } else if (right.isString()) {
      return KValue::createBoolean(right.getString().empty());
    } else if (right.isList()) {
      return KValue::createBoolean(right.getList()->elements.empty());
    } else if (right.isHashmap()) {
      return KValue::createBoolean(right.getHashmap()->keys.empty());
    } else {
      return KValue::createBoolean(false);  // Object, Lambda, etc.
    }
  }

  double get_double(const Token& token, const KValue& value) {
    if (value.isInteger()) {
      return static_cast<double>(value.getInteger());
    } else if (value.isFloat()) {
      return value.getFloat();
    }

    throw ConversionError(token, "Cannot convert value to a double value.");
  }

  double __epsilon__() { return std::numeric_limits<double>::epsilon(); }

  KValue __sin__(const Token& token, const KValue& value) {
    return KValue::createFloat(sin(get_double(token, value)));
  }

  KValue __sinh__(const Token& token, const KValue& value) {
    return KValue::createFloat(sinh(get_double(token, value)));
  }

  KValue __asin__(const Token& token, const KValue& value) {
    return KValue::createFloat(asin(get_double(token, value)));
  }

  KValue __tan__(const Token& token, const KValue& value) {
    return KValue::createFloat(tan(get_double(token, value)));
  }

  KValue __tanh__(const Token& token, const KValue& value) {
    return KValue::createFloat(tanh(get_double(token, value)));
  }

  KValue __atan__(const Token& token, const KValue& value) {
    return KValue::createFloat(atan(get_double(token, value)));
  }

  KValue __atan2__(const Token& token, const KValue& valueY,
                   const KValue& valueX) {
    return KValue::createFloat(
        atan2(get_double(token, valueY), get_double(token, valueX)));
  }

  KValue __cos__(const Token& token, const KValue& value) {
    return KValue::createFloat(cos(get_double(token, value)));
  }

  KValue __acos__(const Token& token, const KValue& value) {
    return KValue::createFloat(acos(get_double(token, value)));
  }

  KValue __cosh__(const Token& token, const KValue& value) {
    return KValue::createFloat(cosh(get_double(token, value)));
  }

  KValue __log__(const Token& token, const KValue& value) {
    return KValue::createFloat(log(get_double(token, value)));
  }

  KValue __log2__(const Token& token, const KValue& value) {
    return KValue::createFloat(log2(get_double(token, value)));
  }

  KValue __log10__(const Token& token, const KValue& value) {
    return KValue::createFloat(log10(get_double(token, value)));
  }

  KValue __log1p__(const Token& token, const KValue& value) {
    return KValue::createFloat(log1p(get_double(token, value)));
  }

  KValue __sqrt__(const Token& token, const KValue& value) {
    return KValue::createFloat(sqrt(get_double(token, value)));
  }

  KValue __cbrt__(const Token& token, const KValue& value) {
    return KValue::createFloat(cbrt(get_double(token, value)));
  }

  KValue __fmod__(const Token& token, const KValue& valueX,
                  const KValue& valueY) {
    return KValue::createFloat(
        fmod(get_double(token, valueX), get_double(token, valueY)));
  }

  KValue __hypot__(const Token& token, const KValue& valueX,
                   const KValue& valueY) {
    return KValue::createFloat(
        hypot(get_double(token, valueX), get_double(token, valueY)));
  }

  KValue __isfinite__(const Token& token, const KValue& value) {
    return KValue::createBoolean(std::isfinite(get_double(token, value)));
  }

  KValue __isinf__(const Token& token, const KValue& value) {
    return KValue::createBoolean(std::isinf(get_double(token, value)));
  }

  KValue __isnan__(const Token& token, const KValue& value) {
    return KValue::createBoolean(std::isnan(get_double(token, value)));
  }

  KValue __isnormal__(const Token& token, const KValue& value) {
    return KValue::createBoolean(std::isnormal(get_double(token, value)));
  }

  KValue __floor__(const Token& token, const KValue& value) {
    return KValue::createFloat(floor(get_double(token, value)));
  }

  KValue __ceil__(const Token& token, const KValue& value) {
    return KValue::createFloat(ceil(get_double(token, value)));
  }

  KValue __round__(const Token& token, const KValue& value) {
    return KValue::createFloat(round(get_double(token, value)));
  }

  KValue __trunc__(const Token& token, const KValue& value) {
    return KValue::createFloat(trunc(get_double(token, value)));
  }

  KValue __remainder__(const Token& token, const KValue& valueX,
                       const KValue& valueY) {
    return KValue::createFloat(
        remainder(get_double(token, valueX), get_double(token, valueY)));
  }

  KValue __exp__(const Token& token, const KValue& value) {
    return KValue::createFloat(exp(get_double(token, value)));
  }

  KValue __expm1__(const Token& token, const KValue& value) {
    return KValue::createFloat(expm1(get_double(token, value)));
  }

  KValue __erf__(const Token& token, const KValue& value) {
    return KValue::createFloat(erf(get_double(token, value)));
  }

  KValue __erfc__(const Token& token, const KValue& value) {
    return KValue::createFloat(erfc(get_double(token, value)));
  }

  KValue __lgamma__(const Token& token, const KValue& value) {
    return KValue::createFloat(lgamma(get_double(token, value)));
  }

  KValue __tgamma__(const Token& token, const KValue& value) {
    return KValue::createFloat(tgamma(get_double(token, value)));
  }

  KValue __fdim__(const Token& token, const KValue& valueX,
                  const KValue& valueY) {
    return KValue::createFloat(
        fdim(get_double(token, valueX), get_double(token, valueY)));
  }

  KValue __copysign__(const Token& token, const KValue& valueX,
                      const KValue& valueY) {
    return KValue::createFloat(
        copysign(get_double(token, valueX), get_double(token, valueY)));
  }

  KValue __nextafter__(const Token& token, const KValue& valueX,
                       const KValue& valueY) {
    return KValue::createFloat(
        nextafter(get_double(token, valueX), get_double(token, valueY)));
  }

  KValue __max__(const Token& token, const KValue& valueX,
                 const KValue& valueY) {
    return KValue::createFloat(
        fmax(get_double(token, valueX), get_double(token, valueY)));
  }

  KValue __min__(const Token& token, const KValue& valueX,
                 const KValue& valueY) {
    return KValue::createFloat(
        fmin(get_double(token, valueX), get_double(token, valueY)));
  }

  KValue __pow__(const Token& token, const KValue& valueX,
                 const KValue& valueY) {
    return KValue::createFloat(
        pow(get_double(token, valueX), get_double(token, valueY)));
  }

  KValue __rotr__(k_int value, k_int shift) {
    if (shift == 0) {
      return KValue::createInteger(value);
    }

    auto unsignedValue = static_cast<unsigned long long>(value);
    const int bits = sizeof(unsignedValue) * CHAR_BIT;

    shift %= bits;

    if (shift < 0) {
      shift += bits;
    }

    unsigned long long result =
        (unsignedValue >> shift) | (unsignedValue << (bits - shift));

    return KValue::createInteger(static_cast<k_int>(result));
  }

  KValue __rotl__(k_int value, k_int shift) {
    if (shift == 0) {
      return KValue::createInteger(value);
    }

    unsigned long long unsignedValue = static_cast<unsigned long long>(value);
    const int bits = sizeof(unsignedValue) * CHAR_BIT;

    shift %= bits;

    if (shift < 0) {
      shift += bits;
    }

    unsigned long long result =
        (unsignedValue << shift) | (unsignedValue >> (bits - shift));

    return KValue::createInteger(static_cast<k_int>(result));
  }

  KValue __abs__(const Token& token, const KValue& value) {
    if (value.isInteger()) {
      return KValue::createInteger(
          static_cast<k_int>(labs(static_cast<long>(value.getInteger()))));
    } else if (value.isFloat()) {
      return KValue::createFloat(fabs(value.getFloat()));
    }

    throw ConversionError(
        token, "Cannot take an absolute value of a non-numeric value.");
  }

  std::vector<KValue> __divisors__(int number) {
    std::vector<KValue> divisors;

    for (int i = 1; i <= sqrt(number); ++i) {
      if (number % i == 0) {
        divisors.emplace_back(KValue::createInteger(i));
        if (i != number / i) {
          divisors.emplace_back(KValue::createInteger(number / i));
        }
      }
    }

    return divisors;
  }

  KValue __random__(const Token& token, const KValue& valueX,
                    const KValue& valueY) {
    if (valueX.isString()) {
      auto limit = get_integer(token, valueY);
      return KValue::createString(
          RNG::getInstance().randomString(valueX.getString(), limit));
    }

    if (valueX.isList()) {
      auto limit = get_integer(token, valueY);
      return KValue::create(
          RNG::getInstance().randomList(valueX.getList(), limit));
    }

    if (valueX.isFloat() && valueY.isFloat()) {
      double x = get_double(token, valueX), y = get_double(token, valueY);
      return KValue::createFloat(RNG::getInstance().random(x, y));
    } else if (valueX.isInteger() && valueY.isInteger()) {
      auto x = get_integer(token, valueX), y = get_integer(token, valueY);
      return KValue::createInteger(RNG::getInstance().random(x, y));
    }

    throw ConversionError(token,
                          "Expected a numeric value in random number range");
  }

  KValue do_unary_op(const Token& token, const KName& op, const KValue& right) {
    switch (op) {
      case KName::Ops_Not:
        return do_logical_not(right);

      case KName::Ops_BitwiseNot:
      case KName::Ops_BitwiseNotAssign:
        return do_bitwise_not(token, right);

      case KName::Ops_Subtract:
        return do_negation(token, right);

      default:
        throw InvalidOperationError(token, "Unknown unary operation.");
    }
  }

  KValue do_binary_op(const Token& token, const KName& op, const KValue& left,
                      const KValue& right) {
    switch (op) {
      case KName::Ops_Add:
      case KName::Ops_AddAssign:
        return do_addition(token, left, right);
      case KName::Ops_Subtract:
      case KName::Ops_SubtractAssign:
        return do_subtraction(token, left, right);
      case KName::Ops_Multiply:
      case KName::Ops_MultiplyAssign:
        return do_multiplication(token, left, right);
      case KName::Ops_Divide:
      case KName::Ops_DivideAssign:
        return do_division(token, left, right);
      case KName::Ops_Modulus:
      case KName::Ops_ModuloAssign:
        return do_modulus(token, left, right);
      case KName::Ops_Exponent:
      case KName::Ops_ExponentAssign:
        return do_exponentiation(token, left, right);
      case KName::Ops_BitwiseAnd:
      case KName::Ops_BitwiseAndAssign:
        return do_bitwise_and(token, left, right);
      case KName::Ops_BitwiseOr:
      case KName::Ops_BitwiseOrAssign:
        return do_bitwise_or(token, left, right);
      case KName::Ops_BitwiseXor:
      case KName::Ops_BitwiseXorAssign:
        return do_bitwise_xor(token, left, right);
      case KName::Ops_BitwiseLeftShift:
      case KName::Ops_BitwiseLeftShiftAssign:
        return do_bitwise_lshift(token, left, right);
      case KName::Ops_BitwiseRightShift:
      case KName::Ops_BitwiseRightShiftAssign:
        return do_bitwise_rshift(token, left, right);
      case KName::Ops_BitwiseUnsignedRightShift:
      case KName::Ops_BitwiseUnsignedRightShiftAssign:
        return do_bitwise_urshift(token, left, right);
      case KName::Ops_And:
      case KName::Ops_AndAssign:
        return KValue::createBoolean(is_truthy(left) && is_truthy(right));
      case KName::Ops_Or:
      case KName::Ops_OrAssign:
        return KValue::createBoolean(is_truthy(left) || is_truthy(right));
      case KName::Ops_LessThan:
        return do_lt_comparison(left, right);
      case KName::Ops_LessThanOrEqual:
        return do_lte_comparison(left, right);
      case KName::Ops_GreaterThan:
        return do_gt_comparison(left, right);
      case KName::Ops_GreaterThanOrEqual:
        return do_gte_comparison(left, right);
      case KName::Ops_Equal:
        return KValue::createBoolean(do_eq_comparison(left, right));
      case KName::Ops_NotEqual:
        return do_neq_comparison(left, right);
      default:
        throw InvalidOperationError(token, "Unknown binary operation.");
    }
  }
} MathImpl;

#endif