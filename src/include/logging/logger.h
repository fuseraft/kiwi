#ifndef LOGGER_H
#define LOGGER_H

#include <iostream>
#include <iomanip>
#include <ctime>
#include <string>

enum class LogLevel {
    DEBUG,
    INFO,
    WARNING,
    ERROR
};

enum class LogMode {
    CONSOLE,
    FILE
};

class Logger {
public:
    Logger(LogLevel minLogLevel = LogLevel::DEBUG, LogMode logMode = LogMode::CONSOLE) : minLogLevel(minLogLevel), logMode(logMode) {}

    // TODO: it would be great to have a descriptor to describe log output/formatting
    void log(LogLevel level, const std::string& message) const {
        if (level < minLogLevel) {
            return;
        }

        std::time_t now = std::time(nullptr);
        std::tm localTime;
        localtime_r(&now, &localTime);

        char timestamp[20];
        std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &localTime);

        switch (logMode) {
            case LogMode::CONSOLE:
                std::cout << "[" << timestamp << "] [" << logLevelToString(level) << "] " << message << std::endl;
            break;
            case LogMode::FILE:
                std::ofstream file(logFilePath, std::ios::app);
                if (file.is_open()) {
                    file << "[" << timestamp << "] [" << logLevelToString(level) << "] " << message << std::endl;
                    file.close();
                }
            break;
        }
    }

    void setLogFilePath(const std::string &filePath) {
        // TODO: add validation
        logFilePath = filePath;
    }

private:
    LogLevel minLogLevel;
    LogMode logMode;
    std::string logFilePath;

    std::string logLevelToString(LogLevel level) const {
        switch (level) {
            case LogLevel::DEBUG:   return "DEBUG";
            case LogLevel::INFO:    return "INFO";
            case LogLevel::WARNING: return "WARNING";
            case LogLevel::ERROR:   return "ERROR";
            default:                return "UNKNOWN";
        }
    }
};

#endif