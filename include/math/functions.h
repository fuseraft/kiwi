#ifndef KIWI_MATH_FUNCTIONS_H
#define KIWI_MATH_FUNCTIONS_H

#include <cmath>
#include <limits>
#include <sstream>
#include "errors/error.h"
#include "parsing/tokens.h"
#include "typing/valuetype.h"

struct {
  bool is_zero(Token tokenTerm, Value v) {
    if (std::holds_alternative<double>(v)) {
      return std::get<double>(v) == 0.0;
    } else if (std::holds_alternative<int>(v)) {
      return std::get<int>(v) == 0;
    } else {
      throw ConversionError(tokenTerm);
    }

    return false;
  }

  Value do_addition(const Token& token, Value left, Value right) {
    Value result;

    if (std::holds_alternative<int>(left) &&
        std::holds_alternative<int>(right)) {
      result = std::get<int>(left) + std::get<int>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<double>(right)) {
      result = std::get<double>(left) + std::get<double>(right);
    } else if (std::holds_alternative<int>(left) &&
               std::holds_alternative<double>(right)) {
      result =
          static_cast<double>(std::get<int>(left)) + std::get<double>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<int>(right)) {
      result =
          std::get<double>(left) + static_cast<double>(std::get<int>(right));
    } else if (std::holds_alternative<std::string>(left)) {
      std::ostringstream build;
      build << std::get<std::string>(left);

      if (std::holds_alternative<int>(right)) {
        build << std::get<int>(right);
      } else if (std::holds_alternative<double>(right)) {
        build << std::get<double>(right);
      } else if (std::holds_alternative<bool>(right)) {
        build << std::boolalpha << std::get<bool>(right);
      } else if (std::holds_alternative<std::string>(right)) {
        build << std::get<std::string>(right);
      }

      result = build.str();
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_subtraction(const Token& token, Value left, Value right) {
    Value result;

    if (std::holds_alternative<int>(left) &&
        std::holds_alternative<int>(right)) {
      result = std::get<int>(left) - std::get<int>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<double>(right)) {
      result = std::get<double>(left) - std::get<double>(right);
    } else if (std::holds_alternative<int>(left) &&
               std::holds_alternative<double>(right)) {
      result =
          static_cast<double>(std::get<int>(left)) - std::get<double>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<int>(right)) {
      result =
          std::get<double>(left) - static_cast<double>(std::get<int>(right));
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_exponentiation(const Token& token, Value left, Value right) {
    Value result;

    if (std::holds_alternative<int>(left) &&
        std::holds_alternative<int>(right)) {
      result = static_cast<int>(pow(std::get<int>(left), std::get<int>(right)));
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<double>(right)) {
      result = pow(std::get<double>(left), std::get<double>(right));
    } else if (std::holds_alternative<int>(left) &&
               std::holds_alternative<double>(right)) {
      result = pow(static_cast<double>(std::get<int>(left)),
                   std::get<double>(right));
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<int>(right)) {
      result = pow(std::get<double>(left),
                   static_cast<double>(std::get<int>(right)));
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_modulus(const Token& token, Value left, Value right) {
    Value result;

    if (std::holds_alternative<int>(left) &&
        std::holds_alternative<int>(right)) {
      int rhs = std::get<int>(right);
      if (rhs == 0) {
        throw DivideByZeroError(token);
      }
      result = std::get<int>(left) % std::get<int>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<double>(right)) {
      double rhs = std::get<double>(right);
      if (rhs == 0.0) {
        throw DivideByZeroError(token);
      }
      result = fmod(std::get<double>(left), rhs);
    } else if (std::holds_alternative<int>(left) &&
               std::holds_alternative<double>(right)) {
      double rhs = std::get<double>(right);
      if (rhs == 0.0) {
        throw DivideByZeroError(token);
      }
      result = fmod(std::get<int>(left), rhs);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<int>(right)) {
      double rhs = static_cast<double>(std::get<int>(right));
      if (rhs == 0) {
        throw DivideByZeroError(token);
      }
      result = fmod(std::get<double>(left), rhs);
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_division(const Token& token, Value left, Value right) {
    Value result;

    if (std::holds_alternative<int>(left) &&
        std::holds_alternative<int>(right)) {
      int rhs = std::get<int>(right);
      if (rhs == 0) {
        throw DivideByZeroError(token);
      }
      result = std::get<int>(left) / rhs;
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<double>(right)) {
      double rhs = std::get<double>(right);
      if (rhs == 0.0) {
        throw DivideByZeroError(token);
      }
      result = std::get<double>(left) / rhs;
    } else if (std::holds_alternative<int>(left) &&
               std::holds_alternative<double>(right)) {
      double rhs = std::get<double>(right);
      if (rhs == 0.0) {
        throw DivideByZeroError(token);
      }
      result = static_cast<double>(std::get<int>(left)) / rhs;
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<int>(right)) {
      double rhs = static_cast<double>(std::get<int>(right));
      if (rhs == 0.0) {
        throw DivideByZeroError(token);
      }
      result = std::get<double>(left) / rhs;
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_multiplication(const Token& token, Value left, Value right) {
    Value result;

    if (std::holds_alternative<int>(left) &&
        std::holds_alternative<int>(right)) {
      result = std::get<int>(left) * std::get<int>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<double>(right)) {
      result = std::get<double>(left) * std::get<double>(right);
    } else if (std::holds_alternative<int>(left) &&
               std::holds_alternative<double>(right)) {
      result =
          static_cast<double>(std::get<int>(left)) * std::get<double>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<int>(right)) {
      result =
          std::get<double>(left) * static_cast<double>(std::get<int>(right));
    } else if (std::holds_alternative<std::string>(left) &&
               std::holds_alternative<int>(right)) {
      std::string string = std::get<std::string>(left);
      int multiplier = std::get<int>(right);

      if (multiplier < 1) {
        throw SyntaxError(
            token, "String multiplier must be a positive non-zero integer.");
      }

      std::ostringstream build;
      build << string;

      for (int i = 1; i < multiplier; ++i) {
        build << string;
      }

      result = build.str();
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_eq_comparison(const Token& token, Value left, Value right) {
    Value result;

    if (std::holds_alternative<int>(left) &&
        std::holds_alternative<int>(right)) {
      result = std::get<int>(left) == std::get<int>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<double>(right)) {
      result = std::get<double>(left) == std::get<double>(right);
    } else if (std::holds_alternative<int>(left) &&
               std::holds_alternative<double>(right)) {
      result =
          static_cast<double>(std::get<int>(left)) == std::get<double>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<int>(right)) {
      result =
          std::get<double>(left) == static_cast<double>(std::get<int>(right));
    } else if (std::holds_alternative<std::string>(left) &&
               std::holds_alternative<std::string>(right)) {
      result = std::get<std::string>(left) == std::get<std::string>(right);
    } else if (std::holds_alternative<bool>(left) &&
               std::holds_alternative<bool>(right)) {
      result = std::get<bool>(left) == std::get<bool>(right);
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_neq_comparison(const Token& token, Value left, Value right) {
    Value result;

    if (std::holds_alternative<int>(left) &&
        std::holds_alternative<int>(right)) {
      result = std::get<int>(left) != std::get<int>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<double>(right)) {
      result = std::get<double>(left) != std::get<double>(right);
    } else if (std::holds_alternative<int>(left) &&
               std::holds_alternative<double>(right)) {
      result =
          static_cast<double>(std::get<int>(left)) != std::get<double>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<int>(right)) {
      result =
          std::get<double>(left) != static_cast<double>(std::get<int>(right));
    } else if (std::holds_alternative<std::string>(left) &&
               std::holds_alternative<std::string>(right)) {
      result = std::get<std::string>(left) != std::get<std::string>(right);
    } else if (std::holds_alternative<bool>(left) &&
               std::holds_alternative<bool>(right)) {
      result = std::get<bool>(left) != std::get<bool>(right);
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_lt_comparison(const Token& token, Value left, Value right) {
    Value result;

    if (std::holds_alternative<int>(left) &&
        std::holds_alternative<int>(right)) {
      result = std::get<int>(left) < std::get<int>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<double>(right)) {
      result = std::get<double>(left) < std::get<double>(right);
    } else if (std::holds_alternative<int>(left) &&
               std::holds_alternative<double>(right)) {
      result =
          static_cast<double>(std::get<int>(left)) < std::get<double>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<int>(right)) {
      result =
          std::get<double>(left) < static_cast<double>(std::get<int>(right));
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_lte_comparison(const Token& token, Value left, Value right) {
    Value result;

    if (std::holds_alternative<int>(left) &&
        std::holds_alternative<int>(right)) {
      result = std::get<int>(left) <= std::get<int>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<double>(right)) {
      result = std::get<double>(left) <= std::get<double>(right);
    } else if (std::holds_alternative<int>(left) &&
               std::holds_alternative<double>(right)) {
      result =
          static_cast<double>(std::get<int>(left)) <= std::get<double>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<int>(right)) {
      result =
          std::get<double>(left) <= static_cast<double>(std::get<int>(right));
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_gt_comparison(const Token& token, Value left, Value right) {
    Value result;

    if (std::holds_alternative<int>(left) &&
        std::holds_alternative<int>(right)) {
      result = std::get<int>(left) > std::get<int>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<double>(right)) {
      result = std::get<double>(left) > std::get<double>(right);
    } else if (std::holds_alternative<int>(left) &&
               std::holds_alternative<double>(right)) {
      result =
          static_cast<double>(std::get<int>(left)) > std::get<double>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<int>(right)) {
      result =
          std::get<double>(left) > static_cast<double>(std::get<int>(right));
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_gte_comparison(const Token& token, Value left, Value right) {
    Value result;

    if (std::holds_alternative<int>(left) &&
        std::holds_alternative<int>(right)) {
      result = std::get<int>(left) >= std::get<int>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<double>(right)) {
      result = std::get<double>(left) >= std::get<double>(right);
    } else if (std::holds_alternative<int>(left) &&
               std::holds_alternative<double>(right)) {
      result =
          static_cast<double>(std::get<int>(left)) >= std::get<double>(right);
    } else if (std::holds_alternative<double>(left) &&
               std::holds_alternative<int>(right)) {
      result =
          std::get<double>(left) >= static_cast<double>(std::get<int>(right));
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_bitwise_and(const Token& token, Value left, Value right) {
    Value result;

    if (std::holds_alternative<int>(left) &&
        std::holds_alternative<int>(right)) {
      result = std::get<int>(left) & std::get<int>(right);
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_bitwise_or(const Token& token, Value left, Value right) {
    Value result;

    if (std::holds_alternative<int>(left) &&
        std::holds_alternative<int>(right)) {
      result = std::get<int>(left) | std::get<int>(right);
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_bitwise_xor(const Token& token, Value left, Value right) {
    Value result;

    if (std::holds_alternative<int>(left) &&
        std::holds_alternative<int>(right)) {
      result = std::get<int>(left) ^ std::get<int>(right);
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_bitwise_not(const Token& token, Value left) {
    Value result;

    if (std::holds_alternative<int>(left)) {
      result = ~std::get<int>(left);
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_bitwise_lshift(const Token& token, Value left, Value right) {
    Value result;

    if (std::holds_alternative<int>(left) &&
        std::holds_alternative<int>(right)) {
      result = std::get<int>(left) << std::get<int>(right);
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_bitwise_rshift(const Token& token, Value left, Value right) {
    Value result;

    if (std::holds_alternative<int>(left) &&
        std::holds_alternative<int>(right)) {
      result = std::get<int>(left) >> std::get<int>(right);
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  double get_double(const Token& token, Value value) {
    double doubleValue;

    if (std::holds_alternative<int>(value)) {
      doubleValue = static_cast<double>(std::get<int>(value));
    } else if (std::holds_alternative<double>(value)) {
      doubleValue = std::get<double>(value);
    } else {
      throw ConversionError(token);
    }

    return doubleValue;
  }

  double epsilon() { return std::numeric_limits<double>::epsilon(); }

  Value do_sin(const Token& token, Value value) {
    return sin(get_double(token, value));
  }

  Value do_sinh(const Token& token, Value value) {
    return sinh(get_double(token, value));
  }

  Value do_asin(const Token& token, Value value) {
    return asin(get_double(token, value));
  }

  Value do_tan(const Token& token, Value value) {
    return tan(get_double(token, value));
  }

  Value do_tanh(const Token& token, Value value) {
    return tanh(get_double(token, value));
  }

  Value do_atan(const Token& token, Value value) {
    return atan(get_double(token, value));
  }

  Value do_atan2(const Token& token, Value valueY, Value valueX) {
    return atan2(get_double(token, valueY), get_double(token, valueX));
  }

  Value do_cos(const Token& token, Value value) {
    return cos(get_double(token, value));
  }

  Value do_acos(const Token& token, Value value) {
    return acos(get_double(token, value));
  }

  Value do_cosh(const Token& token, Value value) {
    return cosh(get_double(token, value));
  }

  Value do_log(const Token& token, Value value) {
    return log(get_double(token, value));
  }

  Value do_log2(const Token& token, Value value) {
    return log2(get_double(token, value));
  }

  Value do_log10(const Token& token, Value value) {
    return log10(get_double(token, value));
  }

  Value do_log1p(const Token& token, Value value) {
    return log1p(get_double(token, value));
  }

  Value do_sqrt(const Token& token, Value value) {
    return sqrt(get_double(token, value));
  }

  Value do_cbrt(const Token& token, Value value) {
    return cbrt(get_double(token, value));
  }

  Value do_fmod(const Token& token, Value valueX, Value valueY) {
    return fmod(get_double(token, valueX), get_double(token, valueY));
  }

  Value do_hypot(const Token& token, Value valueX, Value valueY) {
    return hypot(get_double(token, valueX), get_double(token, valueY));
  }

  Value do_isfinite(const Token& token, Value value) {
    return std::isfinite(get_double(token, value));
  }

  Value do_isinf(const Token& token, Value value) {
    return std::isinf(get_double(token, value));
  }

  Value do_isnan(const Token& token, Value value) {
    return std::isnan(get_double(token, value));
  }

  Value do_isnormal(const Token& token, Value value) {
    return std::isnormal(get_double(token, value));
  }

  Value do_floor(const Token& token, Value value) {
    return floor(get_double(token, value));
  }

  Value do_ceil(const Token& token, Value value) {
    return ceil(get_double(token, value));
  }

  Value do_round(const Token& token, Value value) {
    return round(get_double(token, value));
  }

  Value do_trunc(const Token& token, Value value) {
    return trunc(get_double(token, value));
  }

  Value do_remainder(const Token& token, Value valueX, Value valueY) {
    return remainder(get_double(token, valueX), get_double(token, valueY));
  }

  Value do_exp(const Token& token, Value value) {
    return exp(get_double(token, value));
  }

  Value do_expm1(const Token& token, Value value) {
    return expm1(get_double(token, value));
  }

  Value do_erf(const Token& token, Value value) {
    return erf(get_double(token, value));
  }

  Value do_erfc(const Token& token, Value value) {
    return erfc(get_double(token, value));
  }

  Value do_lgamma(const Token& token, Value value) {
    return lgamma(get_double(token, value));
  }

  Value do_tgamma(const Token& token, Value value) {
    return tgamma(get_double(token, value));
  }

  Value do_fdim(const Token& token, Value valueX, Value valueY) {
    return fdim(get_double(token, valueX), get_double(token, valueY));
  }

  Value do_copysign(const Token& token, Value valueX, Value valueY) {
    return copysign(get_double(token, valueX), get_double(token, valueY));
  }

  Value do_nextafter(const Token& token, Value valueX, Value valueY) {
    return nextafter(get_double(token, valueX), get_double(token, valueY));
  }

  Value do_max(const Token& token, Value valueX, Value valueY) {
    return fmax(get_double(token, valueX), get_double(token, valueY));
  }

  Value do_min(const Token& token, Value valueX, Value valueY) {
    return fmin(get_double(token, valueX), get_double(token, valueY));
  }

  Value do_pow(const Token& token, Value valueX, Value valueY) {
    return pow(get_double(token, valueX), get_double(token, valueY));
  }

  Value do_abs(const Token& token, Value value) {
    if (std::holds_alternative<int>(value)) {
      return abs(static_cast<int>(std::get<int>(value)));
    } else if (std::holds_alternative<double>(value)) {
      return fabs(std::get<double>(value));
    }

    throw ConversionError(token);
  }
} MathImpl;

#endif