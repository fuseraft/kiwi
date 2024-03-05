#ifndef KIWI_BUILTINS_TIMEHANDLER_H
#define KIWI_BUILTINS_TIMEHANDLER_H

#include <vector>
#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "system/time.h"
#include "typing/valuetype.h"

class TimeBuiltinHandler {
 public:
  static Value execute(const Token& tokenTerm, const SubTokenType& builtin,
                       const std::vector<Value>& args) {
    if (builtin == SubTokenType::Builtin_Time_Delay) {
      return executeDelay(tokenTerm, args);
    } else if (builtin == SubTokenType::Builtin_Time_EpochMilliseconds) {
      return executeEpochMilliseconds(tokenTerm, args);
    } else if (builtin == SubTokenType::Builtin_Time_Hour) {
      return executeCurrentHour(tokenTerm, args);
    } else if (builtin == SubTokenType::Builtin_Time_IsDST) {
      return executeIsDST(tokenTerm, args);
    } else if (builtin == SubTokenType::Builtin_Time_Minute) {
      return executeCurrentMinute(tokenTerm, args);
    } else if (builtin == SubTokenType::Builtin_Time_Month) {
      return executeCurrentMonth(tokenTerm, args);
    } else if (builtin == SubTokenType::Builtin_Time_MonthDay) {
      return executeCurrentMonthDay(tokenTerm, args);
    } else if (builtin == SubTokenType::Builtin_Time_Second) {
      return executeCurrentSecond(tokenTerm, args);
    } else if (builtin == SubTokenType::Builtin_Time_Ticks) {
      return executeTicks(tokenTerm, args);
    } else if (builtin == SubTokenType::Builtin_Time_WeekDay) {
      return executeCurrentWeekDay(tokenTerm, args);
    } else if (builtin == SubTokenType::Builtin_Time_Year) {
      return executeCurrentYear(tokenTerm, args);
    } else if (builtin == SubTokenType::Builtin_Time_YearDay) {
      return executeCurrentYearDay(tokenTerm, args);
    } else if (builtin == SubTokenType::Builtin_Time_AMPM) {
      return executeAMPM(tokenTerm, args);
    } else if (builtin == SubTokenType::Builtin_Time_TicksToMilliseconds) {
      return executeTicksToMilliseconds(tokenTerm, args);
    }

    throw UnknownBuiltinError(tokenTerm, "");
  }

 private:
  static Value executeDelay(const Token& tokenTerm,
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

  static Value executeCurrentHour(const Token& tokenTerm,
                                  const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, TimeBuiltins.Hour);
    }

    return Time::currentHour();
  }

  static Value executeCurrentMinute(const Token& tokenTerm,
                                    const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, TimeBuiltins.Minute);
    }

    return Time::currentMinute();
  }

  static Value executeAMPM(const Token& tokenTerm,
                           const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, TimeBuiltins.AMPM);
    }

    return Time::getAMPM();
  }

  static Value executeTicksToMilliseconds(const Token& tokenTerm,
                                          const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           TimeBuiltins.TicksToMilliseconds);
    }

    double ticks = get_integer_or_double(tokenTerm, args.at(0));

    return Time::ticksToMilliseconds(ticks);
  }

  static Value executeCurrentMonth(const Token& tokenTerm,
                                   const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, TimeBuiltins.Month);
    }

    return Time::currentMonth();
  }

  static Value executeCurrentMonthDay(const Token& tokenTerm,
                                      const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, TimeBuiltins.MonthDay);
    }

    return Time::currentMonthDay();
  }

  static Value executeCurrentSecond(const Token& tokenTerm,
                                    const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, TimeBuiltins.Second);
    }

    return Time::currentSecond();
  }

  static Value executeCurrentWeekDay(const Token& tokenTerm,
                                     const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, TimeBuiltins.WeekDay);
    }

    return Time::currentWeekDay();
  }

  static Value executeCurrentYear(const Token& tokenTerm,
                                  const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, TimeBuiltins.Year);
    }

    return Time::currentYear();
  }

  static Value executeCurrentYearDay(const Token& tokenTerm,
                                     const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, TimeBuiltins.YearDay);
    }

    return Time::currentYearDay();
  }

  static Value executeTicks(const Token& tokenTerm,
                            const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, TimeBuiltins.Ticks);
    }

    return Time::getTicks();
  }

  static Value executeIsDST(const Token& tokenTerm,
                            const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, TimeBuiltins.IsDST);
    }

    return Time::isDST();
  }
};

#endif