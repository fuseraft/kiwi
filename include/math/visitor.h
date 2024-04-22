#ifndef KIWI_MATH_VISITOR_H
#define KIWI_MATH_VISITOR_H

#include "parsing/tokens.h"
#include "tracing/error.h"
#include "typing/value.h"
#include "functions.h"

struct AddVisitor {
  const Token& token;

  AddVisitor(const Token& token) : token(token) {}

  k_value operator()(const k_value& left, const k_value& right) const {
    return MathImpl.do_addition(token, left, right);
  }
};

struct SubtractVisitor {
  const Token& token;

  SubtractVisitor(const Token& token) : token(token) {}

  k_value operator()(const k_value& left, const k_value& right) const {
    return MathImpl.do_subtraction(token, left, right);
  }
};

struct MultiplyVisitor {
  const Token& token;

  MultiplyVisitor(const Token& token) : token(token) {}

  k_value operator()(const k_value& left, const k_value& right) const {
    return MathImpl.do_multiplication(token, left, right);
  }
};

struct DivideVisitor {
  const Token& token;

  DivideVisitor(const Token& token) : token(token) {}

  k_value operator()(const k_value& left, const k_value& right) const {
    if (MathImpl.is_zero(token, right)) {
      throw DivideByZeroError(token);
    } else {
      return MathImpl.do_division(token, left, right);
    }
  }
};

struct PowerVisitor {
  const Token& token;

  PowerVisitor(const Token& token) : token(token) {}

  k_value operator()(const k_value& left, const k_value& right) const {
    return MathImpl.do_exponentiation(token, left, right);
  }
};

struct ModuloVisitor {
  const Token& token;

  ModuloVisitor(const Token& token) : token(token) {}

  k_value operator()(const k_value& left, const k_value& right) const {
    return MathImpl.do_modulus(token, left, right);
  }
};

struct EqualityVisitor {
  EqualityVisitor() {}

  k_value operator()(const k_value& left, const k_value& right) const {
    return MathImpl.do_eq_comparison(left, right);
  }
};

struct InequalityVisitor {
  InequalityVisitor() {}

  k_value operator()(const k_value& left, const k_value& right) const {
    return MathImpl.do_neq_comparison(left, right);
  }
};

struct LessThanVisitor {
  LessThanVisitor() {}

  k_value operator()(const k_value& left, const k_value& right) const {
    return MathImpl.do_lt_comparison(left, right);
  }
};

struct LessThanOrEqualVisitor {
  LessThanOrEqualVisitor() {}

  k_value operator()(const k_value& left, const k_value& right) const {
    return MathImpl.do_lte_comparison(left, right);
  }
};

struct GreaterThanVisitor {
  GreaterThanVisitor() {}

  k_value operator()(const k_value& left, const k_value& right) const {
    return MathImpl.do_gt_comparison(left, right);
  }
};

struct GreaterThanOrEqualVisitor {
  GreaterThanOrEqualVisitor() {}

  k_value operator()(const k_value& left, const k_value& right) const {
    return MathImpl.do_gte_comparison(left, right);
  }
};

struct BitwiseAndVisitor {
  const Token& token;

  BitwiseAndVisitor(const Token& token) : token(token) {}

  k_value operator()(const k_value& left, const k_value& right) const {
    return MathImpl.do_bitwise_and(token, left, right);
  }
};

struct BitwiseOrVisitor {
  const Token& token;

  BitwiseOrVisitor(const Token& token) : token(token) {}

  k_value operator()(const k_value& left, const k_value& right) const {
    return MathImpl.do_bitwise_or(token, left, right);
  }
};

struct BitwiseXorVisitor {
  const Token& token;

  BitwiseXorVisitor(const Token& token) : token(token) {}

  k_value operator()(const k_value& left, const k_value& right) const {
    return MathImpl.do_bitwise_xor(token, left, right);
  }
};

struct BitwiseNotVisitor {
  const Token& token;

  BitwiseNotVisitor(const Token& token) : token(token) {}

  k_value operator()(const k_value& left) const {
    return MathImpl.do_bitwise_not(token, left);
  }
};

struct BitwiseLeftShiftVisitor {
  const Token& token;

  BitwiseLeftShiftVisitor(const Token& token) : token(token) {}

  k_value operator()(const k_value& left, const k_value& right) const {
    return MathImpl.do_bitwise_lshift(token, left, right);
  }
};

struct BitwiseRightShiftVisitor {
  const Token& token;

  BitwiseRightShiftVisitor(const Token& token) : token(token) {}

  k_value operator()(const k_value& left, const k_value& right) const {
    return MathImpl.do_bitwise_rshift(token, left, right);
  }
};

struct NegateSignVisitor {
  const Token& token;

  NegateSignVisitor(const Token& token) : token(token) {}

  k_value operator()(const k_value& value) const {
    if (std::holds_alternative<k_int>(value)) {
      return -std::get<k_int>(value);
    } else if (std::holds_alternative<double>(value)) {
      return -std::get<double>(value);
    } else {
      throw ConversionError(
          token,
          "Unary minus applied to a non-numeric value.");
    }
  }
};

struct NegateVisitor {
  const Token& token;

  NegateVisitor(const Token& token) : token(token) {}

  k_value operator()(const k_value& value) const {
    if (std::holds_alternative<bool>(value)) {
      return !std::get<bool>(value);
    } else if (std::holds_alternative<k_int>(value)) {
      return static_cast<k_int>(std::get<k_int>(value) == 0 ? 1 : 0);
    } else if (std::holds_alternative<double>(value)) {
      return std::get<double>(value) == 0;
    } else if (std::holds_alternative<k_string>(value)) {
      return std::get<k_string>(value).empty();
    } else if (std::holds_alternative<k_list>(value)) {
      return std::get<k_list>(value)->elements.empty();
    } else if (std::holds_alternative<k_hash>(value)) {
      return std::get<k_hash>(value)->keys.empty();
    } else {
      throw ConversionError(token,
                            "Unexpected value.");
    }
  }
};

#endif