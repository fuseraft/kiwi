#ifndef KIWI_INTERPBUILTIN_H
#define KIWI_INTERPBUILTIN_H

#include <charconv>
#include <sstream>
#include <string>
#include <vector>
#include "errors/error.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "system/fileio.h"
#include "typing/valuetype.h"

class BuiltinInterpreter {
 public:
  static Value execute(const Token& tokenTerm, const std::string& builtin,
                       const std::vector<Value>& args) {
    if (FileIOBuiltIns.is_builtin(builtin)) {
      return executeFileIOBuiltin(tokenTerm, builtin, args);
    }

    throw UnknownBuiltinError(tokenTerm, builtin);
  }

  static Value execute(const Token& tokenTerm, const std::string& builtin,
                       const Value& value, const std::vector<Value>& args) {
    if (KiwiBuiltins.is_builtin(builtin)) {
      return executeKiwiBuiltin(tokenTerm, builtin, value, args);
    }

    throw UnknownBuiltinError(tokenTerm, builtin);
  }

 private:
  static Value executeKiwiBuiltin(const Token& tokenTerm,
                                  const std::string& builtin,
                                  const Value& value,
                                  const std::vector<Value>& args) {
    if (builtin == KiwiBuiltins.Chars) {
      return executeChars(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.IsA) {

    } else if (builtin == KiwiBuiltins.Join) {
      return executeJoin(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.Size) {
      return executeSize(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.ToD) {
      return executeToDouble(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.ToI) {
      return executeToInteger(tokenTerm, value, args);
    } else if (builtin == KiwiBuiltins.ToS) {
      return executeToString(tokenTerm, value, args);
    }

    throw UnknownBuiltinError(tokenTerm, builtin);
  }

  static Value executeFileIOBuiltin(const Token& tokenTerm,
                                    const std::string& builtin,
                                    const std::vector<Value>& args) {
    if (builtin == FileIOBuiltIns.CreateFile) {
      return executeCreateFile(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.FileExists) {
      return executeFileExists(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.DeleteFile) {
      return executeDeleteFile(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.GetFileExtension) {
      return executeGetFileExtension(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.FileName) {
      return executeGetFileName(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.GetFilePath) {
      return executeGetFilePath(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.GetFileAbsolutePath) {
      return executeGetFileAbsolutePath(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.AppendText) {
      return executeAppendText(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.WriteText) {
      return executeWriteText(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.WriteLine) {
      return executeWriteLine(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.ReadFile) {
      return executeReadFile(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.FileSize) {
      return executeGetFileSize(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.CopyFile) {
      return executeCopyFile(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.MoveFile) {
      return executeMoveFile(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.ReadLines) {
      return executeReadLines(tokenTerm, args);
    } else {
      throw UnknownBuiltinError(tokenTerm, builtin);
    }
  }

  static std::string getString(const Token& tokenTerm, const Value& arg) {
    if (!std::holds_alternative<std::string>(arg)) {
      throw ConversionError(tokenTerm);
    }
    return std::get<std::string>(arg);
  }

  static std::shared_ptr<List> executeChars(const Token& tokenTerm,
                                            const Value& value,
                                            const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.Chars);
    }

    auto newList = std::make_shared<List>();
    std::string stringValue = getString(tokenTerm, value);
    for (char c : stringValue) {
      newList->elements.push_back(std::string(1, c));
    }
    return newList;
  }

  static std::string executeJoin(const Token& tokenTerm, const Value& value,
                                 const std::vector<Value>& args) {
    int argSize = args.size();

    if (argSize != 0 && argSize != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.Join);
    }

    if (!std::holds_alternative<std::shared_ptr<List>>(value)) {
      throw ConversionError(tokenTerm, "Cannot join a non-list type.");
    }

    auto list = std::get<std::shared_ptr<List>>(value);
    std::ostringstream sv;
    std::string joiner;

    if (argSize == 1) {
      joiner = getString(tokenTerm, args.at(0));
    }

    for (auto it = list->elements.begin(); it != list->elements.end(); ++it) {
      if (it != list->elements.begin()) {
        sv << joiner;
      }
      sv << get_value_string(*it);
    }

    return sv.str();
  }

  static int executeSize(const Token& tokenTerm, const Value& value,
                         const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.Size);
    }

    if (std::holds_alternative<std::string>(value)) {
      int size = std::get<std::string>(value).length();
      return size;
    } else if (std::holds_alternative<std::shared_ptr<List>>(value)) {
      auto list = std::get<std::shared_ptr<List>>(value);
      int size = list->elements.size();
      return size;
    } else {
      throw ConversionError(tokenTerm);
    }
  }

  static double executeToDouble(const Token& tokenTerm, const Value& value,
                                const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.ToD);
    }

    if (std::holds_alternative<std::string>(value)) {
      std::string stringValue = std::get<std::string>(value);
      double doubleValue = 0;
      auto [ptr, ec] =
          std::from_chars(stringValue.data(),
                          stringValue.data() + stringValue.size(), doubleValue);

      if (ec == std::errc()) {
        return doubleValue;
      } else {
        throw ConversionError(
            tokenTerm, "Cannot convert non-numeric value to a double: `" +
                           stringValue + "`");
      }
    } else if (std::holds_alternative<int>(value)) {
      return static_cast<double>(std::get<int>(value));
    } else {
      throw ConversionError(tokenTerm,
                            "Cannot convert non-numeric value to a double.");
    }
  }

  static int executeToInteger(const Token& tokenTerm, const Value& value,
                              const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.ToI);
    }

    if (std::holds_alternative<std::string>(value)) {
      std::string stringValue = std::get<std::string>(value);
      int intValue = 0;
      auto [ptr, ec] =
          std::from_chars(stringValue.data(),
                          stringValue.data() + stringValue.size(), intValue);

      if (ec == std::errc()) {
        return intValue;
      } else {
        throw ConversionError(
            tokenTerm, "Cannot convert non-numeric value to an integer: `" +
                           stringValue + "`");
      }
    } else if (std::holds_alternative<double>(value)) {
      return static_cast<int>(std::get<double>(value));
    } else {
      throw ConversionError(tokenTerm,
                            "Cannot convert non-numeric value to an integer.");
    }
  }

  static std::string executeToString(const Token& tokenTerm, const Value& value,
                                     const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, KiwiBuiltins.ToS);
    }

    return get_value_string(value);
  }

  static bool executeAppendText(const Token& tokenTerm,
                                const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           FileIOBuiltIns.AppendText);
    }

    std::string fileName = getString(tokenTerm, args.at(0));
    Value value = args.at(1);
    return FileIO::writeToFile(fileName, value, true, false);
  }

  static bool executeCopyFile(const Token& tokenTerm,
                              const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, FileIOBuiltIns.CopyFile);
    }

    std::string sourcePath = getString(tokenTerm, args.at(0));
    std::string destinationPath = getString(tokenTerm, args.at(1));
    return FileIO::copyFile(sourcePath, destinationPath);
  }

  static bool executeCreateFile(const Token& tokenTerm,
                                const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           FileIOBuiltIns.CreateFile);
    }

    std::string fileName = getString(tokenTerm, args.at(0));
    return FileIO::createFile(fileName);
  }

  static bool executeDeleteFile(const Token& tokenTerm,
                                const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           FileIOBuiltIns.DeleteFile);
    }

