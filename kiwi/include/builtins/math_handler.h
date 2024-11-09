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
  static k_value execute(const Token& token, const KName& builtin,
                         const std::vector<k_value>& args) {
    switch (builtin) {
      case KName::Builtin_Math_Sin:
        return executeSin(token, args);

      case KName::Builtin_Math_Tan:
        return executeTan(token, args);

      case KName::Builtin_Math_Cos:
        return executeCos(token, args);

      case KName::Builtin_Math_Asin:
        return executeAsin(token, args);

      case KName::Builtin_Math_Acos:
        return executeAcos(token, args);

      case KName::Builtin_Math_Atan:
        return executeAtan(token, args);

      case KName::Builtin_Math_Atan2:
        return executeAtan2(token, args);

      case KName::Builtin_Math_Sinh:
        return executeSinh(token, args);

      case KName::Builtin_Math_Cosh:
        return executeCosh(token, args);

      case KName::Builtin_Math_Tanh:
        return executeTanh(token, args);

      case KName::Builtin_Math_Log:
        return executeLog(token, args);

      case KName::Builtin_Math_Log2:
        return executeLog2(token, args);

      case KName::Builtin_Math_Log10:
        return executeLog10(token, args);

      case KName::Builtin_Math_Log1P:
        return executeLog1P(token, args);

      case KName::Builtin_Math_Fmod:
        return executeFmod(token, args);

      case KName::Builtin_Math_Hypot:
        return executeHypot(token, args);

      case KName::Builtin_Math_IsFinite:
        return executeIsFinite(token, args);

      case KName::Builtin_Math_IsInf:
        return executeIsInf(token, args);

      case KName::Builtin_Math_IsNaN:
        return executeIsNaN(token, args);

      case KName::Builtin_Math_IsNormal:
        return executeIsNormal(token, args);

      case KName::Builtin_Math_Sqrt:
        return executeSqrt(token, args);

      case KName::Builtin_Math_Cbrt:
        return executeCbrt(token, args);

      case KName::Builtin_Math_Abs:
        return executeAbs(token, args);

      case KName::Builtin_Math_Floor:
        return executeFloor(token, args);

      case KName::Builtin_Math_Ceil:
        return executeCeil(token, args);

      case KName::Builtin_Math_Round:
        return executeRound(token, args);

      case KName::Builtin_Math_Trunc:
        return executeTrunc(token, args);

      case KName::Builtin_Math_Remainder:
        return executeRemainder(token, args);

      case KName::Builtin_Math_RotateLeft:
        return executeRotateLeft(token, args);

      case KName::Builtin_Math_RotateRight:
        return executeRotateRight(token, args);

      case KName::Builtin_Math_Exp:
        return executeExp(token, args);

      case KName::Builtin_Math_ExpM1:
        return executeExpM1(token, args);

      case KName::Builtin_Math_Erf:
        return executeErf(token, args);

      case KName::Builtin_Math_ErfC:
        return executeErfC(token, args);

      case KName::Builtin_Math_LGamma:
        return executeLGamma(token, args);

      case KName::Builtin_Math_TGamma:
        return executeTGamma(token, args);

      case KName::Builtin_Math_FMax:
        return executeFMax(token, args);

      case KName::Builtin_Math_FMin:
        return executeFMin(token, args);

      case KName::Builtin_Math_FDim:
        return executeFDim(token, args);

      case KName::Builtin_Math_CopySign:
        return executeCopySign(token, args);

      case KName::Builtin_Math_NextAfter:
        return executeNextAfter(token, args);

      case KName::Builtin_Math_Pow:
        return executePow(token, args);

      case KName::Builtin_Math_Epsilon:
        return executeEpsilon(token, args);

      case KName::Builtin_Math_Random:
        return executeRandom(token, args);

      case KName::Builtin_Math_ListPrimes:
        return executeListPrimes(token, args);

      case KName::Builtin_Math_NthPrime:
        return executeNthPrime(token, args);

      case KName::Builtin_Math_Divisors:
        return executeDivisors(token, args);

      default:
        break;
    }

    throw UnknownBuiltinError(token, "");
  }

 private:
  static k_value executeSin(const Token& token,
                            const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Sin);
    }

    return MathImpl.__sin__(token, args.at(0));
  }

  static k_value executeCos(const Token& token,
                            const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Cos);
    }

    return MathImpl.__cos__(token, args.at(0));
  }

  static k_value executeTan(const Token& token,
                            const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Tan);
    }

    return MathImpl.__tan__(token, args.at(0));
  }

  static k_value executeAsin(const Token& token,
                             const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Asin);
    }

    return MathImpl.__asin__(token, args.at(0));
  }

  static k_value executeAcos(const Token& token,
                             const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Acos);
    }

    return MathImpl.__acos__(token, args.at(0));
  }

  static k_value executeAtan(const Token& token,
                             const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Atan);
    }

    return MathImpl.__atan__(token, args.at(0));
  }

  static k_value executeAtan2(const Token& token,
                              const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Atan2);
    }

    return MathImpl.__atan2__(token, args.at(0), args.at(1));
  }

  static k_value executeSinh(const Token& token,
                             const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Sinh);
    }

    return MathImpl.__sinh__(token, args.at(0));
  }

  static k_value executeCosh(const Token& token,
                             const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Cosh);
    }

    return MathImpl.__cosh__(token, args.at(0));
  }

  static k_value executeTanh(const Token& token,
                             const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Tanh);
    }

    return MathImpl.__tanh__(token, args.at(0));
  }

  static k_value executeLog(const Token& token,
                            const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Log);
    }

    return MathImpl.__log__(token, args.at(0));
  }

  static k_value executeLog2(const Token& token,
                             const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Log2);
    }

    return MathImpl.__log2__(token, args.at(0));
  }

  static k_value executeLog10(const Token& token,
                              const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Log10);
    }

    return MathImpl.__log10__(token, args.at(0));
  }

  static k_value executeLog1P(const Token& token,
                              const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Log1P);
    }

    return MathImpl.__log1p__(token, args.at(0));
  }

  static k_value executeFmod(const Token& token,
                             const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Fmod);
    }

    return MathImpl.__fmod__(token, args.at(0), args.at(1));
  }

  static k_value executeHypot(const Token& token,
                              const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Hypot);
    }

    return MathImpl.__hypot__(token, args.at(0), args.at(1));
  }

  static k_value executeIsFinite(const Token& token,
                                 const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.IsFinite);
    }

    return MathImpl.__isfinite__(token, args.at(0));
  }

  static k_value executeIsInf(const Token& token,
                              const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.IsInf);
    }

    return MathImpl.__isinf__(token, args.at(0));
  }

  static k_value executeIsNaN(const Token& token,
                              const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.IsNaN);
    }

    return MathImpl.__isnan__(token, args.at(0));
  }

  static k_value executeIsNormal(const Token& token,
                                 const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.IsNormal);
    }

    return MathImpl.__isnormal__(token, args.at(0));
  }

  static k_value executeSqrt(const Token& token,
                             const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Sqrt);
    }

    return MathImpl.__sqrt__(token, args.at(0));
  }

  static k_value executeCbrt(const Token& token,
                             const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Cbrt);
    }

    return MathImpl.__cbrt__(token, args.at(0));
  }

  static k_value executeAbs(const Token& token,
                            const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Abs);
    }

    return MathImpl.__abs__(token, args.at(0));
  }

  static k_value executeFloor(const Token& token,
                              const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Floor);
    }

    return MathImpl.__floor__(token, args.at(0));
  }

  static k_value executeCeil(const Token& token,
                             const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Ceil);
    }

    return MathImpl.__ceil__(token, args.at(0));
  }

  static k_value executeTrunc(const Token& token,
                              const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Trunc);
    }

    return MathImpl.__trunc__(token, args.at(0));
  }

  static k_value executeRound(const Token& token,
                              const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Round);
    }

    return MathImpl.__round__(token, args.at(0));
  }

  static k_value executeRotateLeft(const Token& token,
                                   const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.RotateLeft);
    }

    auto number = get_integer(token, args.at(0));
    auto shift = get_integer(token, args.at(1));

    return MathImpl.__rotl__(number, shift);
  }

  static k_value executeRotateRight(const Token& token,
                                    const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.RotateRight);
    }

    auto number = get_integer(token, args.at(0));
    auto shift = get_integer(token, args.at(1));

    return MathImpl.__rotr__(number, shift);
  }

  static k_value executeRemainder(const Token& token,
                                  const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Remainder);
    }

    return MathImpl.__remainder__(token, args.at(0), args.at(1));
  }

  static k_value executeExp(const Token& token,
                            const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Remainder);
    }

    return MathImpl.__exp__(token, args.at(0));
  }

  static k_value executeExpM1(const Token& token,
                              const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.ExpM1);
    }

    return MathImpl.__expm1__(token, args.at(0));
  }

  static k_value executeErf(const Token& token,
                            const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Erf);
    }

    return MathImpl.__erf__(token, args.at(0));
  }

  static k_value executeErfC(const Token& token,
                             const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.ErfC);
    }

    return MathImpl.__erfc__(token, args.at(0));
  }

  static k_value executeLGamma(const Token& token,
                               const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.LGamma);
    }

    return MathImpl.__lgamma__(token, args.at(0));
  }

  static k_value executeTGamma(const Token& token,
                               const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.TGamma);
    }

    return MathImpl.__tgamma__(token, args.at(0));
  }

  static k_value executeFMax(const Token& token,
                             const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.FMax);
    }

    return MathImpl.__max__(token, args.at(0), args.at(1));
  }

  static k_value executeFMin(const Token& token,
                             const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.FMin);
    }

    return MathImpl.__min__(token, args.at(0), args.at(1));
  }

  static k_value executeFDim(const Token& token,
                             const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.FDim);
    }

    return MathImpl.__fdim__(token, args.at(0), args.at(1));
  }

  static k_value executeCopySign(const Token& token,
                                 const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.CopySign);
    }

    return MathImpl.__copysign__(token, args.at(0), args.at(1));
  }

  static k_value executeNextAfter(const Token& token,
                                  const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.NextAfter);
    }

    return MathImpl.__nextafter__(token, args.at(0), args.at(1));
  }

  static k_value executePow(const Token& token,
                            const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Pow);
    }

    return MathImpl.__pow__(token, args.at(0), args.at(1));
  }

  static k_value executeEpsilon(const Token& token,
                                const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Epsilon);
    }

    return MathImpl.__epsilon__();
  }

  static k_value executeListPrimes(const Token& token,
                                   const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.ListPrimes);
    }

    if (!std::holds_alternative<k_int>(args.at(0))) {
      throw ConversionError(token,
                            "Expected an integer argument for builtin `" +
                                MathBuiltins.ListPrimes + "`.");
    }

    auto primes = PrimeGenerator::listPrimes(std::get<k_int>(args.at(0)));
    auto list = std::make_shared<List>();
    auto& elements = list->elements;
    elements.reserve(primes.size());

    for (const auto& prime : primes) {
      elements.emplace_back(static_cast<k_int>(prime));
    }

    return list;
  }

  static k_value executeNthPrime(const Token& token,
                                 const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.NthPrime);
    }

    if (!std::holds_alternative<k_int>(args.at(0))) {
      throw ConversionError(token,
                            "Expected an integer argument for builtin `" +
                                MathBuiltins.NthPrime + "`.");
    }

    auto nthPrime = PrimeGenerator::nthPrime(std::get<k_int>(args.at(0)));

    return static_cast<k_int>(nthPrime);
  }

  static k_value executeRandom(const Token& token,
                               const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Random);
    }

    return MathImpl.__random__(token, args.at(0), args.at(1));
  }

  static k_value executeDivisors(const Token& token,
                                 const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, MathBuiltins.Divisors);
    }

    if (!std::holds_alternative<k_int>(args.at(0))) {
      throw ConversionError(token,
                            "Expected an integer argument for builtin `" +
                                MathBuiltins.Divisors + "`.");
    }

    const auto& divisors = MathImpl.__divisors__(std::get<k_int>(args.at(0)));
    auto list = std::make_shared<List>();
    auto& elements = list->elements;
    elements.reserve(divisors.size());

    for (const auto& divisor : divisors) {
      elements.emplace_back(divisor);
    }

    return list;
  }
};

#endif