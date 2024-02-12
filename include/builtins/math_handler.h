#ifndef KIWI_BUILTINS_MATHHANDLER_H
#define KIWI_BUILTINS_MATHHANDLER_H

#include <vector>
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "math/functions.h"
#include "typing/valuetype.h"

class MathBuiltinHandler {
 public:
  static Value execute(const Token& tokenTerm, const std::string& builtin,
                       const std::vector<Value>& args) {
    if (builtin == MathBuiltins.Sin) {
      return executeSin(tokenTerm, args);
    } else if (builtin == MathBuiltins.Tan) {
      return executeTan(tokenTerm, args);
    } else if (builtin == MathBuiltins.Cos) {
      return executeCos(tokenTerm, args);
    } else if (builtin == MathBuiltins.Asin) {
      return executeAsin(tokenTerm, args);
    } else if (builtin == MathBuiltins.Acos) {
      return executeAcos(tokenTerm, args);
    } else if (builtin == MathBuiltins.Atan) {
      return executeAtan(tokenTerm, args);
    } else if (builtin == MathBuiltins.Atan2) {
      return executeAtan2(tokenTerm, args);
    } else if (builtin == MathBuiltins.Sinh) {
      return executeSinh(tokenTerm, args);
    } else if (builtin == MathBuiltins.Cosh) {
      return executeCosh(tokenTerm, args);
    } else if (builtin == MathBuiltins.Tanh) {
      return executeTanh(tokenTerm, args);
    } else if (builtin == MathBuiltins.Log) {
      return executeLog(tokenTerm, args);
    } else if (builtin == MathBuiltins.Log2) {
      return executeLog2(tokenTerm, args);
    } else if (builtin == MathBuiltins.Log10) {
      return executeLog10(tokenTerm, args);
    } else if (builtin == MathBuiltins.Log1P) {
      return executeLog1P(tokenTerm, args);
    } else if (builtin == MathBuiltins.Fmod) {
      return executeFmod(tokenTerm, args);
    } else if (builtin == MathBuiltins.Hypot) {
      return executeHypot(tokenTerm, args);
    } else if (builtin == MathBuiltins.IsFinite) {
      return executeIsFinite(tokenTerm, args);
    } else if (builtin == MathBuiltins.IsInf) {
      return executeIsInf(tokenTerm, args);
    } else if (builtin == MathBuiltins.IsNaN) {
      return executeIsNaN(tokenTerm, args);
    } else if (builtin == MathBuiltins.IsNormal) {
      return executeIsNormal(tokenTerm, args);
    } else if (builtin == MathBuiltins.Sqrt) {
      return executeSqrt(tokenTerm, args);
    } else if (builtin == MathBuiltins.Cbrt) {
      return executeCbrt(tokenTerm, args);
    } else if (builtin == MathBuiltins.Abs) {
      return executeAbs(tokenTerm, args);
    } else if (builtin == MathBuiltins.Floor) {
      return executeFloor(tokenTerm, args);
    } else if (builtin == MathBuiltins.Ceil) {
      return executeCeil(tokenTerm, args);
    } else if (builtin == MathBuiltins.Round) {
      return executeRound(tokenTerm, args);
    } else if (builtin == MathBuiltins.Trunc) {
      return executeTrunc(tokenTerm, args);
    } else if (builtin == MathBuiltins.Remainder) {
      return executeRemainder(tokenTerm, args);
    } else if (builtin == MathBuiltins.Exp) {
      return executeExp(tokenTerm, args);
    } else if (builtin == MathBuiltins.ExpM1) {
      return executeExpM1(tokenTerm, args);
    } else if (builtin == MathBuiltins.Erf) {
      return executeErf(tokenTerm, args);
    } else if (builtin == MathBuiltins.ErfC) {
      return executeErfC(tokenTerm, args);
    } else if (builtin == MathBuiltins.LGamma) {
      return executeLGamma(tokenTerm, args);
    } else if (builtin == MathBuiltins.TGamma) {
      return executeTGamma(tokenTerm, args);
    } else if (builtin == MathBuiltins.FMax) {
      return executeFMax(tokenTerm, args);
    } else if (builtin == MathBuiltins.FMin) {
      return executeFMin(tokenTerm, args);
    } else if (builtin == MathBuiltins.FDim) {
      return executeFDim(tokenTerm, args);
    } else if (builtin == MathBuiltins.CopySign) {
      return executeCopySign(tokenTerm, args);
    } else if (builtin == MathBuiltins.NextAfter) {
      return executeNextAfter(tokenTerm, args);
    } else if (builtin == MathBuiltins.Pow) {
      return executePow(tokenTerm, args);
    } else if (builtin == MathBuiltins.Epsilon) {
      return executeEpsilon(tokenTerm, args);
    } else if (builtin == MathBuiltins.Random) {
      return executeRandom(tokenTerm, args);
    }

    throw UnknownBuiltinError(tokenTerm, builtin);
  }

