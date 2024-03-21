#ifndef KIWI_BUILTINS_MATHHANDLER_H
#define KIWI_BUILTINS_MATHHANDLER_H

#include <vector>
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "math/functions.h"
#include "math/primes.h"
#include "typing/value.h"

class MathBuiltinHandler {
 public:
  static k_value execute(const Token& term, const KName& builtin,
                       const std::vector<k_value>& args) {
    switch (builtin) {
      case KName::Builtin_Math_Sin:
        return executeSin(term, args);

      case KName::Builtin_Math_Tan:
        return executeTan(term, args);

      case KName::Builtin_Math_Cos:
        return executeCos(term, args);

      case KName::Builtin_Math_Asin:
        return executeAsin(term, args);

      case KName::Builtin_Math_Acos:
        return executeAcos(term, args);

      case KName::Builtin_Math_Atan:
        return executeAtan(term, args);

      case KName::Builtin_Math_Atan2:
        return executeAtan2(term, args);

      case KName::Builtin_Math_Sinh:
        return executeSinh(term, args);

      case KName::Builtin_Math_Cosh:
        return executeCosh(term, args);

      case KName::Builtin_Math_Tanh:
        return executeTanh(term, args);

      case KName::Builtin_Math_Log:
        return executeLog(term, args);

      case KName::Builtin_Math_Log2:
        return executeLog2(term, args);

      case KName::Builtin_Math_Log10:
        return executeLog10(term, args);

      case KName::Builtin_Math_Log1P:
        return executeLog1P(term, args);

      case KName::Builtin_Math_Fmod:
        return executeFmod(term, args);

      case KName::Builtin_Math_Hypot:
        return executeHypot(term, args);

      case KName::Builtin_Math_IsFinite:
        return executeIsFinite(term, args);

      case KName::Builtin_Math_IsInf:
        return executeIsInf(term, args);

      case KName::Builtin_Math_IsNaN:
        return executeIsNaN(term, args);

      case KName::Builtin_Math_IsNormal:
        return executeIsNormal(term, args);

      case KName::Builtin_Math_Sqrt:
        return executeSqrt(term, args);

      case KName::Builtin_Math_Cbrt:
        return executeCbrt(term, args);

      case KName::Builtin_Math_Abs:
        return executeAbs(term, args);

      case KName::Builtin_Math_Floor:
        return executeFloor(term, args);

      case KName::Builtin_Math_Ceil:
        return executeCeil(term, args);

      case KName::Builtin_Math_Round:
        return executeRound(term, args);

      case KName::Builtin_Math_Trunc:
        return executeTrunc(term, args);

      case KName::Builtin_Math_Remainder:
        return executeRemainder(term, args);

      case KName::Builtin_Math_Exp:
        return executeExp(term, args);

      case KName::Builtin_Math_ExpM1:
        return executeExpM1(term, args);

      case KName::Builtin_Math_Erf:
        return executeErf(term, args);

      case KName::Builtin_Math_ErfC:
        return executeErfC(term, args);

      case KName::Builtin_Math_LGamma:
        return executeLGamma(term, args);

      case KName::Builtin_Math_TGamma:
        return executeTGamma(term, args);

      case KName::Builtin_Math_FMax:
        return executeFMax(term, args);

      case KName::Builtin_Math_FMin:
        return executeFMin(term, args);

      case KName::Builtin_Math_FDim:
        return executeFDim(term, args);

      case KName::Builtin_Math_CopySign:
        return executeCopySign(term, args);

      case KName::Builtin_Math_NextAfter:
        return executeNextAfter(term, args);

      case KName::Builtin_Math_Pow:
        return executePow(term, args);

      case KName::Builtin_Math_Epsilon:
        return executeEpsilon(term, args);

      case KName::Builtin_Math_Random:
        return executeRandom(term, args);

      case KName::Builtin_Math_ListPrimes:
        return executeListPrimes(term, args);

      case KName::Builtin_Math_NthPrime:
        return executeNthPrime(term, args);

      case KName::Builtin_Math_Divisors:
        return executeDivisors(term, args);

      default:
        break;
    }

    throw UnknownBuiltinError(term, "");
  }

