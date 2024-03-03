#ifndef KIWI_SYSTEM_TIME_H
#define KIWI_SYSTEM_TIME_H

#include <chrono>
#include <ctime>
#include <thread>

class Time {
 public:
  static double getTicks() {
    auto now = std::chrono::high_resolution_clock::now();
    auto now_ns = std::chrono::time_point_cast<std::chrono::nanoseconds>(now);
    return static_cast<double>(now_ns.time_since_epoch().count());
  }

  static long long delay(int ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
    return static_cast<long long>(ms);
  }

  static double epochMilliseconds() {
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch());
    return static_cast<double>(now_ms.count());
  }

  static double ticksToMilliseconds(double ticks) { return ticks / 1000000.0; }

  static long long currentHour() {
    return static_cast<long long>(getLocalTime()->tm_hour);
  }

  static long long currentMinute() {
    return static_cast<long long>(getLocalTime()->tm_min);
  }

  static long long currentSecond() {
    return static_cast<long long>(getLocalTime()->tm_sec);
  }

  static long long currentMonthDay() {
    return static_cast<long long>(getLocalTime()->tm_mday);
  }

  static long long currentWeekDay() {
    return static_cast<long long>(1 + getLocalTime()->tm_wday);
  }

  static long long currentYearDay() {
    return static_cast<long long>(getLocalTime()->tm_yday);
  }

  static long long currentMonth() {
    return static_cast<long long>(1 + getLocalTime()->tm_mon);
  }

  static long long currentYear() {
    return static_cast<long long>(1900 + getLocalTime()->tm_year);
  }

  static std::string getAMPM() { return currentHour() < 12 ? "AM" : "PM"; }

  static bool isDST() { return getLocalTime()->tm_isdst > 0; }

 private:
  static std::tm* getLocalTime() {
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    return std::localtime(&now_c);
  }
};

#endif