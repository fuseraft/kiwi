#ifndef KIWI_BUILTINS_LOGGING_H
#define KIWI_BUILTINS_LOGGING_H

#include "math/functions.h"
#include "logging/logger.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/value.h"
#include "util/string.h"

class LoggingBuiltinHandler {
 public:
  static k_value execute(const Token& term, const KName& builtin,
                         const std::vector<k_value>& args) {
    switch (builtin) {
      case KName::Builtin_Logging_Debug:
      case KName::Builtin_Logging_Error:
      case KName::Builtin_Logging_Info:
      case KName::Builtin_Logging_Warn:
        return executeWriteLog(term, builtin, args);

      case KName::Builtin_Logging_FilePath:
        return executeFilePath(term, args);

      case KName::Builtin_Logging_Level:
        return executeLevel(term, args);

      case KName::Builtin_Logging_Mode:
        return executeMode(term, args);

      case KName::Builtin_Logging_TimestampFormat:
        return executeTimestampFormat(term, args);

      case KName::Builtin_Logging_EntryFormat:
        return executeEntryFormat(term, args);

      default:
        break;
    }

    throw UnknownBuiltinError(term, "");
  }

 private:
  static k_value executeFilePath(const Token& term,
                                 const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, LoggingBuiltins.FilePath);
    }

    auto filePath = get_string(term, args.at(0));
    Logger::getInstance().setLogFilePath(filePath);

    return static_cast<k_int>(0);
  }

  static k_value executeLevel(const Token& term,
                              const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, LoggingBuiltins.Level);
    }

    auto level = Logger::loglevel_from_string(get_string(term, args.at(0)));
    Logger::getInstance().setMinimumLogLevel(level);

    return static_cast<k_int>(0);
  }

  static k_value executeMode(const Token& term,
                             const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, LoggingBuiltins.Mode);
    }

    auto mode = Logger::logmode_from_string(get_string(term, args.at(0)));
    Logger::getInstance().setLogMode(mode);

    return static_cast<k_int>(0);
  }

  static k_value executeEntryFormat(const Token& term,
                                    const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term, LoggingBuiltins.EntryFormat);
    }

    auto entryFormat = get_string(term, args.at(0));
    Logger::getInstance().setEntryFormat(entryFormat);

    return static_cast<k_int>(0);
  }

  static k_value executeTimestampFormat(const Token& term,
                                        const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(term,
                                           LoggingBuiltins.TimestampFormat);
    }

    auto timestampFormat = get_string(term, args.at(0));
    Logger::getInstance().setTimestampFormat(timestampFormat);

    return static_cast<k_int>(0);
  }

  static k_value executeWriteLog(const Token& term, const KName& builtin,
                                 const std::vector<k_value>& args) {
    if (args.size() != 1 && args.size() != 2) {
      switch (builtin) {
        case KName::Builtin_Logging_Debug:
          throw BuiltinUnexpectedArgumentError(term, LoggingBuiltins.Debug);

        case KName::Builtin_Logging_Error:
          throw BuiltinUnexpectedArgumentError(term, LoggingBuiltins.Error);

        case KName::Builtin_Logging_Info:
          throw BuiltinUnexpectedArgumentError(term, LoggingBuiltins.Info);

        case KName::Builtin_Logging_Warn:
          throw BuiltinUnexpectedArgumentError(term, LoggingBuiltins.Warn);

        default:
          break;
      }
    }

    auto message = get_string(term, args.at(0));
    k_string source;

    if (args.size() == 2) {
      source = get_string(term, args.at(1));
    }

    switch (builtin) {
      case KName::Builtin_Logging_Debug:
        Logger::getInstance().debug(term, message, source);
        break;

      case KName::Builtin_Logging_Error:
        Logger::getInstance().error(term, message, source);
        break;

      case KName::Builtin_Logging_Info:
        Logger::getInstance().info(term, message, source);
        break;

      case KName::Builtin_Logging_Warn:
        Logger::getInstance().warn(term, message, source);
        break;

      default:
        break;
    }

    return static_cast<k_int>(0);
  }
};

#endif