 private:
  static k_value executeSin(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Sin);
    }

    return MathImpl.__sin__(term, args.at(0));
  }

  static k_value executeCos(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Cos);
    }

    return MathImpl.__cos__(term, args.at(0));
  }

  static k_value executeTan(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Tan);
    }

    return MathImpl.__tan__(term, args.at(0));
  }

  static k_value executeAsin(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Asin);
    }

    return MathImpl.__asin__(term, args.at(0));
  }

  static k_value executeAcos(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Acos);
    }

    return MathImpl.__acos__(term, args.at(0));
  }

  static k_value executeAtan(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Atan);
    }

    return MathImpl.__atan__(term, args.at(0));
  }

  static k_value executeAtan2(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Atan2);
    }

    return MathImpl.__atan2__(term, args.at(0), args.at(1));
  }
  static k_value executeSinh(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Sinh);
    }

    return MathImpl.__sinh__(term, args.at(0));
  }

  static k_value executeCosh(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Cosh);
    }

    return MathImpl.__cosh__(term, args.at(0));
  }

  static k_value executeTanh(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Tanh);
    }

    return MathImpl.__tanh__(term, args.at(0));
  }

  static k_value executeLog(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Log);
    }

    return MathImpl.__log__(term, args.at(0));
  }

  static k_value executeLog2(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Log2);
    }

    return MathImpl.__log2__(term, args.at(0));
  }

  static k_value executeLog10(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Log10);
    }

    return MathImpl.__log10__(term, args.at(0));
  }

  static k_value executeLog1P(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Log1P);
    }

    return MathImpl.__log1p__(term, args.at(0));
  }

  static k_value executeFmod(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Fmod);
    }

    return MathImpl.__fmod__(term, args.at(0), args.at(1));
  }

  static k_value executeHypot(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Hypot);
    }

    return MathImpl.__hypot__(term, args.at(0), args.at(1));
  }

  static k_value executeIsFinite(const Token& term,
                               const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.IsFinite);
    }

    return MathImpl.__isfinite__(term, args.at(0));
  }

  static k_value executeIsInf(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.IsInf);
    }

    return MathImpl.__isinf__(term, args.at(0));
  }

  static k_value executeIsNaN(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.IsNaN);
    }

    return MathImpl.__isnan__(term, args.at(0));
  }

  static k_value executeIsNormal(const Token& term,
                               const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.IsNormal);
    }

    return MathImpl.__isnormal__(term, args.at(0));
  }

  static k_value executeSqrt(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Sqrt);
    }

    return MathImpl.__sqrt__(term, args.at(0));
  }

  static k_value executeCbrt(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Cbrt);
    }

    return MathImpl.__cbrt__(term, args.at(0));
  }

  static k_value executeAbs(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Abs);
    }

    return MathImpl.__abs__(term, args.at(0));
  }

  static k_value executeFloor(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Floor);
    }

    return MathImpl.__floor__(term, args.at(0));
  }

  static k_value executeCeil(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Ceil);
    }

    return MathImpl.__ceil__(term, args.at(0));
  }

  static k_value executeTrunc(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Trunc);
    }

    return MathImpl.__trunc__(term, args.at(0));
  }

  static k_value executeRound(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Round);
    }

    return MathImpl.__round__(term, args.at(0));
  }

  static k_value executeRemainder(const Token& term,
                                const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Remainder);
    }

    return MathImpl.__remainder__(term, args.at(0), args.at(1));
  }

  static k_value executeExp(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Remainder);
    }

    return MathImpl.__exp__(term, args.at(0));
  }

  static k_value executeExpM1(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.ExpM1);
    }

    return MathImpl.__expm1__(term, args.at(0));
  }

  static k_value executeErf(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Erf);
    }

    return MathImpl.__erf__(term, args.at(0));
  }

  static k_value executeErfC(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.ErfC);
    }

    return MathImpl.__erfc__(term, args.at(0));
  }

  static k_value executeLGamma(const Token& term,
                             const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.LGamma);
    }

    return MathImpl.__lgamma__(term, args.at(0));
  }

  static k_value executeTGamma(const Token& term,
                             const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.TGamma);
    }

    return MathImpl.__tgamma__(term, args.at(0));
  }

  static k_value executeFMax(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.FMax);
    }

    return MathImpl.__max__(term, args.at(0), args.at(1));
  }

  static k_value executeFMin(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.FMin);
    }

    return MathImpl.__min__(term, args.at(0), args.at(1));
  }

  static k_value executeFDim(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.FDim);
    }

    return MathImpl.__fdim__(term, args.at(0), args.at(1));
  }

  static k_value executeCopySign(const Token& term,
                               const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.CopySign);
    }

    return MathImpl.__copysign__(term, args.at(0), args.at(1));
  }

  static k_value executeNextAfter(const Token& term,
                                const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.NextAfter);
    }

    return MathImpl.__nextafter__(term, args.at(0), args.at(1));
  }

  static k_value executePow(const Token& term, const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Pow);
    }

    return MathImpl.__pow__(term, args.at(0), args.at(1));
  }

  static k_value executeEpsilon(const Token& term,
                              const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Epsilon);
    }

    return MathImpl.__epsilon__();
  }

  static k_value executeListPrimes(const Token& term,
                                 const std::vector<k_value>& args) {
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

  static k_value executeNthPrime(const Token& term,
                               const std::vector<k_value>& args) {
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

  static k_value executeRandom(const Token& term,
                             const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Random);
    }

    return MathImpl.__random__(term, args.at(0), args.at(1));
  }

  static k_value executeDivisors(const Token& term,
                               const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, MathBuiltins.Divisors);
    }

    if (!std::holds_alternative<k_int>(args.at(0))) {
      throw ConversionError(term,
                            "Expected an `Integer` argument for builtin `" +
                                MathBuiltins.Divisors + "`.");
    }

    auto list = std::make_shared<List>();
    for (const auto& divisor :
         MathImpl.__divisors__(std::get<k_int>(args.at(0)))) {
      list->elements.push_back(divisor);
    }

    return list;
  }
};

#endif