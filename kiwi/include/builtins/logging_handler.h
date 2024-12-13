#ifndef KIWI_BUILTINS_LOGGING_H
#define KIWI_BUILTINS_LOGGING_H

#include "math/functions.h"
#include "logging/logger.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "typing/value.h"
#include "util/string.h"
#include "globals.h"

class LoggingBuiltinHandler {
 public:
  static KValue execute(const Token& token, const KName& builtin,
                        const std::vector<KValue>& args) {
    if (SAFEMODE) {
      return {};
    }

    switch (builtin) {
      case KName::Builtin_Logging_Debug:
      case KName::Builtin_Logging_Error:
      case KName::Builtin_Logging_Info:
      case KName::Builtin_Logging_Warn:
        return executeWriteLog(token, builtin, args);

      case KName::Builtin_Logging_FilePath:
        return executeFilePath(token, args);

      case KName::Builtin_Logging_Level:
        return executeLevel(token, args);

      case KName::Builtin_Logging_Mode:
        return executeMode(token, args);

      case KName::Builtin_Logging_TimestampFormat:
        return executeTimestampFormat(token, args);

      case KName::Builtin_Logging_EntryFormat:
        return executeEntryFormat(token, args);

      default:
        break;
    }

    throw UnknownBuiltinError(token, "");
  }

 private:
  static KValue executeFilePath(const Token& token,
                                const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, LoggingBuiltins.FilePath);
    }

    auto filePath = get_string(token, args.at(0));
    Logger::getInstance().setLogFilePath(filePath);

    return {};
  }

  static KValue executeLevel(const Token& token,
                             const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, LoggingBuiltins.Level);
    }

    auto level = Logger::loglevel_from_string(get_string(token, args.at(0)));
    Logger::getInstance().setMinimumLogLevel(level);

    return {};
  }

  static KValue executeMode(const Token& token,
                            const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, LoggingBuiltins.Mode);
    }

    auto mode = Logger::logmode_from_string(get_string(token, args.at(0)));
    Logger::getInstance().setLogMode(mode);

    return {};
  }

  static KValue executeEntryFormat(const Token& token,
                                   const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, LoggingBuiltins.EntryFormat);
    }

    auto entryFormat = get_string(token, args.at(0));
    Logger::getInstance().setEntryFormat(entryFormat);

    return {};
  }

  static KValue executeTimestampFormat(const Token& token,
                                       const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           LoggingBuiltins.TimestampFormat);
    }

    auto timestampFormat = get_string(token, args.at(0));
    Logger::getInstance().setTimestampFormat(timestampFormat);

    return {};
  }

  static KValue executeWriteLog(const Token& token, const KName& builtin,
                                const std::vector<KValue>& args) {
    if (args.size() != 1 && args.size() != 2) {
      switch (builtin) {
        case KName::Builtin_Logging_Debug:
          throw BuiltinUnexpectedArgumentError(token, LoggingBuiltins.Debug);

        case KName::Builtin_Logging_Error:
          throw BuiltinUnexpectedArgumentError(token, LoggingBuiltins.Error);

        case KName::Builtin_Logging_Info:
          throw BuiltinUnexpectedArgumentError(token, LoggingBuiltins.Info);

        case KName::Builtin_Logging_Warn:
          throw BuiltinUnexpectedArgumentError(token, LoggingBuiltins.Warn);

        default:
          break;
      }
    }

    auto message = get_string(token, args.at(0));
    k_string source;

    if (args.size() == 2) {
      source = get_string(token, args.at(1));
    }

    switch (builtin) {
      case KName::Builtin_Logging_Debug:
        Logger::getInstance().debug(token, message, source);
        break;

      case KName::Builtin_Logging_Error:
        Logger::getInstance().error(token, message, source);
        break;

      case KName::Builtin_Logging_Info:
        Logger::getInstance().info(token, message, source);
        break;

      case KName::Builtin_Logging_Warn:
        Logger::getInstance().warn(token, message, source);
        break;

      default:
        break;
    }

    return {};
  }
};

#endif
