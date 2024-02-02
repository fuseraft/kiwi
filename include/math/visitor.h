#ifndef KIWI_MATH_VISITOR_H
#define KIWI_MATH_VISITOR_H

#include "errors/error.h"
#include "parsing/tokens.h"
#include "typing/valuetype.h"
#include "functions.h"

struct AddVisitor {
  const Token& token;

  AddVisitor(const Token& token) : token(token) {}

  Value operator()(Value left, Value right) const {
    return MathImpl.do_addition(token, left, right);
  }
};

struct SubtractVisitor {
  const Token& token;

  SubtractVisitor(const Token& token) : token(token) {}

  Value operator()(Value left, Value right) const {
    return MathImpl.do_subtraction(token, left, right);
  }
};

struct MultiplyVisitor {
  const Token& token;

  MultiplyVisitor(const Token& token) : token(token) {}

  Value operator()(Value left, Value right) const {
    return MathImpl.do_multiplication(token, left, right);
  }
};

struct DivideVisitor {
  const Token& token;

  DivideVisitor(const Token& token) : token(token) {}

  Value operator()(Value left, Value right) const {
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

  Value operator()(Value left, Value right) const {
    return MathImpl.do_exponentiation(token, left, right);
  }
};

struct ModuloVisitor {
  const Token& token;

  ModuloVisitor(const Token& token) : token(token) {}

  Value operator()(Value left, Value right) const {
    return MathImpl.do_modulus(token, left, right);
  }
};

struct EqualityVisitor {
  const Token& token;

  EqualityVisitor(const Token& token) : token(token) {}

  Value operator()(Value left, Value right) const {
    return MathImpl.do_eq_comparison(token, left, right);
  }
};

struct InequalityVisitor {
  const Token& token;

  InequalityVisitor(const Token& token) : token(token) {}

  Value operator()(Value left, Value right) const {
    return MathImpl.do_neq_comparison(token, left, right);
  }
};

struct LessThanVisitor {
  const Token& token;

  LessThanVisitor(const Token& token) : token(token) {}

  Value operator()(Value left, Value right) const {
    return MathImpl.do_lt_comparison(token, left, right);
  }
};

struct LessThanOrEqualVisitor {
  const Token& token;

  LessThanOrEqualVisitor(const Token& token) : token(token) {}

  Value operator()(Value left, Value right) const {
    return MathImpl.do_lte_comparison(token, left, right);
  }
};

struct GreaterThanVisitor {
  const Token& token;

  GreaterThanVisitor(const Token& token) : token(token) {}

  Value operator()(Value left, Value right) const {
    return MathImpl.do_gt_comparison(token, left, right);
  }
};

struct GreaterThanOrEqualVisitor {
  const Token& token;

  GreaterThanOrEqualVisitor(const Token& token) : token(token) {}

  Value operator()(Value left, Value right) const {
    return MathImpl.do_gte_comparison(token, left, right);
  }
};

struct BitwiseAndVisitor {
  const Token& token;

  BitwiseAndVisitor(const Token& token) : token(token) {}

  Value operator()(Value left, Value right) const {
    return MathImpl.do_bitwise_and(token, left, right);
  }
};

struct BitwiseOrVisitor {
  const Token& token;

  BitwiseOrVisitor(const Token& token) : token(token) {}

  Value operator()(Value left, Value right) const {
    return MathImpl.do_bitwise_or(token, left, right);
  }
};

struct BitwiseXorVisitor {
  const Token& token;

  BitwiseXorVisitor(const Token& token) : token(token) {}

  Value operator()(Value left, Value right) const {
    return MathImpl.do_bitwise_xor(token, left, right);
  }
};

struct BitwiseNotVisitor {
  const Token& token;

  BitwiseNotVisitor(const Token& token) : token(token) {}

  Value operator()(Value left) const {
    return MathImpl.do_bitwise_not(token, left);
  }
};

struct BitwiseLeftShiftVisitor {
  const Token& token;

  BitwiseLeftShiftVisitor(const Token& token) : token(token) {}

  Value operator()(Value left, Value right) const {
    return MathImpl.do_bitwise_lshift(token, left, right);
  }
};

struct BitwiseRightShiftVisitor {
  const Token& token;

  BitwiseRightShiftVisitor(const Token& token) : token(token) {}

  Value operator()(Value left, Value right) const {
    return MathImpl.do_bitwise_rshift(token, left, right);
  }
};

#endif