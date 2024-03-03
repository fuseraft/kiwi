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
  static Value execute(const Token& token, const std::string& builtin,
                       const std::vector<Value>& args) {
    if (builtin == FileIOBuiltIns.CreateFile) {
      return executeCreateFile(token, args);
    } else if (builtin == FileIOBuiltIns.FileExists) {
      return executeFileExists(token, args);
    } else if (builtin == FileIOBuiltIns.IsDirectory) {
      return executeIsDirectory(token, args);
    } else if (builtin == FileIOBuiltIns.ListDirectory) {
      return executeListDirectory(token, args);
    } else if (builtin == FileIOBuiltIns.MakeDirectory) {
      return executeMakeDirectory(token, args);
    } else if (builtin == FileIOBuiltIns.MakeDirectoryP) {
      return executeMakeDirectoryP(token, args);
    } else if (builtin == FileIOBuiltIns.DeleteFile ||
               builtin == FileIOBuiltIns.RemoveDirectory) {
      return executeRemovePath(token, args, builtin);
    } else if (builtin == FileIOBuiltIns.RemoveDirectoryF) {
      return executeRemovePathF(token, args);
    } else if (builtin == FileIOBuiltIns.TempDir) {
      return executeGetTempDirectory(token, args);
    } else if (builtin == FileIOBuiltIns.GetFileExtension) {
      return executeGetFileExtension(token, args);
    } else if (builtin == FileIOBuiltIns.GetCurrentDirectory) {
      return executeGetCurrentDirectory(token, args);
    } else if (builtin == FileIOBuiltIns.ChangeDirectory) {
      return executeChangeDirectory(token, args);
    } else if (builtin == FileIOBuiltIns.FileName) {
      return executeGetFileName(token, args);
    } else if (builtin == FileIOBuiltIns.GetFilePath) {
      return executeGetFilePath(token, args);
    } else if (builtin == FileIOBuiltIns.GetFileAbsolutePath) {
      return executeGetFileAbsolutePath(token, args);
    } else if (builtin == FileIOBuiltIns.Glob) {
      return executeGlob(token, args);
    } else if (builtin == FileIOBuiltIns.AppendText) {
      return executeAppendText(token, args);
    } else if (builtin == FileIOBuiltIns.WriteText) {
      return executeWriteText(token, args);
    } else if (builtin == FileIOBuiltIns.WriteLine) {
      return executeWriteLine(token, args);
    } else if (builtin == FileIOBuiltIns.ReadFile) {
      return executeReadFile(token, args);
    } else if (builtin == FileIOBuiltIns.FileSize) {
      return executeGetFileSize(token, args);
    } else if (builtin == FileIOBuiltIns.CopyFile) {
      return executeCopyFile(token, args);
    } else if (builtin == FileIOBuiltIns.CopyR) {
      return executeCopyR(token, args);
    } else if (builtin == FileIOBuiltIns.MoveFile) {
      return executeMoveFile(token, args);
    } else if (builtin == FileIOBuiltIns.ReadLines) {
      return executeReadLines(token, args);
    } else {
      throw UnknownBuiltinError(token, builtin);
    }
  }

 private:
  static bool executeAppendText(const Token& token,
                                const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.AppendText);
    }

    auto fileName = get_string(token, args.at(0));
    auto value = args.at(1);
    return FileIO::writeToFile(fileName, value, true, false);
  }

  static bool executeCopyFile(const Token& token,
                              const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.CopyFile);
    }

    auto sourcePath = get_string(token, args.at(0));
    auto destinationPath = get_string(token, args.at(1));
    return FileIO::copyFile(sourcePath, destinationPath);
  }

  static bool executeCopyR(const Token& token, const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.CopyR);
    }

    auto sourcePath = get_string(token, args.at(0));
    auto destinationPath = get_string(token, args.at(1));
    return FileIO::copyR(sourcePath, destinationPath);
  }

  static bool executeCreateFile(const Token& token,
                                const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.CreateFile);
    }

    auto fileName = get_string(token, args.at(0));
    return FileIO::createFile(fileName);
  }

  static std::shared_ptr<List> executeListDirectory(
      const Token& token, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.ListDirectory);
    }

    auto path = get_string(token, args.at(0));
    auto list = std::make_shared<List>();

    for (const auto& entry : FileIO::listDirectory(path)) {
      list->elements.push_back(entry);
    }

    return list;
  }

  static bool executeMakeDirectory(const Token& token,
                                   const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.MakeDirectory);
    }

    auto path = get_string(token, args.at(0));
    return FileIO::makeDirectory(path);
  }

  static bool executeMakeDirectoryP(const Token& token,
                                    const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.MakeDirectoryP);
    }

    auto path = get_string(token, args.at(0));
    return FileIO::makeDirectoryP(path);
  }

  static bool executeRemovePath(const Token& token,
                                const std::vector<Value>& args,
                                const std::string& builtin) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, builtin);
    }

    auto path = get_string(token, args.at(0));
    return FileIO::removePath(path);
  }

  static int executeRemovePathF(const Token& token,
                                const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.RemoveDirectoryF);
    }

    auto path = get_string(token, args.at(0));
    return FileIO::removePathF(path);
  }

  static std::string executeGetTempDirectory(const Token& token,
                                             const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.TempDir);
    }

    return FileIO::getTempDirectory();
  }

  static double executeGetFileSize(const Token& token,
                                   const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.FileSize);
    }

    auto fileName = get_string(token, args.at(0));
    return FileIO::getFileSize(fileName);
  }

  static bool executeFileExists(const Token& token,
                                const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.FileExists);
    }

    auto fileName = get_string(token, args.at(0));
    return FileIO::fileExists(fileName);
  }

  static bool executeIsDirectory(const Token& token,
                                 const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.IsDirectory);
    }

    auto path = get_string(token, args.at(0));
    return FileIO::directoryExists(path);
  }

  static std::string executeGetFileAbsolutePath(
      const Token& token, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.GetFileAbsolutePath);
    }

    auto fileName = get_string(token, args.at(0));
    return FileIO::getAbsolutePath(fileName);
  }

  static std::string executeGetFileExtension(const Token& token,
                                             const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.GetFileExtension);
    }

    std::string fileName = get_string(token, args.at(0));
    return FileIO::getFileExtension(fileName);
  }

  static std::string executeGetFileName(const Token& token,
                                        const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.FileName);
    }

    auto fileName = get_string(token, args.at(0));
    return FileIO::getFileName(fileName);
  }

  static std::string executeGetCurrentDirectory(
      const Token& token, const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.GetCurrentDirectory);
    }

    return FileIO::getCurrentDirectory();
  }

  static bool executeChangeDirectory(const Token& token,
                                     const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.ChangeDirectory);
    }

    auto path = get_string(token, args.at(0));
    return FileIO::setCurrentDirectory(path);
  }

  static std::string executeGetFilePath(const Token& token,
                                        const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.GetFilePath);
    }

    auto fileName = get_string(token, args.at(0));
    return FileIO::getParentPath(fileName);
  }

  static std::shared_ptr<List> executeGlob(const Token& token,
                                           const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.Glob);
    }

    auto glob = get_string(token, args.at(0));
    auto matchedFiles = FileIO::expandGlob(glob);
    auto matchList = std::make_shared<List>();

    for (const auto& file : matchedFiles) {
      matchList->elements.push_back(file);
    }

    return matchList;
  }

  static bool executeMoveFile(const Token& token,
                              const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.MoveFile);
    }

    auto sourcePath = get_string(token, args.at(0));
    auto destinationPath = get_string(token, args.at(1));
    return FileIO::moveFile(sourcePath, destinationPath);
  }

  static std::string executeReadFile(const Token& token,
                                     const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.ReadFile);
    }

    auto fileName = get_string(token, args.at(0));
    return FileIO::readFile(fileName);
  }

  static std::shared_ptr<List> executeReadLines(
      const Token& token, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.ReadLines);
    }

    auto fileName = get_string(token, args.at(0));
    return FileIO::readLines(fileName);
  }

  static bool executeWriteLine(const Token& token,
                               const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.WriteLine);
    }

    auto fileName = get_string(token, args.at(0));
    auto value = args.at(1);
    return FileIO::writeToFile(fileName, value, true, true);
  }

  static bool executeWriteText(const Token& token,
                               const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.WriteText);
    }

    auto fileName = get_string(token, args.at(0));
    auto value = args.at(1);
    return FileIO::writeToFile(fileName, value, false, false);
  }
};

#endif