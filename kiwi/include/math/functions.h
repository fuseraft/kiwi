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
    const Token& term, const k_value& arg,
    const k_string& message = "Expected a string value.") {
  if (!std::holds_alternative<k_string>(arg)) {
    throw ConversionError(term, message);
  }
  return std::get<k_string>(arg);
}

static k_int get_integer(
    const Token& term, const k_value& arg,
    const k_string& message = "Expected an integer value.") {
  if (std::holds_alternative<double>(arg)) {
    return static_cast<k_int>(std::get<double>(arg));
  }
  if (!std::holds_alternative<k_int>(arg)) {
    throw ConversionError(term, message);
  }
  return std::get<k_int>(arg);
}

static double get_double(
    const Token& term, const k_value& arg,
    const k_string& message = "Expected an integer or double value.") {
  if (std::holds_alternative<k_int>(arg)) {
    return static_cast<double>(std::get<k_int>(arg));
  } else if (std::holds_alternative<double>(arg)) {
    return std::get<double>(arg);
  }

  throw ConversionError(term, message);
}

struct {
  bool is_zero(const Token& term, const k_value& v) {
    if (std::holds_alternative<double>(v)) {
      return std::get<double>(v) == 0.0;
    } else if (std::holds_alternative<k_int>(v)) {
      return std::get<k_int>(v) == 0;
    }

    throw ConversionError(term,
                          "Cannot check non-numeric value for zero value.");
  }

  k_value do_addition(const Token& token, const k_value& left,
                      const k_value& right) {
    k_value result;

    if (std::holds_alternative<k_int>(left) &&
        std::holds_alternative<k_int>(right)) {
      result = std::get<k_int>(left) + std::get<k_int>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<double>(right)) {
      result = std::get<double>(left) + std::get<double>(right);
    } else if (std::holds_alternative<k_int>(left) &&
               std::holds_alternative<double>(right)) {
      result =
          static_cast<double>(std::get<k_int>(left)) + std::get<double>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<k_int>(right)) {
      result =
          std::get<double>(left) + static_cast<double>(std::get<k_int>(right));
    } else if (std::holds_alternative<k_string>(left)) {
      std::ostringstream build;
      build << std::get<k_string>(left);

      if (std::holds_alternative<k_int>(right)) {
        build << std::get<k_int>(right);
      } else if (std::holds_alternative<double>(right)) {
        build << std::get<double>(right);
      } else if (std::holds_alternative<bool>(right)) {
        build << std::boolalpha << std::get<bool>(right);
      } else if (std::holds_alternative<k_string>(right)) {
        build << std::get<k_string>(right);
      }

      result = build.str();
    } else if (std::holds_alternative<k_list>(left)) {
      auto list = std::get<k_list>(left);
      list->elements.emplace_back(right);
      return list;
    } else {
      throw ConversionError(token, "Conversion error in addition.");
    }

    return result;
  }

  k_value do_subtraction(const Token& token, const k_value& left,
                         const k_value& right) {
    k_value result;

    if (std::holds_alternative<k_int>(left) &&
        std::holds_alternative<k_int>(right)) {
      result = std::get<k_int>(left) - std::get<k_int>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<double>(right)) {
      result = std::get<double>(left) - std::get<double>(right);
    } else if (std::holds_alternative<k_int>(left) &&
               std::holds_alternative<double>(right)) {
      result =
          static_cast<double>(std::get<k_int>(left)) - std::get<double>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<k_int>(right)) {
      result =
          std::get<double>(left) - static_cast<double>(std::get<k_int>(right));
    } else if (std::holds_alternative<k_list>(left) &&
               !std::holds_alternative<k_list>(right)) {
      std::vector<k_value> listValues;
      bool found = false;

      for (const auto& item : std::get<k_list>(left)->elements) {
        if (!found && same_value(item, right)) {
          found = true;
          continue;
        }
        listValues.emplace_back(item);
      }

      return std::make_shared<List>(listValues);
    } else if (std::holds_alternative<k_list>(left) &&
               std::holds_alternative<k_list>(right)) {
      std::vector<k_value> listValues;
      bool found = false;

      for (const auto& item : std::get<k_list>(left)->elements) {

        if (!found && same_value(item, right)) {
          found = true;
          continue;
        }
        listValues.emplace_back(item);
      }

      return std::make_shared<List>(listValues);
    } else {
      throw ConversionError(token, "Conversion error in subtraction.");
    }

    return result;
  }

