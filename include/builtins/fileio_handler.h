#ifndef KIWI_BUILTINS_FILEIOHANDLER_H
#define KIWI_BUILTINS_FILEIOHANDLER_H

#include <vector>
#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "util/file.h"
#include "typing/valuetype.h"

class FileIOBuiltinHandler {
 public:
  static Value execute(const Token& token, const SubTokenType& builtin,
                       const std::vector<Value>& args) {
    if (builtin == SubTokenType::Builtin_FileIO_CreateFile) {
      return executeCreateFile(token, args);
    } else if (builtin == SubTokenType::Builtin_FileIO_FileExists) {
      return executeFileExists(token, args);
    } else if (builtin == SubTokenType::Builtin_FileIO_IsDirectory) {
      return executeIsDirectory(token, args);
    } else if (builtin == SubTokenType::Builtin_FileIO_ListDirectory) {
      return executeListDirectory(token, args);
    } else if (builtin == SubTokenType::Builtin_FileIO_MakeDirectory) {
      return executeMakeDirectory(token, args);
    } else if (builtin == SubTokenType::Builtin_FileIO_MakeDirectoryP) {
      return executeMakeDirectoryP(token, args);
    } else if (builtin == SubTokenType::Builtin_FileIO_DeleteFile ||
               builtin == SubTokenType::Builtin_FileIO_RemoveDirectory) {
      return executeRemovePath(token, args);
    } else if (builtin == SubTokenType::Builtin_FileIO_RemoveDirectoryF) {
      return executeRemovePathF(token, args);
    } else if (builtin == SubTokenType::Builtin_FileIO_TempDir) {
      return executeGetTempDirectory(token, args);
    } else if (builtin == SubTokenType::Builtin_FileIO_GetFileExtension) {
      return executeGetFileExtension(token, args);
    } else if (builtin == SubTokenType::Builtin_FileIO_GetCurrentDirectory) {
      return executeGetCurrentDirectory(token, args);
    } else if (builtin == SubTokenType::Builtin_FileIO_ChangeDirectory) {
      return executeChangeDirectory(token, args);
    } else if (builtin == SubTokenType::Builtin_FileIO_FileName) {
      return executeGetFileName(token, args);
    } else if (builtin == SubTokenType::Builtin_FileIO_GetFilePath) {
      return executeGetFilePath(token, args);
    } else if (builtin == SubTokenType::Builtin_FileIO_GetFileAbsolutePath) {
      return executeGetFileAbsolutePath(token, args);
    } else if (builtin == SubTokenType::Builtin_FileIO_Glob) {
      return executeGlob(token, args);
    } else if (builtin == SubTokenType::Builtin_FileIO_AppendText) {
      return executeAppendText(token, args);
    } else if (builtin == SubTokenType::Builtin_FileIO_WriteText) {
      return executeWriteText(token, args);
    } else if (builtin == SubTokenType::Builtin_FileIO_WriteLine) {
      return executeWriteLine(token, args);
    } else if (builtin == SubTokenType::Builtin_FileIO_ReadFile) {
      return executeReadFile(token, args);
    } else if (builtin == SubTokenType::Builtin_FileIO_FileSize) {
      return executeGetFileSize(token, args);
    } else if (builtin == SubTokenType::Builtin_FileIO_CopyFile) {
      return executeCopyFile(token, args);
    } else if (builtin == SubTokenType::Builtin_FileIO_CopyR) {
      return executeCopyR(token, args);
    } else if (builtin == SubTokenType::Builtin_FileIO_Combine) {
      return executeCombine(token, args);
    } else if (builtin == SubTokenType::Builtin_FileIO_MoveFile) {
      return executeMoveFile(token, args);
    } else if (builtin == SubTokenType::Builtin_FileIO_ReadLines) {
      return executeReadLines(token, args);
    } else {
      throw UnknownBuiltinError(token, "");
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
    return File::writeToFile(fileName, value, true, false);
  }

  static bool executeCopyFile(const Token& token,
                              const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.CopyFile);
    }

