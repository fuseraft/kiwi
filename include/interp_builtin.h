#ifndef KIWI_INTERPBUILTIN_H
#define KIWI_INTERPBUILTIN_H

#include <charconv>
#include <sstream>
#include <string>
#include <vector>
#include "errors/error.h"
#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/strings.h"
#include "parsing/tokens.h"
#include "system/fileio.h"
#include "system/time.h"
#include "typing/serializer.h"
#include "typing/valuetype.h"

class BuiltinInterpreter {
 public:
  static Value execute(const Token& tokenTerm, const std::string& builtin,
                       const std::vector<Value>& args) {
    if (FileIOBuiltIns.is_builtin(builtin)) {
      return executeFileIOBuiltin(tokenTerm, builtin, args);
    } else if (TimeBuiltins.is_builtin(builtin)) {
      return executeTimeBuiltin(tokenTerm, builtin, args);
    } else if (MathBuiltins.is_builtin(builtin)) {
      return executeMathBuiltin(tokenTerm, builtin, args);
    }

    throw UnknownBuiltinError(tokenTerm, builtin);
  }

  static Value execute(const Token& tokenTerm, const std::string& builtin,
                       const Value& value, const std::vector<Value>& args) {
    if (KiwiBuiltins.is_builtin(builtin)) {
      return executeKiwiBuiltin(tokenTerm, builtin, value, args);
    }

    throw UnknownBuiltinError(tokenTerm, builtin);
  }