  k_value do_exponentiation(const Token& token, const k_value& left,
                            const k_value& right) {
    k_value result;

    if (std::holds_alternative<k_int>(left) &&
        std::holds_alternative<k_int>(right)) {
      result = static_cast<k_int>(
          pow(std::get<k_int>(left), std::get<k_int>(right)));
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<double>(right)) {
      result = pow(std::get<double>(left), std::get<double>(right));
    } else if (std::holds_alternative<k_int>(left) &&
               std::holds_alternative<double>(right)) {
      result = pow(static_cast<double>(std::get<k_int>(left)),
                   std::get<double>(right));
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<k_int>(right)) {
      result = pow(std::get<double>(left),
                   static_cast<double>(std::get<k_int>(right)));
    } else {
      throw ConversionError(token, "Conversion error in exponentiation.");
    }

    return result;
  }

  k_value do_modulus(const Token& token, const k_value& left,
                     const k_value& right) {
    k_value result;

    if (std::holds_alternative<k_int>(left) &&
        std::holds_alternative<k_int>(right)) {
      auto rhs = std::get<k_int>(right);
      if (rhs == 0) {
        throw DivideByZeroError(token);
      }
      result = std::get<k_int>(left) % std::get<k_int>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<double>(right)) {
      double rhs = std::get<double>(right);
      if (rhs == 0.0) {
        throw DivideByZeroError(token);
      }
      result = fmod(std::get<double>(left), rhs);
    } else if (std::holds_alternative<k_int>(left) &&
               std::holds_alternative<double>(right)) {
      double rhs = std::get<double>(right);
      if (rhs == 0.0) {
        throw DivideByZeroError(token);
      }
      result = fmod(std::get<k_int>(left), rhs);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<k_int>(right)) {
      double rhs = static_cast<double>(std::get<k_int>(right));
      if (rhs == 0) {
        throw DivideByZeroError(token);
      }
      result = fmod(std::get<double>(left), rhs);
    } else {
      throw ConversionError(token, "Conversion error in modulus.");
    }

    return result;
  }

  k_value do_division(const Token& token, const k_value& left,
                      const k_value& right) {
    k_value result;

    if (std::holds_alternative<k_int>(left) &&
        std::holds_alternative<k_int>(right)) {
      auto rhs = std::get<k_int>(right);
      if (rhs == 0) {
        throw DivideByZeroError(token);
      }
      result = std::get<k_int>(left) / rhs;
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<double>(right)) {
      double rhs = std::get<double>(right);
      if (rhs == 0.0) {
        throw DivideByZeroError(token);
      }
      result = std::get<double>(left) / rhs;
    } else if (std::holds_alternative<k_int>(left) &&
               std::holds_alternative<double>(right)) {
      double rhs = std::get<double>(right);
      if (rhs == 0.0) {
        throw DivideByZeroError(token);
      }
      result = static_cast<double>(std::get<k_int>(left)) / rhs;
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<k_int>(right)) {
      double rhs = static_cast<double>(std::get<k_int>(right));
      if (rhs == 0.0) {
        throw DivideByZeroError(token);
      }
      result = std::get<double>(left) / rhs;
    } else {
      throw ConversionError(token, "Conversion error in division.");
    }

    return result;
  }

  k_value do_multiplication(const Token& token, const k_value& left,
                            const k_value& right) {
    if (std::holds_alternative<k_int>(left) &&
        std::holds_alternative<k_int>(right)) {
      return std::get<k_int>(left) * std::get<k_int>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<double>(right)) {
      return std::get<double>(left) * std::get<double>(right);
    } else if (std::holds_alternative<k_int>(left) &&
               std::holds_alternative<double>(right)) {
      return static_cast<double>(std::get<k_int>(left)) *
             std::get<double>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<k_int>(right)) {
      return std::get<double>(left) *
             static_cast<double>(std::get<k_int>(right));
    } else if (std::holds_alternative<k_string>(left) &&
               std::holds_alternative<k_int>(right)) {
      return do_string_multiplication(left, right);
    } else if (std::holds_alternative<k_list>(left) &&
               std::holds_alternative<k_int>(right)) {
      return do_list_multiplication(token, left, right);
    }

    throw ConversionError(token, "Conversion error in multiplication.");
  }

