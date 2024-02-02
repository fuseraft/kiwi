#ifndef KIWI_MATH_FUNCTIONS_H
#define KIWI_MATH_FUNCTIONS_H

#include <sstream>
#include <cmath>
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

} MathImpl;

#endif