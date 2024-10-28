#ifndef KIWI_LOGGING_LOGGER_H
#define KIWI_LOGGING_LOGGER_H

#include <iomanip>
#include <iostream>
#include <mutex>
#include <string>
#include "parsing/tokens.h"
#include "tracing/error.h"
#include "util/file.h"
#include "util/time.h"

enum class LogLevel { DEBUG, INFO, WARNING, ERROR_, SILENT };
enum class LogMode { CONSOLE, FILE };

class Logger {
 public:
  Logger(const Logger&) = delete;
  Logger& operator=(const Logger&) = delete;

  static Logger& getInstance() {
    static Logger instance;
    return instance;
  }

  static LogLevel loglevel_from_string(const std::string& logLevel) {
    if (logLevel == "DEBUG")
      return LogLevel::DEBUG;
    else if (logLevel == "INFO")
      return LogLevel::INFO;
    else if (logLevel == "WARNING")
      return LogLevel::WARNING;
    else if (logLevel == "ERROR")
      return LogLevel::ERROR_;
    else
      return LogLevel::SILENT;
  }

  static LogMode logmode_from_string(const std::string& mode) {
    if (mode == "FILE")
      return LogMode::FILE;
    else
      return LogMode::CONSOLE;
  }

  void error(const Token& token, const std::string& message,
             const std::string& source = "") const {
    log(token, LogLevel::ERROR_, message, source);
  }

  void info(const Token& token, const std::string& message,
            const std::string& source = "") const {
    log(token, LogLevel::INFO, message, source);
  }

  void warn(const Token& token, const std::string& message,
            const std::string& source = "") const {
    log(token, LogLevel::WARNING, message, source);
  }

  void debug(const Token& token, const std::string& message,
             const std::string& source = "") const {
    log(token, LogLevel::DEBUG, message, source);
  }

  void setMinimumLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(logMutex);
    minLogLevel = level;
  }

  void setLogMode(LogMode mode) {
    std::lock_guard<std::mutex> lock(logMutex);
    logMode = mode;
  }

  void setTimestampFormat(const std::string& format) {
    std::lock_guard<std::mutex> lock(logMutex);
    timestampFormat = format;
  }

  void setEntryFormat(const std::string& format) {
    std::lock_guard<std::mutex> lock(logMutex);
    entryFormat = format;
  }

  void setLogFilePath(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(logMutex);
    logFilePath = filePath;
    setLogMode(LogMode::FILE);
  }

 private:
  LogLevel minLogLevel;
  LogMode logMode;
  std::string logFilePath;
  std::string timestampFormat = "%Y-%m-%d %H:%M:%S";
  std::string entryFormat = "[%timestamp][%level][%source] %message";
  mutable std::mutex logMutex;

  Logger(LogLevel minLogLevel = LogLevel::INFO,
         LogMode logMode = LogMode::CONSOLE)
      : minLogLevel(minLogLevel), logMode(logMode) {}

  // TODO: it would be great to have a descriptor to describe log
  // output/formatting
  void log(const Token& token, const LogLevel& level,
           const std::string& message, const std::string& source) const {
    std::lock_guard<std::mutex> lock(logMutex);

    if (level < minLogLevel) {
      return;
    }

    auto logEntry = getLogEntry(level, message, source);

    switch (logMode) {
      case LogMode::CONSOLE:
        std::cout << logEntry << std::endl;
        break;
      case LogMode::FILE:
        if (!logFilePath.empty()) {
          File::writeToFile(token, logFilePath, logEntry, true, false);
        }
        break;
    }
  }

  std::string getLogEntry(const LogLevel& level, const std::string& message,
                          const std::string& source) const {
    auto timestamp = Time::getTimestamp(timestampFormat);
    auto format = entryFormat;

    format = String::replace(format, "%timestamp", timestamp);
    format = String::replace(format, "%level", logLevelToString(level));
    format = String::replace(format, "%source", source);
    format = String::replace(format, "%message", message);

    return format;
  }

  std::string logLevelToString(const LogLevel& level) const {
    switch (level) {
      case LogLevel::DEBUG:
        return "DEBUG";
      case LogLevel::INFO:
        return "INFO";
      case LogLevel::WARNING:
        return "WARNING";
      case LogLevel::ERROR_:
        return "ERROR";
      default:
        return "UNKNOWN";
    }
  }
};

#endif
