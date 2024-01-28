#ifndef KIWI_MATH_VISITOR_H
#define KIWI_MATH_VISITOR_H

#include <variant>
#include "errors/error.h"
#include "parsing/tokens.h"
#include "typing/valuetype.h"
#include "functions.h"

struct AddVisitor {
  const Token& token;

  AddVisitor(const Token& token) : token(token) {}

  std::variant<int, double, bool, std::string> operator()(
      std::variant<int, double, bool, std::string> left,
      std::variant<int, double, bool, std::string> right) const {
    ValueType vtleft = get_value_type(left);
    ValueType vtright = get_value_type(right);

    return MathImpl.do_addition(token, vtleft, vtright, left, right);
  }
};

struct SubtractVisitor {
  const Token& token;

  SubtractVisitor(const Token& token) : token(token) {}

  std::variant<int, double, bool, std::string> operator()(
      std::variant<int, double, bool, std::string> left,
      std::variant<int, double, bool, std::string> right) const {
    ValueType vtleft = get_value_type(left);
    ValueType vtright = get_value_type(right);

    if (vtright == ValueType::Integer || vtright == ValueType::Double) {
      return MathImpl.do_subtraction(token, vtleft, vtright, left, right);
    } else {
      throw ConversionError(token);
    }
  }
};

struct MultiplyVisitor {
  const Token& token;

  MultiplyVisitor(const Token& token) : token(token) {}

  std::variant<int, double, bool, std::string> operator()(
      std::variant<int, double, bool, std::string> left,
      std::variant<int, double, bool, std::string> right) const {
    ValueType vtleft = get_value_type(left);
    ValueType vtright = get_value_type(right);

    return MathImpl.do_multiplication(token, vtleft, vtright, left, right);
  }
};

struct DivideVisitor {
  const Token& token;

  DivideVisitor(const Token& token) : token(token) {}

  std::variant<int, double, bool, std::string> operator()(
      std::variant<int, double, bool, std::string> left,
      std::variant<int, double, bool, std::string> right) const {
    ValueType vtleft = get_value_type(left);
    ValueType vtright = get_value_type(right);

    if (vtright == ValueType::Integer || vtright == ValueType::Double) {
      if (MathImpl.is_zero(vtright, right)) {
        throw DivideByZeroError(token);
      } else {
        return MathImpl.do_division(token, vtleft, vtright, left, right);
      }
    } else {
      throw ConversionError(token);
    }
  }
};

struct PowerVisitor {
  const Token& token;

  PowerVisitor(const Token& token) : token(token) {}

  std::variant<int, double, bool, std::string> operator()(
      std::variant<int, double, bool, std::string> left,
      std::variant<int, double, bool, std::string> right) const {
    ValueType vtleft = get_value_type(left);
    ValueType vtright = get_value_type(right);

    if (vtright == ValueType::Integer || vtright == ValueType::Double) {
      return MathImpl.do_exponentiation(token, vtleft, vtright, left, right);
    } else {
      throw ConversionError(token);
    }
  }
};

struct ModuloVisitor {
  const Token& token;

  ModuloVisitor(const Token& token) : token(token) {}

  std::variant<int, double, bool, std::string> operator()(
      std::variant<int, double, bool, std::string> left,
      std::variant<int, double, bool, std::string> right) const {
    ValueType vtleft = get_value_type(left);
    ValueType vtright = get_value_type(right);

    if (vtright == ValueType::Integer || vtright == ValueType::Double) {
      return MathImpl.do_modulus(token, vtleft, vtright, left, right);
    } else {
      throw ConversionError(token);
    }
  }
};

struct EqualityVisitor {
  const Token& token;

  EqualityVisitor(const Token& token) : token(token) {}

  std::variant<int, double, bool, std::string> operator()(
      std::variant<int, double, bool, std::string> left,
      std::variant<int, double, bool, std::string> right) const {
    ValueType vtleft = get_value_type(left);
    ValueType vtright = get_value_type(right);

    return MathImpl.do_eq_comparison(token, vtleft, vtright, left, right);
  }
};

struct InequalityVisitor {
  const Token& token;

  InequalityVisitor(const Token& token) : token(token) {}

  std::variant<int, double, bool, std::string> operator()(
      std::variant<int, double, bool, std::string> left,
      std::variant<int, double, bool, std::string> right) const {
    ValueType vtleft = get_value_type(left);
    ValueType vtright = get_value_type(right);

    return MathImpl.do_neq_comparison(token, vtleft, vtright, left, right);
  }
};

struct LessThanVisitor {
  const Token& token;

  LessThanVisitor(const Token& token) : token(token) {}

