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

class Logger {
public:
    Logger(LogLevel minLogLevel = LogLevel::DEBUG) : minLogLevel(minLogLevel) {}

    void log(LogLevel level, const std::string& message) const {
        if (level < minLogLevel) {
            return;
        }

        std::time_t now = std::time(nullptr);
        std::tm localTime;
        localtime_r(&now, &localTime);  // Use localtime_r for thread-safe version

        char timestamp[20];
        std::strftime(timestamp, sizeof(timestamp), "%Y-%m-%d %H:%M:%S", &localTime);

        std::cout << "[" << timestamp << "] [" << logLevelToString(level) << "] " << message << std::endl;

        /* TODO: implement
        std::string logpath("logfile.txt");
        std::ofstream logfile(logpath, std::ios::app);
        if (logfile.is_open()) {
            logfile << "[" << timestamp << "] [" << logLevelToString(level) << "] " << message << std::endl;
            logfile.close();
        }
        */
    }


private:
    LogLevel minLogLevel;

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