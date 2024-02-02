#ifndef KIWI_INTERPBUILTIN_H
#define KIWI_INTERPBUILTIN_H

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

    return false;
  }

 private:
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
    } else {
      throw UnknownBuiltinError(tokenTerm, builtin);
    }
  }

  static std::string getString(const Token& tokenTerm, const Value& arg) {
    ValueType vt = get_value_type(arg);
    if (vt != ValueType::String) {
      throw ConversionError(tokenTerm);
    }
    return std::get<std::string>(arg);
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