    auto sourcePath = get_string(token, args.at(0));
    auto destinationPath = get_string(token, args.at(1));
    return File::copyFile(sourcePath, destinationPath);
  }

  static bool executeCopyR(const Token& token, const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.CopyR);
    }

    auto sourcePath = get_string(token, args.at(0));
    auto destinationPath = get_string(token, args.at(1));
    return File::copyR(sourcePath, destinationPath);
  }

  static bool executeCreateFile(const Token& token,
                                const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.CreateFile);
    }

    auto fileName = get_string(token, args.at(0));
    return File::createFile(fileName);
  }

  static std::shared_ptr<List> executeListDirectory(
      const Token& token, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.ListDirectory);
    }

    auto path = get_string(token, args.at(0));
    auto list = std::make_shared<List>();

    for (const auto& entry : File::listDirectory(path)) {
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
    return File::makeDirectory(path);
  }

  static bool executeMakeDirectoryP(const Token& token,
                                    const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.MakeDirectoryP);
    }

    auto path = get_string(token, args.at(0));
    return File::makeDirectoryP(path);
  }

  static bool executeRemovePath(const Token& token,
                                const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.RemoveDirectory);
    }

    auto path = get_string(token, args.at(0));
    return File::removePath(path);
  }

  static int executeRemovePathF(const Token& token,
                                const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.RemoveDirectoryF);
    }

    auto path = get_string(token, args.at(0));
    return File::removePathF(path);
  }

  static std::string executeGetTempDirectory(const Token& token,
                                             const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.TempDir);
    }

    return File::getTempDirectory();
  }

  static double executeGetFileSize(const Token& token,
                                   const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.FileSize);
    }

    auto fileName = get_string(token, args.at(0));
    return File::getFileSize(fileName);
  }

  static bool executeFileExists(const Token& token,
                                const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.FileExists);
    }

    auto fileName = get_string(token, args.at(0));
    return File::fileExists(fileName);
  }

  static bool executeIsDirectory(const Token& token,
                                 const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.IsDirectory);
    }

    auto path = get_string(token, args.at(0));
    return File::directoryExists(path);
  }

  static std::string executeGetFileAbsolutePath(
      const Token& token, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.GetFileAbsolutePath);
    }

    auto fileName = get_string(token, args.at(0));
    return File::getAbsolutePath(fileName);
  }

  static std::string executeGetFileExtension(const Token& token,
                                             const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.GetFileExtension);
    }

    std::string fileName = get_string(token, args.at(0));
    return File::getFileExtension(fileName);
  }

  static std::string executeGetFileName(const Token& token,
                                        const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.FileName);
    }

    auto fileName = get_string(token, args.at(0));
    return File::getFileName(fileName);
  }

  static std::string executeGetCurrentDirectory(
      const Token& token, const std::vector<Value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.GetCurrentDirectory);
    }

    return File::getCurrentDirectory();
  }

  static bool executeChangeDirectory(const Token& token,
                                     const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.ChangeDirectory);
    }

    auto path = get_string(token, args.at(0));
    return File::setCurrentDirectory(path);
  }

  static std::string executeGetFilePath(const Token& token,
                                        const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.GetFilePath);
    }

    auto fileName = get_string(token, args.at(0));
    return File::getParentPath(fileName);
  }

  static std::shared_ptr<List> executeGlob(const Token& token,
                                           const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.Glob);
    }

    auto glob = get_string(token, args.at(0));
    auto matchedFiles = File::expandGlob(glob);
    auto matchList = std::make_shared<List>();

    for (const auto& file : matchedFiles) {
      matchList->elements.push_back(file);
    }

    return matchList;
  }

  static std::string executeCombine(const Token& token,
                              const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.Combine);
    }

    auto firstPath = get_string(token, args.at(0));
    auto secondPath = get_string(token, args.at(1));
    return File::joinPath(firstPath, secondPath);
  }

  static bool executeMoveFile(const Token& token,
                              const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.MoveFile);
    }

    auto sourcePath = get_string(token, args.at(0));
    auto destinationPath = get_string(token, args.at(1));
    return File::movePath(sourcePath, destinationPath);
  }

  static std::string executeReadFile(const Token& token,
                                     const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.ReadFile);
    }

    auto fileName = get_string(token, args.at(0));
    return File::readFile(fileName);
  }

  static std::shared_ptr<List> executeReadLines(
      const Token& token, const std::vector<Value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.ReadLines);
    }

    auto fileName = get_string(token, args.at(0));
    auto list = std::make_shared<List>();
    for (const auto& line : File::readLines(fileName)) {
      list->elements.push_back(line);
    }

    return list;
  }

  static bool executeWriteLine(const Token& token,
                               const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.WriteLine);
    }

    auto fileName = get_string(token, args.at(0));
    auto value = args.at(1);
    return File::writeToFile(fileName, value, true, true);
  }

  static bool executeWriteText(const Token& token,
                               const std::vector<Value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.WriteText);
    }

    auto fileName = get_string(token, args.at(0));
    auto value = args.at(1);
    return File::writeToFile(fileName, value, false, false);
  }
};

#endif