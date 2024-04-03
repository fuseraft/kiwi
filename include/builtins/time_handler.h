#ifndef ASTRAL_BUILTINS_TIMEHANDLER_H
#define ASTRAL_BUILTINS_TIMEHANDLER_H

#include <vector>
#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/value.h"
#include "util/time.h"

class TimeBuiltinHandler {
 public:
  static k_value execute(const Token& term, const KName& builtin,
                         const std::vector<k_value>& args) {
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

      case KName::Builtin_Time_Timestamp:
        return executeTimestamp(term, args);

      default:
        break;
    }

    throw UnknownBuiltinError(term, "");
  }

 private:
  static k_value executeDelay(const Token& term,
                              const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, TimeBuiltins.Delay);
    }

    int ms = static_cast<int>(get_integer_or_double(term, args.at(0)));
    return Time::delay(ms);
  }

  static k_value executeEpochMilliseconds(const Token& term,
                                          const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term,
                                           TimeBuiltins.EpochMilliseconds);
    }

    return Time::epochMilliseconds();
  }

  static k_value executeCurrentHour(const Token& term,
                                    const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, TimeBuiltins.Hour);
    }

    return Time::currentHour();
  }

  static k_value executeCurrentMinute(const Token& term,
                                      const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, TimeBuiltins.Minute);
    }

    return Time::currentMinute();
  }

  static k_value executeAMPM(const Token& term,
                             const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, TimeBuiltins.AMPM);
    }

    return Time::getAMPM();
  }

  static k_value executeTimestamp(const Token& term,
                                  const std::vector<k_value>& args) {
    if (args.size() != 0 && args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, TimeBuiltins.Timestamp);
    }

    if (args.size() == 1) {
      return Time::getTimestamp(get_string(term, args.at(0)));
    }

    return Time::getTimestamp();
  }

  static k_value executeTicksToMilliseconds(const Token& term,
                                            const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term,
                                           TimeBuiltins.TicksToMilliseconds);
    }

    double ticks = get_integer_or_double(term, args.at(0));

    return Time::ticksToMilliseconds(ticks);
  }

  static k_value executeCurrentMonth(const Token& term,
                                     const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, TimeBuiltins.Month);
    }

    return Time::currentMonth();
  }

  static k_value executeCurrentMonthDay(const Token& term,
                                        const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, TimeBuiltins.MonthDay);
    }

    return Time::currentMonthDay();
  }

  static k_value executeCurrentSecond(const Token& term,
                                      const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, TimeBuiltins.Second);
    }

    return Time::currentSecond();
  }

  static k_value executeCurrentWeekDay(const Token& term,
                                       const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, TimeBuiltins.WeekDay);
    }

    return Time::currentWeekDay();
  }

  static k_value executeCurrentYear(const Token& term,
                                    const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, TimeBuiltins.Year);
    }

    return Time::currentYear();
  }

  static k_value executeCurrentYearDay(const Token& term,
                                       const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, TimeBuiltins.YearDay);
    }

    return Time::currentYearDay();
  }

  static k_value executeTicks(const Token& term,
                              const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, TimeBuiltins.Ticks);
    }

    return Time::getTicks();
  }

  static k_value executeIsDST(const Token& term,
                              const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(term, TimeBuiltins.IsDST);
    }

    return Time::isDST();
  }
};

#endif