    std::string fileName = getString(tokenTerm, args.at(0));
    return FileIO::deleteFile(fileName);
  }

  static double executeGetFileSize(const Token& tokenTerm,
                                   const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, FileIOBuiltIns.FileSize);
    }

    std::string fileName = getString(tokenTerm, args.at(0));
    return FileIO::getFileSize(fileName);
  }

  static bool executeFileExists(const Token& tokenTerm,
                                const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           FileIOBuiltIns.FileExists);
    }

    std::string fileName = getString(tokenTerm, args.at(0));
    return FileIO::fileExists(fileName);
  }

  static std::string executeGetFileAbsolutePath(
      const Token& tokenTerm, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           FileIOBuiltIns.GetFileAbsolutePath);
    }

    std::string fileName = getString(tokenTerm, args.at(0));
    return FileIO::getAbsolutePath(fileName);
  }

  static std::string executeGetFileExtension(const Token& tokenTerm,
                                             const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           FileIOBuiltIns.GetFileExtension);
    }

    std::string fileName = getString(tokenTerm, args.at(0));
    return FileIO::getFileExtension(fileName);
  }

  static std::string executeGetFileName(const Token& tokenTerm,
                                        const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, FileIOBuiltIns.FileName);
    }

    std::string fileName = getString(tokenTerm, args.at(0));
    return FileIO::getFileName(fileName);
  }

  static std::string executeGetFilePath(const Token& tokenTerm,
                                        const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           FileIOBuiltIns.GetFilePath);
    }

    std::string fileName = getString(tokenTerm, args.at(0));
    return FileIO::getParentPath(fileName);
  }

  static bool executeMoveFile(const Token& tokenTerm,
                              const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, FileIOBuiltIns.MoveFile);
    }

    std::string sourcePath = getString(tokenTerm, args.at(0));
    std::string destinationPath = getString(tokenTerm, args.at(1));
    return FileIO::moveFile(sourcePath, destinationPath);
  }

  static std::string executeReadFile(const Token& tokenTerm,
                                     const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, FileIOBuiltIns.ReadFile);
    }

    std::string fileName = getString(tokenTerm, args.at(0));
    return FileIO::readFile(fileName);
  }

  static std::shared_ptr<List> executeReadLines(
      const Token& tokenTerm, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, FileIOBuiltIns.ReadLines);
    }
    std::string fileName = getString(tokenTerm, args.at(0));
    return FileIO::readLines(fileName);
  }

  static bool executeWriteLine(const Token& tokenTerm,
                               const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, FileIOBuiltIns.WriteLine);
    }

    std::string fileName = getString(tokenTerm, args.at(0));
    Value value = args.at(1);
    return FileIO::writeToFile(fileName, value, true, true);
  }

  static bool executeWriteText(const Token& tokenTerm,
                               const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, FileIOBuiltIns.WriteText);
    }

    std::string fileName = getString(tokenTerm, args.at(0));
    Value value = args.at(1);
    return FileIO::writeToFile(fileName, value, false, false);
  }
};

#endif
