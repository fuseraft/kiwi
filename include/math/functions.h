#ifndef KIWI_MATH_FUNCTIONS_H
#define KIWI_MATH_FUNCTIONS_H

#include <sstream>
#include <cmath>
#include "errors/error.h"
#include "parsing/tokens.h"
#include "typing/valuetype.h"

struct {
  bool is_zero(ValueType type, Value v) {
    if (type == ValueType::Double) {
      return std::get<double>(v) == 0.0;
    } else if (type == ValueType::Integer) {
      return std::get<int>(v) == 0;
    }

    return false;
  }

  Value do_addition(const Token& token, ValueType vtleft, ValueType vtright,
                    Value left, Value right) {
    Value result;

    if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
      result = std::get<int>(left) + std::get<int>(right);
    } else if (vtleft == ValueType::Double && vtright == ValueType::Double) {
      result = std::get<double>(left) + std::get<double>(right);
    } else if (vtleft == ValueType::Integer && vtright == ValueType::Double) {
      result =
          static_cast<double>(std::get<int>(left)) + std::get<double>(right);
    } else if (vtleft == ValueType::Double && vtright == ValueType::Integer) {
      result =
          std::get<double>(left) + static_cast<double>(std::get<int>(right));
    } else if (vtleft == ValueType::String) {
      std::ostringstream build;
      build << std::get<std::string>(left);

      if (vtright == ValueType::Integer) {
        build << std::get<int>(right);
      } else if (vtright == ValueType::Double) {
        build << std::get<double>(right);
      } else if (vtright == ValueType::Boolean) {
        build << std::boolalpha << std::get<bool>(right);
      } else if (vtright == ValueType::String) {
        build << std::get<std::string>(right);
      }

      result = build.str();
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_subtraction(const Token& token, ValueType vtleft, ValueType vtright,
                       Value left, Value right) {
    Value result;

    if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
      result = std::get<int>(left) - std::get<int>(right);
    } else if (vtleft == ValueType::Double && vtright == ValueType::Double) {
      result = std::get<double>(left) - std::get<double>(right);
    } else if (vtleft == ValueType::Integer && vtright == ValueType::Double) {
      result =
          static_cast<double>(std::get<int>(left)) - std::get<double>(right);
    } else if (vtleft == ValueType::Double && vtright == ValueType::Integer) {
      result =
          std::get<double>(left) - static_cast<double>(std::get<int>(right));
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_exponentiation(const Token& token, ValueType vtleft,
                          ValueType vtright, Value left, Value right) {
    Value result;

    if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
      result = static_cast<int>(pow(std::get<int>(left), std::get<int>(right)));
    } else if (vtleft == ValueType::Double && vtright == ValueType::Double) {
      result = pow(std::get<double>(left), std::get<double>(right));
    } else if (vtleft == ValueType::Integer && vtright == ValueType::Double) {
      result = pow(static_cast<double>(std::get<int>(left)),
                   std::get<double>(right));
    } else if (vtleft == ValueType::Double && vtright == ValueType::Integer) {
      result = pow(std::get<double>(left),
                   static_cast<double>(std::get<int>(right)));
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_modulus(const Token& token, ValueType vtleft, ValueType vtright,
                   Value left, Value right) {
    Value result;

    if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
      int rhs = std::get<int>(right);
      if (rhs == 0) {
        throw DivideByZeroError(token);
      }
      result = std::get<int>(left) % std::get<int>(right);
    } else if (vtleft == ValueType::Double && vtright == ValueType::Double) {
      double rhs = std::get<double>(right);
      if (rhs == 0.0) {
        throw DivideByZeroError(token);
      }
      result = fmod(std::get<double>(left), rhs);
    } else if (vtleft == ValueType::Integer && vtright == ValueType::Double) {
      double rhs = std::get<double>(right);
      if (rhs == 0.0) {
        throw DivideByZeroError(token);
      }
      result = fmod(std::get<int>(left), rhs);
    } else if (vtleft == ValueType::Double && vtright == ValueType::Integer) {
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

  Value do_division(const Token& token, ValueType vtleft, ValueType vtright,
                    Value left, Value right) {
    Value result;

    if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
      int rhs = std::get<int>(right);
      if (rhs == 0) {
        throw DivideByZeroError(token);
      }
      result = std::get<int>(left) / rhs;
    } else if (vtleft == ValueType::Double && vtright == ValueType::Double) {
      double rhs = std::get<double>(right);
      if (rhs == 0.0) {
        throw DivideByZeroError(token);
      }
      result = std::get<double>(left) / rhs;
    } else if (vtleft == ValueType::Integer && vtright == ValueType::Double) {
      double rhs = std::get<double>(right);
      if (rhs == 0.0) {
        throw DivideByZeroError(token);
      }
      result = static_cast<double>(std::get<int>(left)) / rhs;
    } else if (vtleft == ValueType::Double && vtright == ValueType::Integer) {
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

  Value do_multiplication(const Token& token, ValueType vtleft,
                          ValueType vtright, Value left, Value right) {
    Value result;

    if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
      result = std::get<int>(left) * std::get<int>(right);
    } else if (vtleft == ValueType::Double && vtright == ValueType::Double) {
      result = std::get<double>(left) * std::get<double>(right);
    } else if (vtleft == ValueType::Integer && vtright == ValueType::Double) {
      result =
          static_cast<double>(std::get<int>(left)) * std::get<double>(right);
    } else if (vtleft == ValueType::Double && vtright == ValueType::Integer) {
      result =
          std::get<double>(left) * static_cast<double>(std::get<int>(right));
    } else if (vtleft == ValueType::String && vtright == ValueType::Integer) {
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

  Value do_eq_comparison(const Token& token, ValueType vtleft,
                         ValueType vtright, Value left, Value right) {
    Value result;

    if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
      result = std::get<int>(left) == std::get<int>(right);
    } else if (vtleft == ValueType::Double && vtright == ValueType::Double) {
      result = std::get<double>(left) == std::get<double>(right);
    } else if (vtleft == ValueType::Integer && vtright == ValueType::Double) {
      result =
          static_cast<double>(std::get<int>(left)) == std::get<double>(right);
    } else if (vtleft == ValueType::Double && vtright == ValueType::Integer) {
      result =
          std::get<double>(left) == static_cast<double>(std::get<int>(right));
    } else if (vtleft == ValueType::String && vtright == ValueType::String) {
      result = std::get<std::string>(left) == std::get<std::string>(right);
    } else if (vtleft == ValueType::Boolean && vtright == ValueType::Boolean) {
      result = std::get<bool>(left) == std::get<bool>(right);
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_neq_comparison(const Token& token, ValueType vtleft,
                          ValueType vtright, Value left, Value right) {
    Value result;

    if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
      result = std::get<int>(left) != std::get<int>(right);
    } else if (vtleft == ValueType::Double && vtright == ValueType::Double) {
      result = std::get<double>(left) != std::get<double>(right);
    } else if (vtleft == ValueType::Integer && vtright == ValueType::Double) {
      result =
          static_cast<double>(std::get<int>(left)) != std::get<double>(right);
    } else if (vtleft == ValueType::Double && vtright == ValueType::Integer) {
      result =
          std::get<double>(left) != static_cast<double>(std::get<int>(right));
    } else if (vtleft == ValueType::String && vtright == ValueType::String) {
      result = std::get<std::string>(left) != std::get<std::string>(right);
    } else if (vtleft == ValueType::Boolean && vtright == ValueType::Boolean) {
      result = std::get<bool>(left) != std::get<bool>(right);
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_lt_comparison(const Token& token, ValueType vtleft,
                         ValueType vtright, Value left, Value right) {
    Value result;

    if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
      result = std::get<int>(left) < std::get<int>(right);
    } else if (vtleft == ValueType::Double && vtright == ValueType::Double) {
      result = std::get<double>(left) < std::get<double>(right);
    } else if (vtleft == ValueType::Integer && vtright == ValueType::Double) {
      result =
          static_cast<double>(std::get<int>(left)) < std::get<double>(right);
    } else if (vtleft == ValueType::Double && vtright == ValueType::Integer) {
      result =
          std::get<double>(left) < static_cast<double>(std::get<int>(right));
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_lte_comparison(const Token& token, ValueType vtleft,
                          ValueType vtright, Value left, Value right) {
    Value result;

    if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
      result = std::get<int>(left) <= std::get<int>(right);
    } else if (vtleft == ValueType::Double && vtright == ValueType::Double) {
      result = std::get<double>(left) <= std::get<double>(right);
    } else if (vtleft == ValueType::Integer && vtright == ValueType::Double) {
      result =
          static_cast<double>(std::get<int>(left)) <= std::get<double>(right);
    } else if (vtleft == ValueType::Double && vtright == ValueType::Integer) {
      result =
          std::get<double>(left) <= static_cast<double>(std::get<int>(right));
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_gt_comparison(const Token& token, ValueType vtleft,
                         ValueType vtright, Value left, Value right) {
    Value result;

    if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
      result = std::get<int>(left) > std::get<int>(right);
    } else if (vtleft == ValueType::Double && vtright == ValueType::Double) {
      result = std::get<double>(left) > std::get<double>(right);
    } else if (vtleft == ValueType::Integer && vtright == ValueType::Double) {
      result =
          static_cast<double>(std::get<int>(left)) > std::get<double>(right);
    } else if (vtleft == ValueType::Double && vtright == ValueType::Integer) {
      result =
          std::get<double>(left) > static_cast<double>(std::get<int>(right));
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_gte_comparison(const Token& token, ValueType vtleft,
                          ValueType vtright, Value left, Value right) {
    Value result;

    if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
      result = std::get<int>(left) >= std::get<int>(right);
    } else if (vtleft == ValueType::Double && vtright == ValueType::Double) {
      result = std::get<double>(left) >= std::get<double>(right);
    } else if (vtleft == ValueType::Integer && vtright == ValueType::Double) {
      result =
          static_cast<double>(std::get<int>(left)) >= std::get<double>(right);
    } else if (vtleft == ValueType::Double && vtright == ValueType::Integer) {
      result =
          std::get<double>(left) >= static_cast<double>(std::get<int>(right));
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_bitwise_and(const Token& token, ValueType vtleft, ValueType vtright,
                       Value left, Value right) {
    Value result;

    if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
      result = std::get<int>(left) & std::get<int>(right);
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_bitwise_or(const Token& token, ValueType vtleft, ValueType vtright,
                      Value left, Value right) {
    Value result;

    if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
      result = std::get<int>(left) | std::get<int>(right);
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_bitwise_xor(const Token& token, ValueType vtleft, ValueType vtright,
                       Value left, Value right) {
    Value result;

    if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
      result = std::get<int>(left) ^ std::get<int>(right);
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_bitwise_not(const Token& token, ValueType vtleft, Value left) {
    Value result;

    if (vtleft == ValueType::Integer) {
      result = ~std::get<int>(left);
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_bitwise_lshift(const Token& token, ValueType vtleft,
                          ValueType vtright, Value left, Value right) {
    Value result;

    if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
      result = std::get<int>(left) << std::get<int>(right);
    } else {
      throw ConversionError(token);
    }

    return result;
  }

  Value do_bitwise_rshift(const Token& token, ValueType vtleft,
                          ValueType vtright, Value left, Value right) {
    Value result;

    if (vtleft == ValueType::Integer && vtright == ValueType::Integer) {
      result = std::get<int>(left) >> std::get<int>(right);
    } else {
      throw ConversionError(token);
    }

    return result;
  }

} MathImpl;

#endif