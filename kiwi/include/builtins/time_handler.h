#ifndef KIWI_BUILTINS_TIMEHANDLER_H
#define KIWI_BUILTINS_TIMEHANDLER_H

#include <exception>
#include <vector>
#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/value.h"
#include "util/time.h"

class TimeBuiltinHandler {
 public:
  static k_value execute(const Token& token, const KName& builtin,
                         const std::vector<k_value>& args) {
    switch (builtin) {
      case KName::Builtin_Time_Delay:
        return executeDelay(token, args);

      case KName::Builtin_Time_EpochMilliseconds:
        return executeEpochMilliseconds(token, args);

      case KName::Builtin_Time_Hour:
        return executeCurrentHour(token, args);

      case KName::Builtin_Time_IsDST:
        return executeIsDST(token, args);

      case KName::Builtin_Time_Minute:
        return executeCurrentMinute(token, args);

      case KName::Builtin_Time_Month:
        return executeCurrentMonth(token, args);

      case KName::Builtin_Time_MonthDay:
        return executeCurrentMonthDay(token, args);

      case KName::Builtin_Time_Second:
        return executeCurrentSecond(token, args);

      case KName::Builtin_Time_Ticks:
        return executeTicks(token, args);

      case KName::Builtin_Time_WeekDay:
        return executeCurrentWeekDay(token, args);

      case KName::Builtin_Time_Year:
        return executeCurrentYear(token, args);

      case KName::Builtin_Time_YearDay:
        return executeCurrentYearDay(token, args);

      case KName::Builtin_Time_AMPM:
        return executeAMPM(token, args);

      case KName::Builtin_Time_TicksToMilliseconds:
        return executeTicksToMilliseconds(token, args);

      case KName::Builtin_Time_Timestamp:
        return executeTimestamp(token, args);

      case KName::Builtin_Time_FormatDateTime:
        return executeFormatDateTime(token, args);

      default:
        break;
    }

    throw UnknownBuiltinError(token, "");
  }

 private:
  static k_value executeDelay(const Token& token,
                              const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.Delay);
    }

    int ms = static_cast<int>(get_double(token, args.at(0)));
    return Time::delay(ms);
  }

  static k_value executeEpochMilliseconds(const Token& token,
                                          const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token,
                                           TimeBuiltins.EpochMilliseconds);
    }

    return Time::epochMilliseconds();
  }

  static k_value executeCurrentHour(const Token& token,
                                    const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.Hour);
    }

    return Time::currentHour();
  }

  static k_value executeCurrentMinute(const Token& token,
                                      const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.Minute);
    }

    return Time::currentMinute();
  }

  static k_value executeAMPM(const Token& token,
                             const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.AMPM);
    }

    return Time::getAMPM();
  }

  static k_value executeTimestamp(const Token& token,
                                  const std::vector<k_value>& args) {
    if (args.size() != 0 && args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.Timestamp);
    }

    if (args.size() == 1) {
      return Time::getTimestamp(get_string(token, args.at(0)));
    }

    return Time::getTimestamp();
  }

  static k_value executeFormatDateTime(const Token& token,
                                       const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.FormatDateTime);
    }

    auto date = args.at(0);
    auto format = get_string(token, args.at(1));
    k_int year, month, day, hour, minute, second;

    if (!std::holds_alternative<k_object>(date)) {
      throw InvalidOperationError(token, "Expected a `DateTime` object.");
    }

    auto dateValue = std::get<k_object>(date);
    if (dateValue->structName != "DateTime") {
      throw InvalidOperationError(token, "Expected a `DateTime` object.");
    }

    try {
      year = get_integer(token, dateValue->instanceVariables["year"]);
      month = get_integer(token, dateValue->instanceVariables["month"]);
      day = get_integer(token, dateValue->instanceVariables["day"]);
      hour = get_integer(token, dateValue->instanceVariables["hour"]);
      minute = get_integer(token, dateValue->instanceVariables["minute"]);
      second = get_integer(token, dateValue->instanceVariables["second"]);
    } catch (const std::exception& e) {
      throw InvalidOperationError(token, "Expected a DateTime object.");
    }

    return Time::formatDateTime(year, month, day, hour, minute, second, format);
  }

  static k_value executeTicksToMilliseconds(const Token& token,
                                            const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           TimeBuiltins.TicksToMilliseconds);
    }

    double ticks = get_double(token, args.at(0));

    return Time::ticksToMilliseconds(ticks);
  }

  static k_value executeCurrentMonth(const Token& token,
                                     const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.Month);
    }

    return Time::currentMonth();
  }

  static k_value executeCurrentMonthDay(const Token& token,
                                        const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.MonthDay);
    }

    return Time::currentMonthDay();
  }

  static k_value executeCurrentSecond(const Token& token,
                                      const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.Second);
    }

    return Time::currentSecond();
  }

  static k_value executeCurrentWeekDay(const Token& token,
                                       const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.WeekDay);
    }

    return Time::currentWeekDay();
  }

  static k_value executeCurrentYear(const Token& token,
                                    const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.Year);
    }

    return Time::currentYear();
  }

  static k_value executeCurrentYearDay(const Token& token,
                                       const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.YearDay);
    }

    return Time::currentYearDay();
  }

  static k_value executeTicks(const Token& token,
                              const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.Ticks);
    }

    return Time::getTicks();
  }

  static k_value executeIsDST(const Token& token,
                              const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.IsDST);
    }

    return Time::isDST();
  }
};

#endif