  k_value do_list_multiplication(const Token& token, const k_value& left,
                                 const k_value& right) {
    auto list = std::get<k_list>(left);
    auto multiplier = std::get<k_int>(right);

    if (multiplier < 1) {
      throw SyntaxError(token,
                        "List multiplier must be a positive non-zero integer.");
    }

    if (list->elements.size() == 0) {
      throw SyntaxError(token, "Cannot multiply an empty list.");
    }

    auto newList = std::make_shared<List>();
    auto& elements = newList->elements;

    for (int i = 0; i < multiplier; ++i) {
      for (const auto& item : list->elements) {
        elements.emplace_back(clone_value(item));
      }
    }

    return newList;
  }

  k_value do_string_multiplication(const k_value& left, const k_value& right) {
    auto string = std::get<k_string>(left);
    auto multiplier = std::get<k_int>(right);

    std::ostringstream build;

    for (int i = 0; i < multiplier; ++i) {
      build << string;
    }

    return build.str();
  }

  k_value do_eq_comparison(const k_value& left, const k_value& right) {
    return same_value(left, right);
  }

  k_value do_neq_comparison(const k_value& left, const k_value& right) {
    return !same_value(left, right);
  }

  k_value do_lt_comparison(const k_value& left, const k_value& right) {
    return lt_value(left, right);
  }

  k_value do_lte_comparison(const k_value& left, const k_value& right) {
    return lt_value(left, right) || same_value(left, right);
  }

  k_value do_gt_comparison(const k_value& left, const k_value& right) {
    return gt_value(left, right);
  }

  k_value do_gte_comparison(const k_value& left, const k_value& right) {
    return gt_value(left, right) || same_value(left, right);
  }

  k_value do_bitwise_and(const Token& token, const k_value& left,
                         const k_value& right) {
    if (std::holds_alternative<k_int>(left)) {
      auto lhs = std::get<k_int>(left);
      if (std::holds_alternative<k_int>(right)) {
        return lhs & std::get<k_int>(right);
      } else if (std::holds_alternative<double>(right)) {
        return lhs & static_cast<k_int>(std::get<double>(right));
      } else if (std::holds_alternative<bool>(right)) {
        k_int rhs = std::get<bool>(right) ? 1 : 0;
        return lhs & rhs;
      }
    }

    throw ConversionError(token, "Conversion error in bitwise & operation.");
  }

  k_value do_bitwise_or(const Token& token, const k_value& left,
                        const k_value& right) {
    if (std::holds_alternative<k_int>(left)) {
      auto lhs = std::get<k_int>(left);
      if (std::holds_alternative<k_int>(right)) {
        return lhs | std::get<k_int>(right);
      } else if (std::holds_alternative<double>(right)) {
        return lhs | static_cast<k_int>(std::get<double>(right));
      } else if (std::holds_alternative<bool>(right)) {
        k_int rhs = std::get<bool>(right) ? 1 : 0;
        return lhs | rhs;
      }
    }

    throw ConversionError(token, "Conversion error in bitwise | operation.");
  }

  k_value do_bitwise_xor(const Token& token, const k_value& left,
                         const k_value& right) {
    if (std::holds_alternative<k_int>(left)) {
      auto lhs = std::get<k_int>(left);
      if (std::holds_alternative<k_int>(right)) {
        return lhs ^ std::get<k_int>(right);
      } else if (std::holds_alternative<double>(right)) {
        return lhs ^ static_cast<k_int>(std::get<double>(right));
      } else if (std::holds_alternative<bool>(right)) {
        k_int rhs = std::get<bool>(right) ? 1 : 0;
        return lhs ^ rhs;
      }
    }

    throw ConversionError(token, "Conversion error in bitwise ^ operation.");
  }

  k_value do_bitwise_not(const Token& token, const k_value& left) {
    if (std::holds_alternative<k_int>(left)) {
      return ~std::get<k_int>(left);
    } else if (std::holds_alternative<double>(left)) {
      return ~static_cast<k_int>(std::get<double>(left));
    } else if (std::holds_alternative<bool>(left)) {
      return ~static_cast<k_int>(std::get<bool>(left) ? 1 : 0);
    }

    throw ConversionError(token, "Conversion error in bitwise ~ operation.");
  }

