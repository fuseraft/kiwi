#ifndef ASTRAL_LOGGING_LOGGER_H
#define ASTRAL_LOGGING_LOGGER_H

#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <string>

enum class LogLevel { DEBUG, INFO, WARNING, ERROR, SILENT };

enum class LogMode { CONSOLE, FILE };

class Logger {
 public:
  Logger(LogLevel minLogLevel = LogLevel::INFO,
         LogMode logMode = LogMode::CONSOLE)
      : minLogLevel(minLogLevel), logMode(logMode) {}

  static LogLevel loglevel_from_string(const std::string& logLevel) {
    if (logLevel == "DEBUG")
      return LogLevel::DEBUG;
    else if (logLevel == "INFO")
      return LogLevel::INFO;
    else if (logLevel == "WARNING")
      return LogLevel::WARNING;
    else if (logLevel == "ERROR")
      return LogLevel::ERROR;
    else
      return LogLevel::SILENT;
  }

  static LogMode logmode_from_string(const std::string& mode) {
    if (mode == "FILE")
      return LogMode::FILE;
    else
      return LogMode::CONSOLE;
  }

  void error(const std::string& message, const std::string& source = "") const {
    log(LogLevel::ERROR, message, source);
  }

  void info(const std::string& message, const std::string& source = "") const {
    log(LogLevel::INFO, message, source);
  }

  void warn(const std::string& message, const std::string& source = "") const {
    log(LogLevel::WARNING, message, source);
  }

  void debug(const std::string& message, const std::string& source = "") const {
    log(LogLevel::DEBUG, message, source);
  }

  void setMinimumLogLevel(LogLevel level) { minLogLevel = level; }

  void setLogMode(LogMode mode) { logMode = mode; }

  void setLogFilePath(const std::string& filePath) {
    try {
      std::ifstream inputFileStream(filePath);
      std::ofstream outputFileStream;

      bool fileExists = inputFileStream.is_open();

      if (!fileExists) {
        outputFileStream.open(filePath);
        fileExists = outputFileStream.is_open();
      }

      if (fileExists)
        logFilePath = filePath;
    } catch (const std::exception& e) {
      std::cerr << "Could not open '" << filePath
                << "' for writing. Log level will be set to SILENT."
                << std::endl;
      minLogLevel = LogLevel::SILENT;
    }
  }

 private:
  LogLevel minLogLevel;
  LogMode logMode;
  std::string logFilePath;

  // TODO: it would be great to have a descriptor to describe log
  // output/formatting
  void log(const LogLevel& level, const std::string& message,
           const std::string& source) const {
    if (level < minLogLevel)
      return;

    std::time_t now = std::time(nullptr);
    std::tm localTime;
#ifdef _WIN64
    localtime_s(&localTime, &now);
#else
    localtime_r(&now, &localTime);
#endif

    char timestamp[20];
    std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S",
                  &localTime);

    switch (logMode) {
      case LogMode::CONSOLE:
        std::cout << "[" << timestamp << "] [" << logLevelToString(level)
                  << "] ";
        if (!source.empty())
          std::cout << "[" << source << "] ";
        std::cout << message << std::endl;
        break;
      case LogMode::FILE:
        std::ofstream file(logFilePath, std::ios::app);
        if (file.is_open()) {
          file << "[" << timestamp << "] [" << logLevelToString(level) << "] ";
          if (!source.empty())
            file << "[" << source << "] ";
          file << message << std::endl;
          file.close();
        }
        break;
    }
  }

  std::string logLevelToString(const LogLevel& level) const {
    switch (level) {
      case LogLevel::DEBUG:
        return "DEBUG";
      case LogLevel::INFO:
        return "INFO";
      case LogLevel::WARNING:
        return "WARNING";
      case LogLevel::ERROR:
        return "ERROR";
      default:
        return "UNKNOWN";
    }
  }
};

#endif