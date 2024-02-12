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
  static Value execute(const Token& tokenTerm, const std::string& builtin,
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
    } else if (builtin == TimeBuiltins.TicksToMilliseconds) {
      return executeTicksToMilliseconds(tokenTerm, args);
    }

    throw UnknownBuiltinError(tokenTerm, builtin);
  }

 private:
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

  static double executeTicksToMilliseconds(const Token& tokenTerm,
                                           const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           TimeBuiltins.TicksToMilliseconds);
    }

    double ticks = get_integer_or_double(tokenTerm, args.at(0));

    return Time::ticksToMilliseconds(ticks);
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
};

#endif