#ifndef KIWI_SYSTEM_TIME_H
#define KIWI_SYSTEM_TIME_H

#include <chrono>
#include <ctime>
#include <thread>
#include "typing/value.h"

class Time {
 public:
  static double getTicks() {
    auto now = std::chrono::high_resolution_clock::now();
    auto now_ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
    return static_cast<double>(now_ns.time_since_epoch().count());
  }

  static k_int delay(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    return static_cast<k_int>(ms);
  }

  static double epochMilliseconds() {
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    return static_cast<double>(now_ms.count());
  }

  static double ticksToMilliseconds(double ticks) { return ticks / 1000000.0; }

  static k_string formatDateTime(
      k_int year, k_int month, k_int day, k_int hour, k_int minute,
      k_int second, const std::string& timestampFormat = "%Y-%m-%d %H:%M:%S") {
    std::tm timeInfo = {};
    timeInfo.tm_year = year - 1900;
    timeInfo.tm_mon = month - 1;
    timeInfo.tm_mday = day;
    timeInfo.tm_hour = hour;
    timeInfo.tm_min = minute;
    timeInfo.tm_sec = second;

    std::vector<char> timestamp(64);

    auto result = std::strftime(timestamp.data(), timestamp.size(),
                                timestampFormat.c_str(), &timeInfo);

    while (result == 0) {
      timestamp.resize(timestamp.size() * 2);
      result = std::strftime(timestamp.data(), timestamp.size(),
                             timestampFormat.c_str(), &timeInfo);
    }

    return k_string(timestamp.data());
  }

  static k_string getTimestamp(
      const std::string& timestampFormat = "%Y-%m-%d %H:%M:%S") {
    std::time_t now = std::time(nullptr);
    std::tm localTime;

#ifdef _WIN64
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif

    std::vector<char> timestamp(64);

    auto result = std::strftime(timestamp.data(), timestamp.size(),
                                timestampFormat.c_str(), &localTime);

    while (result == 0) {
      timestamp.resize(timestamp.size() * 2);
      result = std::strftime(timestamp.data(), timestamp.size(),
                             timestampFormat.c_str(), &localTime);
    }

    return k_string(timestamp.data());
  }

  static k_int currentHour() {
    return static_cast<k_int>(getLocalTime()->tm_hour);
  }

  static k_int currentMinute() {
    return static_cast<k_int>(getLocalTime()->tm_min);
  }

  static k_int currentSecond() {
    return static_cast<k_int>(getLocalTime()->tm_sec);
  }

  static k_int currentMonthDay() {
    return static_cast<k_int>(getLocalTime()->tm_mday);
  }

  static k_int currentWeekDay() {
    return static_cast<k_int>(1 + getLocalTime()->tm_wday);
  }

  static k_int currentYearDay() {
    return static_cast<k_int>(getLocalTime()->tm_yday);
  }

  static k_int currentMonth() {
    return static_cast<k_int>(1 + getLocalTime()->tm_mon);
  }

  static k_int currentYear() {
    return static_cast<k_int>(1900 + getLocalTime()->tm_year);
  }

  static k_string getAMPM() { return currentHour() < 12 ? "AM" : "PM"; }

  static bool isDST() { return getLocalTime()->tm_isdst > 0; }

 private:
  static std::tm* getLocalTime() {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    return std::localtime(&now_c);
  }
};

#endif