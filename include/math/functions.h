#ifndef KIWI_MATH_FUNCTIONS_H
#define KIWI_MATH_FUNCTIONS_H

#include <cmath>
#include <limits>
#include <memory>
#include <sstream>
#include "errors/error.h"
#include "k_int.h"
#include "parsing/tokens.h"
#include "typing/valuetype.h"
#include "rng.h"

static std::string get_string(const Token& term, const Value& arg) {
  if (!std::holds_alternative<std::string>(arg)) {
    throw ConversionError(term, "Expected a String value.");
  }
  return std::get<std::string>(arg);
}

static k_int get_integer(const Token& term, const Value& arg) {
  if (std::holds_alternative<double>(arg)) {
    return static_cast<k_int>(std::get<double>(arg));
  }
  if (!std::holds_alternative<k_int>(arg)) {
    throw ConversionError(term, "Expected an Integer value.");
  }
  return std::get<k_int>(arg);
}

static double get_integer_or_double(const Token& term, const Value& arg) {
  if (std::holds_alternative<k_int>(arg)) {
    return std::get<k_int>(arg);
  } else if (std::holds_alternative<double>(arg)) {
    return std::get<double>(arg);
  }

  throw ConversionError(term, "Expected an Integer or Double value.");
}

struct {
  bool is_zero(const Token& term, const Value& v) {
    if (std::holds_alternative<double>(v)) {
      return std::get<double>(v) == 0.0;
    } else if (std::holds_alternative<k_int>(v)) {
      return std::get<k_int>(v) == 0;
    } else {
      throw ConversionError(term,
                            "Cannot check non-numeric value for zero value.");
    }

    return false;
  }

  Value do_addition(const Token& token, const Value& left, const Value& right) {
    Value result;

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
    } else if (std::holds_alternative<std::string>(left)) {
      std::ostringstream build;
      build << std::get<std::string>(left);

      if (std::holds_alternative<k_int>(right)) {
        build << std::get<k_int>(right);
      } else if (std::holds_alternative<double>(right)) {
        build << std::get<double>(right);
      } else if (std::holds_alternative<bool>(right)) {
        build << std::boolalpha << std::get<bool>(right);
      } else if (std::holds_alternative<std::string>(right)) {
        build << std::get<std::string>(right);
      }

      result = build.str();
    } else if (std::holds_alternative<std::shared_ptr<List>>(left)) {
      auto list = std::get<std::shared_ptr<List>>(left);
      list->elements.push_back(right);
      return list;
    } else {
      throw ConversionError(token, "Conversion error in addition.");
    }

