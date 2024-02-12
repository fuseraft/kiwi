#ifndef KIWI_BUILTINS_FILEIOHANDLER_H
#define KIWI_BUILTINS_FILEIOHANDLER_H

#include <vector>
#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "system/fileio.h"
#include "typing/valuetype.h"

class FileIOBuiltinHandler {
 public:
  static Value execute(const Token& tokenTerm, const std::string& builtin,
                       const std::vector<Value>& args) {
    if (builtin == FileIOBuiltIns.CreateFile) {
      return executeCreateFile(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.FileExists) {
      return executeFileExists(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.DeleteFile) {
      return executeDeleteFile(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.GetFileExtension) {
      return executeGetFileExtension(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.GetCurrentDirectory) {
      return executeGetCurrentDirectory(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.FileName) {
      return executeGetFileName(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.GetFilePath) {
      return executeGetFilePath(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.GetFileAbsolutePath) {
      return executeGetFileAbsolutePath(tokenTerm, args);
    } else if (builtin == FileIOBuiltIns.Glob) {
      return executeGlob(tokenTerm, args);
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

 private:
  static bool executeAppendText(const Token& tokenTerm,
                                const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           FileIOBuiltIns.AppendText);
    }

    std::string fileName = get_string(tokenTerm, args.at(0));
    Value value = args.at(1);
    return FileIO::writeToFile(fileName, value, true, false);
  }

  static bool executeCopyFile(const Token& tokenTerm,
                              const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, FileIOBuiltIns.CopyFile);
    }

    std::string sourcePath = get_string(tokenTerm, args.at(0));
    std::string destinationPath = get_string(tokenTerm, args.at(1));
    return FileIO::copyFile(sourcePath, destinationPath);
  }

  static bool executeCreateFile(const Token& tokenTerm,
                                const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           FileIOBuiltIns.CreateFile);
    }

    std::string fileName = get_string(tokenTerm, args.at(0));
    return FileIO::createFile(fileName);
  }

  static bool executeDeleteFile(const Token& tokenTerm,
                                const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           FileIOBuiltIns.DeleteFile);
    }

    std::string fileName = get_string(tokenTerm, args.at(0));
    return FileIO::deleteFile(fileName);
  }

  static double executeGetFileSize(const Token& tokenTerm,
                                   const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, FileIOBuiltIns.FileSize);
    }

    std::string fileName = get_string(tokenTerm, args.at(0));
    return FileIO::getFileSize(fileName);
  }

  static bool executeFileExists(const Token& tokenTerm,
                                const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           FileIOBuiltIns.FileExists);
    }

    std::string fileName = get_string(tokenTerm, args.at(0));
    return FileIO::fileExists(fileName);
  }

  static std::string executeGetFileAbsolutePath(
      const Token& tokenTerm, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           FileIOBuiltIns.GetFileAbsolutePath);
    }

    std::string fileName = get_string(tokenTerm, args.at(0));
    return FileIO::getAbsolutePath(fileName);
  }

  static std::string executeGetFileExtension(const Token& tokenTerm,
                                             const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           FileIOBuiltIns.GetFileExtension);
    }

    std::string fileName = get_string(tokenTerm, args.at(0));
    return FileIO::getFileExtension(fileName);
  }

  static std::string executeGetFileName(const Token& tokenTerm,
                                        const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, FileIOBuiltIns.FileName);
    }

    std::string fileName = get_string(tokenTerm, args.at(0));
    return FileIO::getFileName(fileName);
  }

  static std::string executeGetCurrentDirectory(
      const Token& tokenTerm, const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           FileIOBuiltIns.GetCurrentDirectory);
    }

    return FileIO::getCurrentWorkingDirectory();
  }

  static std::string executeGetFilePath(const Token& tokenTerm,
                                        const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm,
                                           FileIOBuiltIns.GetFilePath);
    }

    std::string fileName = get_string(tokenTerm, args.at(0));
    return FileIO::getParentPath(fileName);
  }

  static std::shared_ptr<List> executeGlob(const Token& tokenTerm,
                                           const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, FileIOBuiltIns.Glob);
    }

    std::string glob = get_string(tokenTerm, args.at(0));
    auto matchedFiles = FileIO::expandGlob(glob);

    std::shared_ptr<List> matchList = std::make_shared<List>();

    for (const auto& file : matchedFiles) {
      matchList->elements.push_back(file);
    }

    return matchList;
  }

  static bool executeMoveFile(const Token& tokenTerm,
                              const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, FileIOBuiltIns.MoveFile);
    }

    std::string sourcePath = get_string(tokenTerm, args.at(0));
    std::string destinationPath = get_string(tokenTerm, args.at(1));
    return FileIO::moveFile(sourcePath, destinationPath);
  }

  static std::string executeReadFile(const Token& tokenTerm,
                                     const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, FileIOBuiltIns.ReadFile);
    }

    std::string fileName = get_string(tokenTerm, args.at(0));
    return FileIO::readFile(fileName);
  }

  static std::shared_ptr<List> executeReadLines(
      const Token& tokenTerm, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, FileIOBuiltIns.ReadLines);
    }
    std::string fileName = get_string(tokenTerm, args.at(0));
    return FileIO::readLines(fileName);
  }

  static bool executeWriteLine(const Token& tokenTerm,
                               const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, FileIOBuiltIns.WriteLine);
    }

    std::string fileName = get_string(tokenTerm, args.at(0));
    Value value = args.at(1);
    return FileIO::writeToFile(fileName, value, true, true);
  }

  static bool executeWriteText(const Token& tokenTerm,
                               const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(tokenTerm, FileIOBuiltIns.WriteText);
    }

    std::string fileName = get_string(tokenTerm, args.at(0));
    Value value = args.at(1);
    return FileIO::writeToFile(fileName, value, false, false);
  }
};

#endif