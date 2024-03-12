#ifndef KIWI_BUILTINS_MATHHANDLER_H
#define KIWI_BUILTINS_MATHHANDLER_H

#include <vector>
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "math/functions.h"
#include "math/primes.h"
#include "typing/valuetype.h"

class MathBuiltinHandler {
 public:
  static Value execute(const Token& term, const SubTokenType& builtin,
                       const std::vector<Value>& args) {
    if (builtin == SubTokenType::Builtin_Math_Sin) {
      return executeSin(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Tan) {
      return executeTan(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Cos) {
      return executeCos(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Asin) {
      return executeAsin(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Acos) {
      return executeAcos(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Atan) {
      return executeAtan(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Atan2) {
      return executeAtan2(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Sinh) {
      return executeSinh(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Cosh) {
      return executeCosh(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Tanh) {
      return executeTanh(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Log) {
      return executeLog(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Log2) {
      return executeLog2(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Log10) {
      return executeLog10(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Log1P) {
      return executeLog1P(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Fmod) {
      return executeFmod(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Hypot) {
      return executeHypot(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_IsFinite) {
      return executeIsFinite(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_IsInf) {
      return executeIsInf(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_IsNaN) {
      return executeIsNaN(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_IsNormal) {
      return executeIsNormal(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Sqrt) {
      return executeSqrt(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Cbrt) {
      return executeCbrt(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Abs) {
      return executeAbs(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Floor) {
      return executeFloor(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Ceil) {
      return executeCeil(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Round) {
      return executeRound(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Trunc) {
      return executeTrunc(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Remainder) {
      return executeRemainder(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Exp) {
      return executeExp(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_ExpM1) {
      return executeExpM1(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Erf) {
      return executeErf(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_ErfC) {
      return executeErfC(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_LGamma) {
      return executeLGamma(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_TGamma) {
      return executeTGamma(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_FMax) {
      return executeFMax(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_FMin) {
      return executeFMin(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_FDim) {
      return executeFDim(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_CopySign) {
      return executeCopySign(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_NextAfter) {
      return executeNextAfter(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Pow) {
      return executePow(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Epsilon) {
      return executeEpsilon(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_Random) {
      return executeRandom(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_ListPrimes) {
      return executeListPrimes(term, args);
    } else if (builtin == SubTokenType::Builtin_Math_NthPrime) {
      return executeNthPrime(term, args);
    }

    throw UnknownBuiltinError(term, "");
  }

 private:
  static Value executeSin(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Sin);
    }

    return MathImpl.do_sin(term, args.at(0));
  }

  static Value executeCos(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Cos);
    }

    return MathImpl.do_cos(term, args.at(0));
  }

  static Value executeTan(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Tan);
    }

    return MathImpl.do_tan(term, args.at(0));
  }

  static Value executeAsin(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Asin);
    }

    return MathImpl.do_asin(term, args.at(0));
  }

  static Value executeAcos(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Acos);
    }

    return MathImpl.do_acos(term, args.at(0));
  }

  static Value executeAtan(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Atan);
    }

    return MathImpl.do_atan(term, args.at(0));
  }

  static Value executeAtan2(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Atan2);
    }

    return MathImpl.do_atan2(term, args.at(0), args.at(1));
  }
  static Value executeSinh(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Sinh);
    }

    return MathImpl.do_sinh(term, args.at(0));
  }

  static Value executeCosh(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Cosh);
    }

    return MathImpl.do_cosh(term, args.at(0));
  }

  static Value executeTanh(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Tanh);
    }

    return MathImpl.do_tanh(term, args.at(0));
  }

  static Value executeLog(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Log);
    }

    return MathImpl.do_log(term, args.at(0));
  }

  static Value executeLog2(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Log2);
    }

    return MathImpl.do_log2(term, args.at(0));
  }

  static Value executeLog10(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Log10);
    }

    return MathImpl.do_log10(term, args.at(0));
  }

  static Value executeLog1P(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Log1P);
    }

    return MathImpl.do_log1p(term, args.at(0));
  }

  static Value executeFmod(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Fmod);
    }

    return MathImpl.do_fmod(term, args.at(0), args.at(1));
  }

  static Value executeHypot(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Hypot);
    }

    return MathImpl.do_hypot(term, args.at(0), args.at(1));
  }

  static Value executeIsFinite(const Token& term,
                               const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.IsFinite);
    }

    return MathImpl.do_isfinite(term, args.at(0));
  }

  static Value executeIsInf(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.IsInf);
    }

    return MathImpl.do_isinf(term, args.at(0));
  }

  static Value executeIsNaN(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.IsNaN);
    }

    return MathImpl.do_isnan(term, args.at(0));
  }

  static Value executeIsNormal(const Token& term,
                               const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.IsNormal);
    }

    return MathImpl.do_isnormal(term, args.at(0));
  }

  static Value executeSqrt(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Sqrt);
    }

    return MathImpl.do_sqrt(term, args.at(0));
  }