 private:
  static Value executeSin(const Token& tokenTerm,
                          const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Sin);
    }

    return MathImpl.do_sin(tokenTerm, args.at(0));
  }

  static Value executeCos(const Token& tokenTerm,
                          const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Cos);
    }

    return MathImpl.do_cos(tokenTerm, args.at(0));
  }

  static Value executeTan(const Token& tokenTerm,
                          const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Tan);
    }

    return MathImpl.do_tan(tokenTerm, args.at(0));
  }

  static Value executeAsin(const Token& tokenTerm,
                           const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Asin);
    }

    return MathImpl.do_asin(tokenTerm, args.at(0));
  }

  static Value executeAcos(const Token& tokenTerm,
                           const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Acos);
    }

    return MathImpl.do_acos(tokenTerm, args.at(0));
  }

  static Value executeAtan(const Token& tokenTerm,
                           const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Atan);
    }

    return MathImpl.do_atan(tokenTerm, args.at(0));
  }

  static Value executeAtan2(const Token& tokenTerm,
                            const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Atan2);
    }

    return MathImpl.do_atan2(tokenTerm, args.at(0), args.at(1));
  }
  static Value executeSinh(const Token& tokenTerm,
                           const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Sinh);
    }

    return MathImpl.do_sinh(tokenTerm, args.at(0));
  }

  static Value executeCosh(const Token& tokenTerm,
                           const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Cosh);
    }

    return MathImpl.do_cosh(tokenTerm, args.at(0));
  }

  static Value executeTanh(const Token& tokenTerm,
                           const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Tanh);
    }

    return MathImpl.do_tanh(tokenTerm, args.at(0));
  }

  static Value executeLog(const Token& tokenTerm,
                          const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Log);
    }

    return MathImpl.do_log(tokenTerm, args.at(0));
  }

  static Value executeLog2(const Token& tokenTerm,
                           const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Log2);
    }

    return MathImpl.do_log2(tokenTerm, args.at(0));
  }

  static Value executeLog10(const Token& tokenTerm,
                            const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Log10);
    }

    return MathImpl.do_log10(tokenTerm, args.at(0));
  }

  static Value executeLog1P(const Token& tokenTerm,
                            const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Log1P);
    }

    return MathImpl.do_log1p(tokenTerm, args.at(0));
  }

  static Value executeFmod(const Token& tokenTerm,
                           const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Fmod);
    }

    return MathImpl.do_fmod(tokenTerm, args.at(0), args.at(1));
  }

  static Value executeHypot(const Token& tokenTerm,
                            const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Hypot);
    }

    return MathImpl.do_hypot(tokenTerm, args.at(0), args.at(1));
  }

  static Value executeIsFinite(const Token& tokenTerm,
                               const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.IsFinite);
    }

    return MathImpl.do_isfinite(tokenTerm, args.at(0));
  }

  static Value executeIsInf(const Token& tokenTerm,
                            const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.IsInf);
    }

    return MathImpl.do_isinf(tokenTerm, args.at(0));
  }

  static Value executeIsNaN(const Token& tokenTerm,
                            const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.IsNaN);
    }

    return MathImpl.do_isnan(tokenTerm, args.at(0));
  }

  static Value executeIsNormal(const Token& tokenTerm,
                               const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.IsNormal);
    }

    return MathImpl.do_isnormal(tokenTerm, args.at(0));
  }

  static Value executeSqrt(const Token& tokenTerm,
                           const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Sqrt);
    }

    return MathImpl.do_sqrt(tokenTerm, args.at(0));
  }

  static Value executeCbrt(const Token& tokenTerm,
                           const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Cbrt);
    }

    return MathImpl.do_cbrt(tokenTerm, args.at(0));
  }

  static Value executeAbs(const Token& tokenTerm,
                          const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Abs);
    }

    return MathImpl.do_abs(tokenTerm, args.at(0));
  }

  static Value executeFloor(const Token& tokenTerm,
                            const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Floor);
    }

    return MathImpl.do_floor(tokenTerm, args.at(0));
  }

  static Value executeCeil(const Token& tokenTerm,
                           const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Ceil);
    }

    return MathImpl.do_ceil(tokenTerm, args.at(0));
  }

  static Value executeTrunc(const Token& tokenTerm,
                            const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Trunc);
    }

    return MathImpl.do_trunc(tokenTerm, args.at(0));
  }

  static Value executeRound(const Token& tokenTerm,
                            const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Round);
    }

    return MathImpl.do_round(tokenTerm, args.at(0));
  }

  static Value executeRemainder(const Token& tokenTerm,
                                const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Remainder);
    }

    return MathImpl.do_remainder(tokenTerm, args.at(0), args.at(1));
  }

  static Value executeExp(const Token& tokenTerm,
                          const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Remainder);
    }

    return MathImpl.do_exp(tokenTerm, args.at(0));
  }

  static Value executeExpM1(const Token& tokenTerm,
                            const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.ExpM1);
    }

    return MathImpl.do_expm1(tokenTerm, args.at(0));
  }

  static Value executeErf(const Token& tokenTerm,
                          const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Erf);
    }

    return MathImpl.do_erf(tokenTerm, args.at(0));
  }

  static Value executeErfC(const Token& tokenTerm,
                           const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.ErfC);
    }

    return MathImpl.do_erfc(tokenTerm, args.at(0));
  }

  static Value executeLGamma(const Token& tokenTerm,
                             const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.LGamma);
    }

    return MathImpl.do_lgamma(tokenTerm, args.at(0));
  }

  static Value executeTGamma(const Token& tokenTerm,
                             const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.TGamma);
    }

    return MathImpl.do_tgamma(tokenTerm, args.at(0));
  }

  static Value executeFMax(const Token& tokenTerm,
                           const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.FMax);
    }

    return MathImpl.do_max(tokenTerm, args.at(0), args.at(1));
  }

  static Value executeFMin(const Token& tokenTerm,
                           const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.FMin);
    }

    return MathImpl.do_min(tokenTerm, args.at(0), args.at(1));
  }

  static Value executeFDim(const Token& tokenTerm,
                           const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.FDim);
    }

    return MathImpl.do_fdim(tokenTerm, args.at(0), args.at(1));
  }

  static Value executeCopySign(const Token& tokenTerm,
                               const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.CopySign);
    }

    return MathImpl.do_copysign(tokenTerm, args.at(0), args.at(1));
  }

  static Value executeNextAfter(const Token& tokenTerm,
                                const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.NextAfter);
    }

    return MathImpl.do_nextafter(tokenTerm, args.at(0), args.at(1));
  }

  static Value executePow(const Token& tokenTerm,
                          const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Pow);
    }

    return MathImpl.do_pow(tokenTerm, args.at(0), args.at(1));
  }

  static Value executeEpsilon(const Token& tokenTerm,
                              const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Epsilon);
    }

    return MathImpl.epsilon();
  }

  static Value executeRandom(const Token& tokenTerm,
                             const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, MathBuiltins.Random);
    }

    return MathImpl.do_random(tokenTerm, args.at(0), args.at(1));
  }
};

#endif