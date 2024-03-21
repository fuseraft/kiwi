#ifndef KIWI_BUILTINS_TIMEHANDLER_H
#define KIWI_BUILTINS_TIMEHANDLER_H

#include <vector>
#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/value.h"
#include "util/time.h"

class TimeBuiltinHandler {
 public:
  static Value execute(const Token& term, const KName& builtin,
                       const std::vector<Value>& args) {
    switch (builtin) {
      case KName::Builtin_Time_Delay:
        return executeDelay(term, args);

      case KName::Builtin_Time_EpochMilliseconds:
        return executeEpochMilliseconds(term, args);

      case KName::Builtin_Time_Hour:
        return executeCurrentHour(term, args);

      case KName::Builtin_Time_IsDST:
        return executeIsDST(term, args);

      case KName::Builtin_Time_Minute:
        return executeCurrentMinute(term, args);

      case KName::Builtin_Time_Month:
        return executeCurrentMonth(term, args);

      case KName::Builtin_Time_MonthDay:
        return executeCurrentMonthDay(term, args);

      case KName::Builtin_Time_Second:
        return executeCurrentSecond(term, args);

      case KName::Builtin_Time_Ticks:
        return executeTicks(term, args);

      case KName::Builtin_Time_WeekDay:
        return executeCurrentWeekDay(term, args);

      case KName::Builtin_Time_Year:
        return executeCurrentYear(term, args);

      case KName::Builtin_Time_YearDay:
        return executeCurrentYearDay(term, args);

      case KName::Builtin_Time_AMPM:
        return executeAMPM(term, args);

      case KName::Builtin_Time_TicksToMilliseconds:
        return executeTicksToMilliseconds(term, args);

      default:
        break;
    }

    throw UnknownBuiltinError(term, "");
  }

 private:
  static Value executeDelay(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, TimeBuiltins.Delay);
    }

    int ms = static_cast<int>(get_integer_or_double(term, args.at(0)));
    return Time::delay(ms);
  }

  static Value executeEpochMilliseconds(const Token& term,
                                        const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term,
                                           TimeBuiltins.EpochMilliseconds);
    }

    return Time::epochMilliseconds();
  }

  static Value executeCurrentHour(const Token& term,
                                  const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, TimeBuiltins.Hour);
    }

    return Time::currentHour();
  }

  static Value executeCurrentMinute(const Token& term,
                                    const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, TimeBuiltins.Minute);
    }

    return Time::currentMinute();
  }

  static Value executeAMPM(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, TimeBuiltins.AMPM);
    }

    return Time::getAMPM();
  }

  static Value executeTicksToMilliseconds(const Token& term,
                                          const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term,
                                           TimeBuiltins.TicksToMilliseconds);
    }

    double ticks = get_integer_or_double(term, args.at(0));

    return Time::ticksToMilliseconds(ticks);
  }

  static Value executeCurrentMonth(const Token& term,
                                   const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, TimeBuiltins.Month);
    }

    return Time::currentMonth();
  }

  static Value executeCurrentMonthDay(const Token& term,
                                      const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, TimeBuiltins.MonthDay);
    }

    return Time::currentMonthDay();
  }

  static Value executeCurrentSecond(const Token& term,
                                    const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, TimeBuiltins.Second);
    }

    return Time::currentSecond();
  }

  static Value executeCurrentWeekDay(const Token& term,
                                     const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, TimeBuiltins.WeekDay);
    }

    return Time::currentWeekDay();
  }

  static Value executeCurrentYear(const Token& term,
                                  const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, TimeBuiltins.Year);
    }

    return Time::currentYear();
  }

  static Value executeCurrentYearDay(const Token& term,
                                     const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, TimeBuiltins.YearDay);
    }

    return Time::currentYearDay();
  }

  static Value executeTicks(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, TimeBuiltins.Ticks);
    }

    return Time::getTicks();
  }

  static Value executeIsDST(const Token& term, const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, TimeBuiltins.IsDST);
    }

    return Time::isDST();
  }
};

#endif