 private:
  static Value executeKiwiBuiltin(const Token& tokenTerm,
                                  const std::string& builtin,
                                  const Value& value,
                                  const std::vector<Value>& args) {
    if (builtin == KiwiBuiltins.Chars) {
      return executeChars(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.IsA) {
      return executeIsA(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.Join) {
      return executeJoin(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.Size) {
      return executeSize(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.ToD) {
      return executeToDouble(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.ToI) {
      return executeToInteger(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.ToS) {
      return executeToString(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.BeginsWith) {
      return executeBeginsWith(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.Contains) {
      return executeContains(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.EndsWith) {
      return executeEndsWith(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.Replace) {
      return executeReplace(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.IndexOf) {
      return executeIndexOf(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.Upcase) {
      return executeUpcase(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.Downcase) {
      return executeDowncase(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.Keys) {
      return executeKeys(tokenTerm, value, args);
    }

    throw UnknownBuiltinError(tokenTerm, builtin);
  }

  static Value executeMathBuiltin(const Token& tokenTerm,
                                  const std::string& builtin,
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

  static Value executeTimeBuiltin(const Token& tokenTerm,
                                  const std::string& builtin,
                                  const std::vector<Value>& args) {
    if (builtin == TimeBuiltins.Delay) {
      return executeDelay(tokenTerm, args);
    } else if (builtin == TimeBuiltins.EpochMilliseconds) {
      return executeEpochMilliseconds(tokenTerm, args);
    } else if (builtin == TimeBuiltins.Hour) {
      return executeCurrentHour(tokenTerm, args);
    } else if (builtin == TimeBuiltins.IsDST) {
      return executeIsDST(tokenTerm, args);
    } else if (builtin == TimeBuiltins.Minute) {
      return executeCurrentMinute(tokenTerm, args);
    } else if (builtin == TimeBuiltins.Month) {
      return executeCurrentMonth(tokenTerm, args);
    } else if (builtin == TimeBuiltins.MonthDay) {
      return executeCurrentMonthDay(tokenTerm, args);
    } else if (builtin == TimeBuiltins.Second) {
      return executeCurrentSecond(tokenTerm, args);
    } else if (builtin == TimeBuiltins.Ticks) {
      return executeTicks(tokenTerm, args);
    } else if (builtin == TimeBuiltins.WeekDay) {
      return executeCurrentWeekDay(tokenTerm, args);
    } else if (builtin == TimeBuiltins.Year) {
      return executeCurrentYear(tokenTerm, args);
    } else if (builtin == TimeBuiltins.YearDay) {
      return executeCurrentYearDay(tokenTerm, args);
    } else if (builtin == TimeBuiltins.AMPM) {
      return executeAMPM(tokenTerm, args);
    }

    throw UnknownBuiltinError(tokenTerm, builtin);
  }

  static Value executeFileIOBuiltin(const Token& tokenTerm,
                                    const std::string& builtin,
                                    const std::vector<Value>& args) {
    if (builtin == FileIOBuiltIns.CreateFile) {
      return executeCreateFile(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.FileExists) {
      return executeFileExists(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.DeleteFile) {
      return executeDeleteFile(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.GetFileExtension) {
      return executeGetFileExtension(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.FileName) {
      return executeGetFileName(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.GetFilePath) {
      return executeGetFilePath(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.GetFileAbsolutePath) {
      return executeGetFileAbsolutePath(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.AppendText) {
      return executeAppendText(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.WriteText) {
      return executeWriteText(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.WriteLine) {
      return executeWriteLine(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.ReadFile) {
      return executeReadFile(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.FileSize) {
      return executeGetFileSize(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.CopyFile) {
      return executeCopyFile(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.MoveFile) {
      return executeMoveFile(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.ReadLines) {
      return executeReadLines(tokenTerm, args);
    } else {
      throw UnknownBuiltinError(tokenTerm, builtin);
    }
  }

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

  static int executeDelay(const Token& tokenTerm,
                          const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, TimeBuiltins.Delay);
    }

    int ms = static_cast<int>(get_integer_or_double(tokenTerm, args.at(0)));
    return Time::delay(ms);
  }

  static double executeEpochMilliseconds(const Token& tokenTerm,
                                         const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           TimeBuiltins.EpochMilliseconds);
    }

    return Time::epochMilliseconds();
  }

  static int executeCurrentHour(const Token& tokenTerm,
                                const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, TimeBuiltins.Hour);
    }

    return Time::currentHour();
  }

  static int executeCurrentMinute(const Token& tokenTerm,
                                  const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, TimeBuiltins.Minute);
    }

    return Time::currentMinute();
  }

  static std::string executeAMPM(const Token& tokenTerm,
                                 const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, TimeBuiltins.AMPM);
    }

    return Time::getAMPM();
  }

  static int executeCurrentMonth(const Token& tokenTerm,
                                 const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, TimeBuiltins.Month);
    }

    return Time::currentMonth();
  }

  static int executeCurrentMonthDay(const Token& tokenTerm,
                                    const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, TimeBuiltins.MonthDay);
    }

    return Time::currentMonthDay();
  }

  static int executeCurrentSecond(const Token& tokenTerm,
                                  const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, TimeBuiltins.Second);
    }

    return Time::currentSecond();
  }

  static int executeCurrentWeekDay(const Token& tokenTerm,
                                   const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, TimeBuiltins.WeekDay);
    }

    return Time::currentWeekDay();
  }

  static int executeCurrentYear(const Token& tokenTerm,
                                const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, TimeBuiltins.Year);
    }

    return Time::currentYear();
  }

  static int executeCurrentYearDay(const Token& tokenTerm,
                                   const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, TimeBuiltins.YearDay);
    }

    return Time::currentYearDay();
  }

  static double executeTicks(const Token& tokenTerm,
                             const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, TimeBuiltins.Ticks);
    }

    return Time::getTicks();
  }

  static bool executeIsDST(const Token& tokenTerm,
                           const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, TimeBuiltins.IsDST);
    }

    return Time::isDST();
  }

  static std::shared_ptr<List> executeChars(const Token& tokenTerm,
                                            const Value& value,
                                            const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.Chars);
    }

    auto newList = std::make_shared<List>();
    std::string stringValue = get_string(tokenTerm, value);
    for (char c : stringValue) {
      newList->elements.push_back(std::string(1, c));
    }
    return newList;
  }

  static std::string executeJoin(const Token& tokenTerm, const Value& value,
                                 const std::vector<Value>& args) {
    int argSize = args.size();

    if (argSize != 0 && argSize != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.Join);
    }

    if (!std::holds_alternative<std::shared_ptr<List>>(value)) {
      throw ConversionError(tokenTerm, "Cannot join a non-list type.");
    }

    auto list = std::get<std::shared_ptr<List>>(value);
    std::ostringstream sv;
    std::string joiner;

    if (argSize == 1) {
      joiner = get_string(tokenTerm, args.at(0));
    }

    for (auto it = list->elements.begin(); it != list->elements.end(); ++it) {
      if (it != list->elements.begin()) {
        sv << joiner;
      }
      sv << Serializer::get_value_string(*it);
    }

    return sv.str();
  }