  static Value executeCbrt(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Cbrt);
    }

    return MathImpl.do_cbrt(term, args.at(0));
  }

  static Value executeAbs(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Abs);
    }

    return MathImpl.do_abs(term, args.at(0));
  }

  static Value executeFloor(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Floor);
    }

    return MathImpl.do_floor(term, args.at(0));
  }

  static Value executeCeil(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Ceil);
    }

    return MathImpl.do_ceil(term, args.at(0));
  }

  static Value executeTrunc(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Trunc);
    }

    return MathImpl.do_trunc(term, args.at(0));
  }

  static Value executeRound(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Round);
    }

    return MathImpl.do_round(term, args.at(0));
  }

  static Value executeRemainder(const Token& term,
                                const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Remainder);
    }

    return MathImpl.do_remainder(term, args.at(0), args.at(1));
  }

  static Value executeExp(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Remainder);
    }

    return MathImpl.do_exp(term, args.at(0));
  }

  static Value executeExpM1(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.ExpM1);
    }

    return MathImpl.do_expm1(term, args.at(0));
  }

  static Value executeErf(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Erf);
    }

    return MathImpl.do_erf(term, args.at(0));
  }

  static Value executeErfC(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.ErfC);
    }

    return MathImpl.do_erfc(term, args.at(0));
  }

  static Value executeLGamma(const Token& term,
                             const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.LGamma);
    }

    return MathImpl.do_lgamma(term, args.at(0));
  }

  static Value executeTGamma(const Token& term,
                             const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.TGamma);
    }

    return MathImpl.do_tgamma(term, args.at(0));
  }

  static Value executeFMax(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.FMax);
    }

    return MathImpl.do_max(term, args.at(0), args.at(1));
  }

  static Value executeFMin(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.FMin);
    }

    return MathImpl.do_min(term, args.at(0), args.at(1));
  }

  static Value executeFDim(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.FDim);
    }

    return MathImpl.do_fdim(term, args.at(0), args.at(1));
  }

  static Value executeCopySign(const Token& term,
                               const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.CopySign);
    }

    return MathImpl.do_copysign(term, args.at(0), args.at(1));
  }

  static Value executeNextAfter(const Token& term,
                                const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.NextAfter);
    }

    return MathImpl.do_nextafter(term, args.at(0), args.at(1));
  }

  static Value executePow(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Pow);
    }

    return MathImpl.do_pow(term, args.at(0), args.at(1));
  }

  static Value executeEpsilon(const Token& term,
                              const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Epsilon);
    }

    return MathImpl.epsilon();
  }

  static Value executeListPrimes(const Token& term,
                                 const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.ListPrimes);
    }

    if (!std::holds_alternative<k_int>(args.at(0))) {
      throw ConversionError(term,
                            "Expected an `Integer` argument for builtin `" +
                                MathBuiltins.ListPrimes + "`.");
    }

    auto primes = PrimeGenerator::listPrimes(std::get<k_int>(args.at(0)));
    auto list = std::make_shared<List>();

    for (const auto& prime : primes) {
      list->elements.push_back(static_cast<k_int>(prime));
    }

    return list;
  }

  static Value executeNthPrime(const Token& term,
                               const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.NthPrime);
    }

    if (!std::holds_alternative<k_int>(args.at(0))) {
      throw ConversionError(term,
                            "Expected an `Integer` argument for builtin `" +
                                MathBuiltins.NthPrime + "`.");
    }

    auto nthPrime = PrimeGenerator::nthPrime(std::get<k_int>(args.at(0)));

    return static_cast<k_int>(nthPrime);
  }

  static Value executeRandom(const Token& term,
                             const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Random);
    }

    return MathImpl.do_random(term, args.at(0), args.at(1));
  }
};

#endif