  k_value do_bitwise_lshift(const Token& token, const k_value& left,
                            const k_value& right) {
    if (std::holds_alternative<k_int>(left) &&
        std::holds_alternative<k_int>(right)) {
      return std::get<k_int>(left) << std::get<k_int>(right);
    }

    throw ConversionError(token, "Conversion error in bitwise << operation.");
  }

  k_value do_bitwise_rshift(const Token& token, const k_value& left,
                            const k_value& right) {
    if (std::holds_alternative<k_int>(left) &&
        std::holds_alternative<k_int>(right)) {
      return std::get<k_int>(left) >> std::get<k_int>(right);
    }

    throw ConversionError(token, "Conversion error in bitwise >> operation.");
  }

  double get_double(const Token& token, const k_value& value) {
    if (std::holds_alternative<k_int>(value)) {
      return static_cast<double>(std::get<k_int>(value));
    } else if (std::holds_alternative<double>(value)) {
      return std::get<double>(value);
    }

    throw ConversionError(token, "Cannot convert value to a double value.");
  }

  double __epsilon__() { return std::numeric_limits<double>::epsilon(); }

  k_value __sin__(const Token& token, const k_value& value) {
    return sin(get_double(token, value));
  }

  k_value __sinh__(const Token& token, const k_value& value) {
    return sinh(get_double(token, value));
  }

  k_value __asin__(const Token& token, const k_value& value) {
    return asin(get_double(token, value));
  }

  k_value __tan__(const Token& token, const k_value& value) {
    return tan(get_double(token, value));
  }

  k_value __tanh__(const Token& token, const k_value& value) {
    return tanh(get_double(token, value));
  }

  k_value __atan__(const Token& token, const k_value& value) {
    return atan(get_double(token, value));
  }

  k_value __atan2__(const Token& token, const k_value& valueY,
                    const k_value& valueX) {
    return atan2(get_double(token, valueY), get_double(token, valueX));
  }

  k_value __cos__(const Token& token, const k_value& value) {
    return cos(get_double(token, value));
  }

  k_value __acos__(const Token& token, const k_value& value) {
    return acos(get_double(token, value));
  }

  k_value __cosh__(const Token& token, const k_value& value) {
    return cosh(get_double(token, value));
  }

  k_value __log__(const Token& token, const k_value& value) {
    return log(get_double(token, value));
  }

  k_value __log2__(const Token& token, const k_value& value) {
    return log2(get_double(token, value));
  }

  k_value __log10__(const Token& token, const k_value& value) {
    return log10(get_double(token, value));
  }

  k_value __log1p__(const Token& token, const k_value& value) {
    return log1p(get_double(token, value));
  }

  k_value __sqrt__(const Token& token, const k_value& value) {
    return sqrt(get_double(token, value));
  }

  k_value __cbrt__(const Token& token, const k_value& value) {
    return cbrt(get_double(token, value));
  }

  k_value __fmod__(const Token& token, const k_value& valueX,
                   const k_value& valueY) {
    return fmod(get_double(token, valueX), get_double(token, valueY));
  }

  k_value __hypot__(const Token& token, const k_value& valueX,
                    const k_value& valueY) {
    return hypot(get_double(token, valueX), get_double(token, valueY));
  }

  k_value __isfinite__(const Token& token, const k_value& value) {
    return std::isfinite(get_double(token, value));
  }

  k_value __isinf__(const Token& token, const k_value& value) {
    return std::isinf(get_double(token, value));
  }

  k_value __isnan__(const Token& token, const k_value& value) {
    return std::isnan(get_double(token, value));
  }

  k_value __isnormal__(const Token& token, const k_value& value) {
    return std::isnormal(get_double(token, value));
  }

  k_value __floor__(const Token& token, const k_value& value) {
    return floor(get_double(token, value));
  }

  k_value __ceil__(const Token& token, const k_value& value) {
    return ceil(get_double(token, value));
  }

  k_value __round__(const Token& token, const k_value& value) {
    return round(get_double(token, value));
  }

  k_value __trunc__(const Token& token, const k_value& value) {
    return trunc(get_double(token, value));
  }

  k_value __remainder__(const Token& token, const k_value& valueX,
                        const k_value& valueY) {
    return remainder(get_double(token, valueX), get_double(token, valueY));
  }

  k_value __exp__(const Token& token, const k_value& value) {
    return exp(get_double(token, value));
  }

