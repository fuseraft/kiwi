#ifndef KIWI_INTERPBUILTIN_H
#define KIWI_INTERPBUILTIN_H

#include <string>
#include <variant>
#include <vector>
#include "errors/error.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "system/fileio.h"
#include "typing/valuetype.h"

class BuiltinInterpreter {
 public:
  static std::variant<int, double, bool, std::string> execute(
      const Token& tokenTerm, const std::string& functionName,
      const std::vector<std::variant<int, double, bool, std::string>>& args) {

    if (FileIOBuiltIns.is_builtin(functionName)) {
      return executeFileIOBuiltin(tokenTerm, functionName, args);
    }

    return false;
  }

 private:
  static std::variant<int, double, bool, std::string> executeFileIOBuiltin(
      const Token& tokenTerm, const std::string& functionName,
      const std::vector<std::variant<int, double, bool, std::string>>& args) {
    if (functionName == FileIOBuiltIns.CreateFile) {
      return executeCreateFile(tokenTerm, args);
    } else if (functionName == FileIOBuiltIns.FileExists) {
      return executeFileExists(tokenTerm, args);
    } else if (functionName == FileIOBuiltIns.DeleteFile) {
      return executeDeleteFile(tokenTerm, args);
    } else {
      throw UnknownBuiltinError(tokenTerm, functionName);
    }
  }

  static std::string getString(
      const Token& tokenTerm,
      const std::variant<int, double, bool, std::string>& arg) {
    ValueType vt = get_value_type(arg);
    if (vt != ValueType::String) {
      throw ConversionError(tokenTerm);
    }
    return std::get<std::string>(arg);
  }

  static bool executeCreateFile(
      const Token& tokenTerm,
      const std::vector<std::variant<int, double, bool, std::string>>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           FileIOBuiltIns.CreateFile);
    }

    std::string fileName = getString(tokenTerm, args.at(0));
    return FileIO::createFile(fileName);
  }

  static bool executeDeleteFile(
      const Token& tokenTerm,
      const std::vector<std::variant<int, double, bool, std::string>>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           FileIOBuiltIns.DeleteFile);
    }

    std::string fileName = getString(tokenTerm, args.at(0));
    return FileIO::deleteFile(fileName);
  }

  static bool executeFileExists(
      const Token& tokenTerm,
      const std::vector<std::variant<int, double, bool, std::string>>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           FileIOBuiltIns.FileExists);
    }

    std::string fileName = getString(tokenTerm, args.at(0));
    return FileIO::fileExists(fileName);
  }
};

#endif