  std::variant<int, double, bool, std::string> operator()(
      std::variant<int, double, bool, std::string> left,
      std::variant<int, double, bool, std::string> right) const {
    ValueType vtleft = get_value_type(left);
    ValueType vtright = get_value_type(right);

    return MathImpl.do_lt_comparison(token, vtleft, vtright, left, right);
  }
};

struct LessThanOrEqualVisitor {
  const Token& token;

  LessThanOrEqualVisitor(const Token& token) : token(token) {}

  std::variant<int, double, bool, std::string> operator()(
      std::variant<int, double, bool, std::string> left,
      std::variant<int, double, bool, std::string> right) const {
    ValueType vtleft = get_value_type(left);
    ValueType vtright = get_value_type(right);

    return MathImpl.do_lte_comparison(token, vtleft, vtright, left, right);
  }
};

struct GreaterThanVisitor {
  const Token& token;

  GreaterThanVisitor(const Token& token) : token(token) {}

  std::variant<int, double, bool, std::string> operator()(
      std::variant<int, double, bool, std::string> left,
      std::variant<int, double, bool, std::string> right) const {
    ValueType vtleft = get_value_type(left);
    ValueType vtright = get_value_type(right);

    return MathImpl.do_gt_comparison(token, vtleft, vtright, left, right);
  }
};

struct GreaterThanOrEqualVisitor {
  const Token& token;

  GreaterThanOrEqualVisitor(const Token& token) : token(token) {}

  std::variant<int, double, bool, std::string> operator()(
      std::variant<int, double, bool, std::string> left,
      std::variant<int, double, bool, std::string> right) const {
    ValueType vtleft = get_value_type(left);
    ValueType vtright = get_value_type(right);

    return MathImpl.do_gte_comparison(token, vtleft, vtright, left, right);
  }
};

struct BitwiseAndVisitor {
  const Token& token;

  BitwiseAndVisitor(const Token& token) : token(token) {}

  std::variant<int, double, bool, std::string> operator()(
      std::variant<int, double, bool, std::string> left,
      std::variant<int, double, bool, std::string> right) const {
    ValueType vtleft = get_value_type(left);
    ValueType vtright = get_value_type(right);

    return MathImpl.do_bitwise_and(token, vtleft, vtright, left, right);
  }
};

struct BitwiseOrVisitor {
  const Token& token;

  BitwiseOrVisitor(const Token& token) : token(token) {}

  std::variant<int, double, bool, std::string> operator()(
      std::variant<int, double, bool, std::string> left,
      std::variant<int, double, bool, std::string> right) const {
    ValueType vtleft = get_value_type(left);
    ValueType vtright = get_value_type(right);

    return MathImpl.do_bitwise_or(token, vtleft, vtright, left, right);
  }
};

struct BitwiseXorVisitor {
  const Token& token;

  BitwiseXorVisitor(const Token& token) : token(token) {}

  std::variant<int, double, bool, std::string> operator()(
      std::variant<int, double, bool, std::string> left,
      std::variant<int, double, bool, std::string> right) const {
    ValueType vtleft = get_value_type(left);
    ValueType vtright = get_value_type(right);

    return MathImpl.do_bitwise_xor(token, vtleft, vtright, left, right);
  }
};

struct BitwiseNotVisitor {
  const Token& token;

  BitwiseNotVisitor(const Token& token) : token(token) {}

  std::variant<int, double, bool, std::string> operator()(
      std::variant<int, double, bool, std::string> left) const {
    ValueType vtleft = get_value_type(left);
    
    return MathImpl.do_bitwise_not(token, vtleft, left);
  }
};

struct BitwiseLeftShiftVisitor {
  const Token& token;

  BitwiseLeftShiftVisitor(const Token& token) : token(token) {}

  std::variant<int, double, bool, std::string> operator()(
      std::variant<int, double, bool, std::string> left,
      std::variant<int, double, bool, std::string> right) const {
    ValueType vtleft = get_value_type(left);
    ValueType vtright = get_value_type(right);

    return MathImpl.do_bitwise_lshift(token, vtleft, vtright, left, right);
  }
};

struct BitwiseRightShiftVisitor {
  const Token& token;

  BitwiseRightShiftVisitor(const Token& token) : token(token) {}

  std::variant<int, double, bool, std::string> operator()(
      std::variant<int, double, bool, std::string> left,
      std::variant<int, double, bool, std::string> right) const {
    ValueType vtleft = get_value_type(left);
    ValueType vtright = get_value_type(right);

    return MathImpl.do_bitwise_rshift(token, vtleft, vtright, left, right);
  }
};

#endif