  k_value __expm1__(const Token& token, const k_value& value) {
    return expm1(get_double(token, value));
  }

  k_value __erf__(const Token& token, const k_value& value) {
    return erf(get_double(token, value));
  }

  k_value __erfc__(const Token& token, const k_value& value) {
    return erfc(get_double(token, value));
  }

  k_value __lgamma__(const Token& token, const k_value& value) {
    return lgamma(get_double(token, value));
  }

  k_value __tgamma__(const Token& token, const k_value& value) {
    return tgamma(get_double(token, value));
  }

  k_value __fdim__(const Token& token, const k_value& valueX,
                   const k_value& valueY) {
    return fdim(get_double(token, valueX), get_double(token, valueY));
  }

  k_value __copysign__(const Token& token, const k_value& valueX,
                       const k_value& valueY) {
    return copysign(get_double(token, valueX), get_double(token, valueY));
  }

  k_value __nextafter__(const Token& token, const k_value& valueX,
                        const k_value& valueY) {
    return nextafter(get_double(token, valueX), get_double(token, valueY));
  }

  k_value __max__(const Token& token, const k_value& valueX,
                  const k_value& valueY) {
    return fmax(get_double(token, valueX), get_double(token, valueY));
  }

  k_value __min__(const Token& token, const k_value& valueX,
                  const k_value& valueY) {
    return fmin(get_double(token, valueX), get_double(token, valueY));
  }

  k_value __pow__(const Token& token, const k_value& valueX,
                  const k_value& valueY) {
    return pow(get_double(token, valueX), get_double(token, valueY));
  }

  k_value __rotr__(k_int value, k_int shift) {
    if (shift == 0) {
      return value;
    }

    auto unsignedValue = static_cast<unsigned long long>(value);
    const int bits = sizeof(unsignedValue) * CHAR_BIT;

    shift %= bits;

    if (shift < 0) {
      shift += bits;
    }

    unsigned long long result =
        (unsignedValue >> shift) | (unsignedValue << (bits - shift));

    return static_cast<k_int>(result);
  }

  k_value __rotl__(k_int value, k_int shift) {
    if (shift == 0) {
      return value;
    }

    unsigned long long unsignedValue = static_cast<unsigned long long>(value);
    const int bits = sizeof(unsignedValue) * CHAR_BIT;

    shift %= bits;

    if (shift < 0) {
      shift += bits;
    }

    unsigned long long result =
        (unsignedValue << shift) | (unsignedValue >> (bits - shift));

    return static_cast<k_int>(result);
  }

  k_value __abs__(const Token& token, const k_value& value) {
    if (std::holds_alternative<k_int>(value)) {
      return static_cast<k_int>(
          labs(static_cast<long>(std::get<k_int>(value))));
    } else if (std::holds_alternative<double>(value)) {
      return fabs(std::get<double>(value));
    }

    throw ConversionError(
        token, "Cannot take an absolute value of a non-numeric value.");
  }

  std::vector<k_value> __divisors__(int number) {
    std::vector<k_value> divisors;

    for (int i = 1; i <= sqrt(number); ++i) {
      if (number % i == 0) {
        divisors.emplace_back(static_cast<k_int>(i));
        if (i != number / i) {
          divisors.emplace_back(static_cast<k_int>(number / i));
        }
      }
    }

    return divisors;
  }

  k_value __random__(const Token& token, const k_value& valueX,
                     const k_value& valueY) {
    if (std::holds_alternative<k_string>(valueX)) {
      auto limit = get_integer(token, valueY);
      return RNG::getInstance().randomString(std::get<k_string>(valueX), limit);
    }

    if (std::holds_alternative<k_list>(valueX)) {
      auto limit = get_integer(token, valueY);
      return RNG::getInstance().randomList(std::get<k_list>(valueX), limit);
    }

    if (std::holds_alternative<double>(valueX) ||
        std::holds_alternative<double>(valueY)) {
      double x = get_double(token, valueX), y = get_double(token, valueY);
      return RNG::getInstance().random(x, y);
    } else if (std::holds_alternative<k_int>(valueX) ||
               std::holds_alternative<k_int>(valueY)) {
      auto x = get_integer(token, valueX), y = get_integer(token, valueY);
      return RNG::getInstance().random(x, y);
    }

    throw ConversionError(token,
                          "Expected a numeric value in random number range");
  }
} MathImpl;

#endif