    return result;
  }

  Value do_subtraction(const Token& token, const Value& left,
                       const Value& right) {
    Value result;

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
    } else if (std::holds_alternative<std::shared_ptr<List>>(left)) {
      std::vector<Value> listValues;
      bool found = false;

      for (const auto& item : std::get<std::shared_ptr<List>>(left)->elements) {
        if (!found && same_value(item, right)) {
          found = true;
          continue;
        }
        listValues.push_back(item);
      }

      return std::make_shared<List>(listValues);
    } else {
      throw ConversionError(token, "Conversion error in subtraction.");
    }

    return result;
  }

  Value do_exponentiation(const Token& token, const Value& left,
                          const Value& right) {
    Value result;

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

  Value do_modulus(const Token& token, const Value& left, const Value& right) {
    Value result;

    if (std::holds_alternative<k_int>(left) &&
        std::holds_alternative<k_int>(right)) {
      int rhs = std::get<k_int>(right);
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

  Value do_division(const Token& token, const Value& left, const Value& right) {
    Value result;

    if (std::holds_alternative<k_int>(left) &&
        std::holds_alternative<k_int>(right)) {
      int rhs = std::get<k_int>(right);
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

  Value do_multiplication(const Token& token, const Value& left,
                          const Value& right) {
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
    } else if (std::holds_alternative<std::string>(left) &&
               std::holds_alternative<k_int>(right)) {
      return do_string_multiplication(token, left, right);
    } else if (std::holds_alternative<std::shared_ptr<List>>(left) &&
               std::holds_alternative<k_int>(right)) {
      do_list_multiplication(token, left, right);
    }

    throw ConversionError(token, "Conversion error in multiplication.");
  }

  Value do_list_multiplication(const Token& token, const Value& left,
                               const Value& right) {
    auto list = std::get<std::shared_ptr<List>>(left);
    int multiplier = std::get<k_int>(right);

    if (multiplier < 1) {
      throw SyntaxError(token,
                        "List multiplier must be a positive non-zero integer.");
    }

    if (list->elements.size() == 0) {
      throw SyntaxError(token, "Cannot multiply an empty list.");
    }

    auto newList = std::make_shared<List>();

    for (int i = 0; i < multiplier; ++i) {
      for (const auto& item : list->elements) {
        newList->elements.push_back(item);
      }
    }

    return newList;
  }

  Value do_string_multiplication(const Token& token, const Value& left,
                                 const Value& right) {
    auto string = std::get<std::string>(left);
    int multiplier = std::get<k_int>(right);

    if (multiplier < 1) {
      throw SyntaxError(
          token, "String multiplier must be a positive non-zero integer.");
    }

    std::ostringstream build;

    for (int i = 0; i < multiplier; ++i) {
      build << string;
    }

    return build.str();
  }

  Value do_eq_comparison(const Value& left, const Value& right) {
    return same_value(left, right);
  }

  Value do_neq_comparison(const Value& left, const Value& right) {
    return !same_value(left, right);
  }

  Value do_lt_comparison(const Value& left, const Value& right) {
    return lt_value(left, right);
  }

  Value do_lte_comparison(const Value& left, const Value& right) {
    return lt_value(left, right) || same_value(left, right);
  }

  Value do_gt_comparison(const Value& left, const Value& right) {
    return gt_value(left, right);
  }

  Value do_gte_comparison(const Value& left, const Value& right) {
    return gt_value(left, right) || same_value(left, right);
  }

  Value do_bitwise_and(const Token& token, const Value& left,
                       const Value& right) {
    if (std::holds_alternative<k_int>(left) &&
        std::holds_alternative<k_int>(right)) {
      return std::get<k_int>(left) & std::get<k_int>(right);
    }

    throw ConversionError(token, "Conversion error in bitwise & operation.");
  }

  Value do_bitwise_or(const Token& token, const Value& left,
                      const Value& right) {
    if (std::holds_alternative<k_int>(left) &&
        std::holds_alternative<k_int>(right)) {
      return std::get<k_int>(left) | std::get<k_int>(right);
    }

    throw ConversionError(token, "Conversion error in bitwise | operation.");
  }

  Value do_bitwise_xor(const Token& token, const Value& left,
                       const Value& right) {
    if (std::holds_alternative<k_int>(left) &&
        std::holds_alternative<k_int>(right)) {
      return std::get<k_int>(left) ^ std::get<k_int>(right);
    }

    throw ConversionError(token, "Conversion error in bitwise ^ operation.");
  }

  Value do_bitwise_not(const Token& token, const Value& left) {
    if (std::holds_alternative<k_int>(left)) {
      return ~std::get<k_int>(left);
    }

    throw ConversionError(token, "Conversion error in bitwise ~ operation.");
  }

  Value do_bitwise_lshift(const Token& token, const Value& left,
                          const Value& right) {
    if (std::holds_alternative<k_int>(left) &&
        std::holds_alternative<k_int>(right)) {
      return std::get<k_int>(left) << std::get<k_int>(right);
    }

    throw ConversionError(token, "Conversion error in bitwise << operation.");
  }

  Value do_bitwise_rshift(const Token& token, const Value& left,
                          const Value& right) {
    if (std::holds_alternative<k_int>(left) &&
        std::holds_alternative<k_int>(right)) {
      return std::get<k_int>(left) >> std::get<k_int>(right);
    }

    throw ConversionError(token, "Conversion error in bitwise >> operation.");
  }

  double get_double(const Token& token, const Value& value) {
    if (std::holds_alternative<k_int>(value)) {
      return static_cast<double>(std::get<k_int>(value));
    } else if (std::holds_alternative<double>(value)) {
      return std::get<double>(value);
    }

    throw ConversionError(token, "Cannot convert value to a double value.");
  }

  double epsilon() { return std::numeric_limits<double>::epsilon(); }

  Value do_sin(const Token& token, const Value& value) {
    return sin(get_double(token, value));
  }

  Value do_sinh(const Token& token, const Value& value) {
    return sinh(get_double(token, value));
  }

  Value do_asin(const Token& token, const Value& value) {
    return asin(get_double(token, value));
  }

  Value do_tan(const Token& token, const Value& value) {
    return tan(get_double(token, value));
  }

  Value do_tanh(const Token& token, const Value& value) {
    return tanh(get_double(token, value));
  }

  Value do_atan(const Token& token, const Value& value) {
    return atan(get_double(token, value));
  }

  Value do_atan2(const Token& token, const Value& valueY, const Value& valueX) {
    return atan2(get_double(token, valueY), get_double(token, valueX));
  }

  Value do_cos(const Token& token, const Value& value) {
    return cos(get_double(token, value));
  }

  Value do_acos(const Token& token, const Value& value) {
    return acos(get_double(token, value));
  }

  Value do_cosh(const Token& token, const Value& value) {
    return cosh(get_double(token, value));
  }

  Value do_log(const Token& token, const Value& value) {
    return log(get_double(token, value));
  }

  Value do_log2(const Token& token, const Value& value) {
    return log2(get_double(token, value));
  }

  Value do_log10(const Token& token, const Value& value) {
    return log10(get_double(token, value));
  }

  Value do_log1p(const Token& token, const Value& value) {
    return log1p(get_double(token, value));
  }

  Value do_sqrt(const Token& token, const Value& value) {
    return sqrt(get_double(token, value));
  }

  Value do_cbrt(const Token& token, const Value& value) {
    return cbrt(get_double(token, value));
  }

  Value do_fmod(const Token& token, const Value& valueX, const Value& valueY) {
    return fmod(get_double(token, valueX), get_double(token, valueY));
  }

  Value do_hypot(const Token& token, const Value& valueX, const Value& valueY) {
    return hypot(get_double(token, valueX), get_double(token, valueY));
  }

  Value do_isfinite(const Token& token, const Value& value) {
    return std::isfinite(get_double(token, value));
  }

  Value do_isinf(const Token& token, const Value& value) {
    return std::isinf(get_double(token, value));
  }

  Value do_isnan(const Token& token, const Value& value) {
    return std::isnan(get_double(token, value));
  }

  Value do_isnormal(const Token& token, const Value& value) {
    return std::isnormal(get_double(token, value));
  }

  Value do_floor(const Token& token, const Value& value) {
    return floor(get_double(token, value));
  }

  Value do_ceil(const Token& token, const Value& value) {
    return ceil(get_double(token, value));
  }

  Value do_round(const Token& token, const Value& value) {
    return round(get_double(token, value));
  }

  Value do_trunc(const Token& token, const Value& value) {
    return trunc(get_double(token, value));
  }

  Value do_remainder(const Token& token, const Value& valueX,
                     const Value& valueY) {
    return remainder(get_double(token, valueX), get_double(token, valueY));
  }

  Value do_exp(const Token& token, const Value& value) {
    return exp(get_double(token, value));
  }

  Value do_expm1(const Token& token, const Value& value) {
    return expm1(get_double(token, value));
  }

  Value do_erf(const Token& token, const Value& value) {
    return erf(get_double(token, value));
  }

  Value do_erfc(const Token& token, const Value& value) {
    return erfc(get_double(token, value));
  }

  Value do_lgamma(const Token& token, const Value& value) {
    return lgamma(get_double(token, value));
  }

  Value do_tgamma(const Token& token, const Value& value) {
    return tgamma(get_double(token, value));
  }

  Value do_fdim(const Token& token, const Value& valueX, const Value& valueY) {
    return fdim(get_double(token, valueX), get_double(token, valueY));
  }

  Value do_copysign(const Token& token, const Value& valueX,
                    const Value& valueY) {
    return copysign(get_double(token, valueX), get_double(token, valueY));
  }

  Value do_nextafter(const Token& token, const Value& valueX,
                     const Value& valueY) {
    return nextafter(get_double(token, valueX), get_double(token, valueY));
  }

  Value do_max(const Token& token, const Value& valueX, const Value& valueY) {
    return fmax(get_double(token, valueX), get_double(token, valueY));
  }

  Value do_min(const Token& token, const Value& valueX, const Value& valueY) {
    return fmin(get_double(token, valueX), get_double(token, valueY));
  }

  Value do_pow(const Token& token, const Value& valueX, const Value& valueY) {
    return pow(get_double(token, valueX), get_double(token, valueY));
  }

  Value do_abs(const Token& token, const Value& value) {
    if (std::holds_alternative<k_int>(value)) {
      return static_cast<k_int>(
          labs(static_cast<long>(std::get<k_int>(value))));
    } else if (std::holds_alternative<double>(value)) {
      return fabs(std::get<double>(value));
    }

    throw ConversionError(
        token, "Cannot take an absolute value of a non-numeric value.");
  }

  Value do_random(const Token& token, const Value& valueX,
                  const Value& valueY) {
    if (std::holds_alternative<double>(valueX) ||
        std::holds_alternative<double>(valueY)) {
      double x = get_integer_or_double(token, valueX),
             y = get_integer_or_double(token, valueY);
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