  static int executeSize(const Token& tokenTerm, const Value& value,
                         const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.Size);
    }

    if (std::holds_alternative<std::string>(value)) {
      int size = std::get<std::string>(value).length();
      return size;
    } else if (std::holds_alternative<std::shared_ptr<List>>(value)) {
      auto list = std::get<std::shared_ptr<List>>(value);
      int size = list->elements.size();
      return size;
    } else if (std::holds_alternative<std::shared_ptr<Hash>>(value)) {
      auto hash = std::get<std::shared_ptr<Hash>>(value);
      int size = hash->kvp.size();
      return size;
    } else {
      throw ConversionError(tokenTerm);
    }
  }

  static double executeToDouble(const Token& tokenTerm, const Value& value,
                                const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.ToD);
    }

    if (std::holds_alternative<std::string>(value)) {
      std::string stringValue = std::get<std::string>(value);
      double doubleValue = 0;
      auto [ptr, ec] =
          std::from_chars(stringValue.data(),
                          stringValue.data() + stringValue.size(), doubleValue);

      if (ec == std::errc()) {
        return doubleValue;
      } else {
        throw ConversionError(
            tokenTerm, "Cannot convert non-numeric value to a double: `" +
                           stringValue + "`");
      }
    } else if (std::holds_alternative<int>(value)) {
      return static_cast<double>(std::get<int>(value));
    } else {
      throw ConversionError(tokenTerm,
                            "Cannot convert non-numeric value to a double.");
    }
  }

  static int executeToInteger(const Token& tokenTerm, const Value& value,
                              const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.ToI);
    }

    if (std::holds_alternative<std::string>(value)) {
      std::string stringValue = std::get<std::string>(value);
      int intValue = 0;
      auto [ptr, ec] =
          std::from_chars(stringValue.data(),
                          stringValue.data() + stringValue.size(), intValue);

      if (ec == std::errc()) {
        return intValue;
      } else {
        throw ConversionError(
            tokenTerm, "Cannot convert non-numeric value to an integer: `" +
                           stringValue + "`");
      }
    } else if (std::holds_alternative<double>(value)) {
      return static_cast<int>(std::get<double>(value));
    } else {
      throw ConversionError(tokenTerm,
                            "Cannot convert non-numeric value to an integer.");
    }
  }

  static std::string executeToString(const Token& tokenTerm, const Value& value,
                                     const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.ToS);
    }

    return Serializer::get_value_string(value);
  }

  static std::shared_ptr<List> executeKeys(const Token& tokenTerm,
                                           const Value& value,
                                           const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.Keys);
    }

    if (!std::holds_alternative<std::shared_ptr<Hash>>(value)) {
      throw InvalidOperationError(
          tokenTerm, "Attempted to retrieve keys from non-Hash type.");
    }

    auto hash = std::get<std::shared_ptr<Hash>>(value);

    return Serializer::get_hash_keys_list(hash);
  }

  static bool executeBeginsWith(const Token& tokenTerm, const Value& value,
                                const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.BeginsWith);
    }

    auto str = get_string(tokenTerm, value);
    auto search = get_string(tokenTerm, args.at(0));
    return begins_with(str, search);
  }

  static bool executeContains(const Token& tokenTerm, const Value& value,
                              const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.Contains);
    }

    auto str = get_string(tokenTerm, value);
    auto search = get_string(tokenTerm, args.at(0));
    return contains(str, search);
  }

  static bool executeEndsWith(const Token& tokenTerm, const Value& value,
                              const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.Contains);
    }

    auto str = get_string(tokenTerm, value);
    auto search = get_string(tokenTerm, args.at(0));
    return ends_with(str, search);
  }

  static bool executeIsA(const Token& tokenTerm, const Value& value,
                         const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.Contains);
    }

    auto typeName = get_string(tokenTerm, args.at(0));
    if (!TypeNames.is_typename(typeName)) {
      throw InvalidTypeNameError(tokenTerm, typeName);
    }

    return (typeName == TypeNames.Boolean &&
            std::holds_alternative<bool>(value)) ||
           (typeName == TypeNames.Double &&
            std::holds_alternative<double>(value)) ||
           (typeName == TypeNames.Hash &&
            std::holds_alternative<std::shared_ptr<Hash>>(value)) ||
           (typeName == TypeNames.Integer &&
            std::holds_alternative<int>(value)) ||
           (typeName == TypeNames.List &&
            std::holds_alternative<std::shared_ptr<List>>(value)) ||
           (typeName == TypeNames.String &&
            std::holds_alternative<std::string>(value));
  }

  static std::string executeReplace(const Token& tokenTerm, const Value& value,
                                    const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.Replace);
    }

    auto str = get_string(tokenTerm, value);
    auto search = get_string(tokenTerm, args.at(0));
    auto replacement = get_string(tokenTerm, args.at(1));
    return replace(str, search, replacement);
  }

  static int executeIndexOf(const Token& tokenTerm, const Value& value,
                            const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.IndexOf);
    }

    auto str = get_string(tokenTerm, value);
    auto search = get_string(tokenTerm, args.at(0));
    return index_of(str, search);
  }

  static std::string executeUpcase(const Token& tokenTerm, const Value& value,
                                   const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.Upcase);
    }

    auto str = get_string(tokenTerm, value);
    return upcase(str);
  }

  static std::string executeDowncase(const Token& tokenTerm, const Value& value,
                                     const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.Downcase);
    }

    auto str = get_string(tokenTerm, value);
    return downcase(str);
  }

  static bool executeAppendText(const Token& tokenTerm,
                                const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           FileIOBuiltIns.AppendText);
    }

    std::string fileName = get_string(tokenTerm, args.at(0));
    Value value = args.at(1);
    return FileIO::writeToFile(fileName, value, true, false);
  }

  static bool executeCopyFile(const Token& tokenTerm,
                              const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, FileIOBuiltIns.CopyFile);
    }

    std::string sourcePath = get_string(tokenTerm, args.at(0));
    std::string destinationPath = get_string(tokenTerm, args.at(1));
    return FileIO::copyFile(sourcePath, destinationPath);
  }

  static bool executeCreateFile(const Token& tokenTerm,
                                const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           FileIOBuiltIns.CreateFile);
    }

    std::string fileName = get_string(tokenTerm, args.at(0));
    return FileIO::createFile(fileName);
  }

  static bool executeDeleteFile(const Token& tokenTerm,
                                const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           FileIOBuiltIns.DeleteFile);
    }

    std::string fileName = get_string(tokenTerm, args.at(0));
    return FileIO::deleteFile(fileName);
  }

  static double executeGetFileSize(const Token& tokenTerm,
                                   const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, FileIOBuiltIns.FileSize);
    }

    std::string fileName = get_string(tokenTerm, args.at(0));
    return FileIO::getFileSize(fileName);
  }

  static bool executeFileExists(const Token& tokenTerm,
                                const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           FileIOBuiltIns.FileExists);
    }

    std::string fileName = get_string(tokenTerm, args.at(0));
    return FileIO::fileExists(fileName);
  }

  static std::string executeGetFileAbsolutePath(
      const Token& tokenTerm, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           FileIOBuiltIns.GetFileAbsolutePath);
    }

    std::string fileName = get_string(tokenTerm, args.at(0));
    return FileIO::getAbsolutePath(fileName);
  }

  static std::string executeGetFileExtension(const Token& tokenTerm,
                                             const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           FileIOBuiltIns.GetFileExtension);
    }

    std::string fileName = get_string(tokenTerm, args.at(0));
    return FileIO::getFileExtension(fileName);
  }

  static std::string executeGetFileName(const Token& tokenTerm,
                                        const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, FileIOBuiltIns.FileName);
    }

    std::string fileName = get_string(tokenTerm, args.at(0));
    return FileIO::getFileName(fileName);
  }

  static std::string executeGetFilePath(const Token& tokenTerm,
                                        const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           FileIOBuiltIns.GetFilePath);
    }

    std::string fileName = get_string(tokenTerm, args.at(0));
    return FileIO::getParentPath(fileName);
  }

  static bool executeMoveFile(const Token& tokenTerm,
                              const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, FileIOBuiltIns.MoveFile);
    }

    std::string sourcePath = get_string(tokenTerm, args.at(0));
    std::string destinationPath = get_string(tokenTerm, args.at(1));
    return FileIO::moveFile(sourcePath, destinationPath);
  }

  static std::string executeReadFile(const Token& tokenTerm,
                                     const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, FileIOBuiltIns.ReadFile);
    }

    std::string fileName = get_string(tokenTerm, args.at(0));
    return FileIO::readFile(fileName);
  }

  static std::shared_ptr<List> executeReadLines(
      const Token& tokenTerm, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, FileIOBuiltIns.ReadLines);
    }
    std::string fileName = get_string(tokenTerm, args.at(0));
    return FileIO::readLines(fileName);
  }

  static bool executeWriteLine(const Token& tokenTerm,
                               const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, FileIOBuiltIns.WriteLine);
    }

    std::string fileName = get_string(tokenTerm, args.at(0));
    Value value = args.at(1);
    return FileIO::writeToFile(fileName, value, true, true);
  }

  static bool executeWriteText(const Token& tokenTerm,
                               const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, FileIOBuiltIns.WriteText);
    }

    std::string fileName = get_string(tokenTerm, args.at(0));
    Value value = args.at(1);
    return FileIO::writeToFile(fileName, value, false, false);
  }
};

#endif
