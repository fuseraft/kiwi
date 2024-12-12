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
  static KValue execute(const Token& token, const KName& builtin,
                        const std::vector<KValue>& args) {
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
  static KValue executeDelay(const Token& token,
                             const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.Delay);
    }

    int ms = static_cast<int>(get_float(token, args.at(0)));
    return KValue::createInteger(Time::delay(ms));
  }

  static KValue executeEpochMilliseconds(const Token& token,
                                         const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token,
                                           TimeBuiltins.EpochMilliseconds);
    }

    return KValue::createFloat(Time::epochMilliseconds());
  }

  static KValue executeCurrentHour(const Token& token,
                                   const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.Hour);
    }

    return KValue::createInteger(Time::currentHour());
  }

  static KValue executeCurrentMinute(const Token& token,
                                     const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.Minute);
    }

    return KValue::createInteger(Time::currentMinute());
  }

  static KValue executeAMPM(const Token& token,
                            const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.AMPM);
    }

    return KValue::createString(Time::getAMPM());
  }

  static KValue executeTimestamp(const Token& token,
                                 const std::vector<KValue>& args) {
    if (args.size() != 0 && args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.Timestamp);
    }

    if (args.size() == 1) {
      return KValue::createString(
          Time::getTimestamp(get_string(token, args.at(0))));
    }

    return KValue::createString(Time::getTimestamp());
  }

  static KValue executeFormatDateTime(const Token& token,
                                      const std::vector<KValue>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.FormatDateTime);
    }

    auto date = args.at(0);
    auto format = get_string(token, args.at(1));
    k_int year, month, day, hour, minute, second;

    if (!date.isObject()) {
      throw InvalidOperationError(token, "Expected a `DateTime` object.");
    }

    auto dateValue = date.getObject();
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

    return KValue::createString(
        Time::formatDateTime(year, month, day, hour, minute, second, format));
  }

  static KValue executeTicksToMilliseconds(const Token& token,
                                           const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           TimeBuiltins.TicksToMilliseconds);
    }

    double ticks = get_float(token, args.at(0));

    return KValue::createFloat(Time::ticksToMilliseconds(ticks));
  }

  static KValue executeCurrentMonth(const Token& token,
                                    const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.Month);
    }

    return KValue::createInteger(Time::currentMonth());
  }

  static KValue executeCurrentMonthDay(const Token& token,
                                       const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.MonthDay);
    }

    return KValue::createInteger(Time::currentMonthDay());
  }

  static KValue executeCurrentSecond(const Token& token,
                                     const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.Second);
    }

    return KValue::createInteger(Time::currentSecond());
  }

  static KValue executeCurrentWeekDay(const Token& token,
                                      const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.WeekDay);
    }

    return KValue::createInteger(Time::currentWeekDay());
  }

  static KValue executeCurrentYear(const Token& token,
                                   const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.Year);
    }

    return KValue::createInteger(Time::currentYear());
  }

  static KValue executeCurrentYearDay(const Token& token,
                                      const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.YearDay);
    }

    return KValue::createInteger(Time::currentYearDay());
  }

  static KValue executeTicks(const Token& token,
                             const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.Ticks);
    }

    return KValue::createFloat(Time::getTicks());
  }

  static KValue executeIsDST(const Token& token,
                             const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, TimeBuiltins.IsDST);
    }

    return KValue::createBoolean(Time::isDST());
  }
};

#endif