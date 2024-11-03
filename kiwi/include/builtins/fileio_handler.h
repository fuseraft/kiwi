#ifndef KIWI_BUILTINS_FILEIOHANDLER_H
#define KIWI_BUILTINS_FILEIOHANDLER_H

#include <vector>
#include "math/functions.h"
#include "parsing/builtins.h"
#include "parsing/tokens.h"
#include "util/file.h"
#include "typing/value.h"
#include "globals.h"

class FileIOBuiltinHandler {
 public:
  static k_value execute(const Token& token, const KName& builtin,
                         const std::vector<k_value>& args) {
    if (SAFEMODE) {
      return static_cast<k_int>(0);
    }
    
    switch (builtin) {
      case KName::Builtin_FileIO_CreateFile:
        return executeCreateFile(token, args);

      case KName::Builtin_FileIO_FileExists:
        return executeFileExists(token, args);

      case KName::Builtin_FileIO_IsDirectory:
        return executeIsDirectory(token, args);

      case KName::Builtin_FileIO_ListDirectory:
        return executeListDirectory(token, args);

      case KName::Builtin_FileIO_MakeDirectory:
        return executeMakeDirectory(token, args);

      case KName::Builtin_FileIO_MakeDirectoryP:
        return executeMakeDirectoryP(token, args);

      case KName::Builtin_FileIO_DeleteFile:
      case KName::Builtin_FileIO_RemoveDirectory:
        return executeRemovePath(token, args);

      case KName::Builtin_FileIO_RemoveDirectoryF:
        return executeRemovePathF(token, args);

      case KName::Builtin_FileIO_TempDir:
        return executeGetTempDirectory(token, args);

      case KName::Builtin_FileIO_GetFileExtension:
        return executeGetFileExtension(token, args);

      case KName::Builtin_FileIO_GetCurrentDirectory:
        return executeGetCurrentDirectory(token, args);

      case KName::Builtin_FileIO_ChangeDirectory:
        return executeChangeDirectory(token, args);

      case KName::Builtin_FileIO_FileName:
        return executeGetFileName(token, args);

      case KName::Builtin_FileIO_GetFilePath:
        return executeGetFilePath(token, args);

      case KName::Builtin_FileIO_GetFileAbsolutePath:
        return executeGetFileAbsolutePath(token, args);

      case KName::Builtin_FileIO_Glob:
        return executeGlob(token, args);

      case KName::Builtin_FileIO_AppendText:
        return executeAppendText(token, args);

      case KName::Builtin_FileIO_WriteText:
        return executeWriteText(token, args);

      case KName::Builtin_FileIO_WriteLine:
        return executeWriteLine(token, args);

      case KName::Builtin_FileIO_ReadFile:
        return executeReadFile(token, args);

      case KName::Builtin_FileIO_ReadBytes:
        return executeReadBytes(token, args);

      case KName::Builtin_FileIO_FileSize:
        return executeGetFileSize(token, args);

      case KName::Builtin_FileIO_CopyFile:
        return executeCopyFile(token, args);

      case KName::Builtin_FileIO_CopyR:
        return executeCopyR(token, args);

      case KName::Builtin_FileIO_Combine:
        return executeCombine(token, args);

      case KName::Builtin_FileIO_MoveFile:
        return executeMoveFile(token, args);

      case KName::Builtin_FileIO_ReadLines:
        return executeReadLines(token, args);

      case KName::Builtin_FileIO_WriteBytes:
        return executeWriteBytes(token, args);

      default:
        break;
    }

    throw UnknownBuiltinError(token, "");
  }

 private:
  static k_value executeAppendText(const Token& token,
                                   const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.AppendText);
    }

    auto fileName = get_string(token, args.at(0));
    auto value = args.at(1);
    return File::writeToFile(token, fileName, value, true, false);
  }

  static k_value executeCopyFile(const Token& token,
                                 const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.CopyFile);
    }

    auto sourcePath = get_string(token, args.at(0));
    auto destinationPath = get_string(token, args.at(1));
    return File::copyFile(token, sourcePath, destinationPath);
  }

  static k_value executeCopyR(const Token& token,
                              const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.CopyR);
    }

    auto sourcePath = get_string(token, args.at(0));
    auto destinationPath = get_string(token, args.at(1));
    return File::copyR(token, sourcePath, destinationPath);
  }

  static k_value executeCreateFile(const Token& token,
                                   const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.CreateFile);
    }

    auto fileName = get_string(token, args.at(0));
    return File::createFile(token, fileName);
  }

  static k_value executeListDirectory(const Token& token,
                                      const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.ListDirectory);
    }

    auto path = get_string(token, args.at(0));
    auto list = std::make_shared<List>();
    auto& elements = list->elements;
    const auto& listing = File::listDirectory(token, path);
    elements.reserve(listing.size());

    for (const auto& entry : listing) {
      elements.emplace_back(entry);
    }

    return list;
  }

  static k_value executeMakeDirectory(const Token& token,
                                      const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.MakeDirectory);
    }

    auto path = get_string(token, args.at(0));
    return File::makeDirectory(token, path);
  }

  static k_value executeMakeDirectoryP(const Token& token,
                                       const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.MakeDirectoryP);
    }

    auto path = get_string(token, args.at(0));
    return File::makeDirectoryP(token, path);
  }

  static k_value executeRemovePath(const Token& token,
                                   const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.RemoveDirectory);
    }

    auto path = get_string(token, args.at(0));
    return File::removePath(token, path);
  }

  static k_value executeRemovePathF(const Token& token,
                                    const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.RemoveDirectoryF);
    }

    auto path = get_string(token, args.at(0));
    return static_cast<k_int>(File::removePathF(token, path));
  }

  static k_value executeGetTempDirectory(const Token& token,
                                         const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.TempDir);
    }

    return File::getTempDirectory(token);
  }

  static k_value executeGetFileSize(const Token& token,
                                    const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.FileSize);
    }

    auto fileName = get_string(token, args.at(0));
    return File::getFileSize(token, fileName);
  }

  static k_value executeFileExists(const Token& token,
                                   const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.FileExists);
    }

    auto fileName = get_string(token, args.at(0));
    return File::fileExists(token, fileName);
  }

  static k_value executeIsDirectory(const Token& token,
                                    const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.IsDirectory);
    }

    auto path = get_string(token, args.at(0));
    return File::directoryExists(token, path);
  }

  static k_value executeGetFileAbsolutePath(const Token& token,
                                            const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.GetFileAbsolutePath);
    }

    auto fileName = get_string(token, args.at(0));
    return File::getAbsolutePath(token, fileName);
  }

  static k_value executeGetFileExtension(const Token& token,
                                         const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.GetFileExtension);
    }

    k_string fileName = get_string(token, args.at(0));
    return File::getFileExtension(token, fileName);
  }

  static k_value executeGetFileName(const Token& token,
                                    const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.FileName);
    }

    auto fileName = get_string(token, args.at(0));
    return File::getFileName(token, fileName);
  }

  static k_value executeGetCurrentDirectory(const Token& token,
                                            const std::vector<k_value>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.GetCurrentDirectory);
    }

    return File::getCurrentDirectory();
  }

  static k_value executeChangeDirectory(const Token& token,
                                        const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.ChangeDirectory);
    }

    auto path = get_string(token, args.at(0));
    return File::setCurrentDirectory(path);
  }

  static k_value executeGetFilePath(const Token& token,
                                    const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.GetFilePath);
    }

    auto fileName = get_string(token, args.at(0));
    return File::getParentPath(token, fileName);
  }

  static k_value executeGlob(const Token& token,
                             const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.Glob);
    }

    auto glob = get_string(token, args.at(0));
    auto matchedFiles = File::expandGlob(token, glob);
    auto matchList = std::make_shared<List>();
    auto& elements = matchList->elements;
    elements.reserve(matchedFiles.size());

    for (const auto& file : matchedFiles) {
      elements.emplace_back(file);
    }

    return matchList;
  }

  static k_value executeCombine(const Token& token,
                                const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.Combine);
    }

    auto firstPath = get_string(token, args.at(0));
    auto secondPath = get_string(token, args.at(1));
    return File::joinPath(firstPath, secondPath);
  }

  static k_value executeMoveFile(const Token& token,
                                 const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.MoveFile);
    }

    auto sourcePath = get_string(token, args.at(0));
    auto destinationPath = get_string(token, args.at(1));
    return File::movePath(token, sourcePath, destinationPath);
  }

  static k_value executeReadFile(const Token& token,
                                 const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.ReadFile);
    }

    auto fileName = get_string(token, args.at(0));
    return File::readFile(token, fileName);
  }

  static k_value executeReadLines(const Token& token,
                                  const std::vector<k_value>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.ReadLines);
    }

    auto fileName = get_string(token, args.at(0));
    auto lines = File::readLines(token, fileName);

    auto list = std::make_shared<List>();
    auto& elements = list->elements;
    elements.reserve(lines.size());

    for (const auto& line : lines) {
      elements.emplace_back(line);
    }

    return list;
  }

  static k_value executeReadBytes(const Token& token,
                                  const std::vector<k_value>& args) {
    if (args.size() != 1 && args.size() != 3) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.ReadBytes);
    }

    auto fileName = get_string(token, args.at(0));
    auto list = std::make_shared<List>();
    auto& elements = list->elements;

    if (args.size() == 1) {
      auto bytes = File::readBytes(token, fileName);
      elements.reserve(bytes.size());

      for (const auto& byte : bytes) {
        elements.emplace_back(
            static_cast<k_int>(static_cast<unsigned char>(byte)));
      }
    } else {
      auto offset = get_integer(token, args.at(1));
      auto size = get_integer(token, args.at(2));

      auto bytes = File::readBytes(token, fileName, offset, size);
      elements.reserve(bytes.size());

      for (const auto& byte : bytes) {
        elements.emplace_back(static_cast<k_int>(byte));
      }
    }

    return list;
  }

  static k_value executeWriteLine(const Token& token,
                                  const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.WriteLine);
    }

    auto fileName = get_string(token, args.at(0));
    auto value = args.at(1);
    return File::writeToFile(token, fileName, value, true, true);
  }

  static k_value executeWriteText(const Token& token,
                                  const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.WriteText);
    }

    auto fileName = get_string(token, args.at(0));
    auto value = args.at(1);
    return File::writeToFile(token, fileName, value, false, false);
  }

  static k_value executeWriteBytes(const Token& token,
                                   const std::vector<k_value>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.WriteBytes);
    }

    auto fileName = get_string(token, args.at(0));
    auto value = args.at(1);

    if (!std::holds_alternative<k_list>(value)) {
      throw ConversionError(token, "Expected a list of bytes to write.");
    }

    auto elements = std::get<k_list>(value)->elements;
    std::vector<char> bytes;
    bytes.reserve(elements.size());

    for (const auto& item : elements) {
      if (!std::holds_alternative<k_int>(item)) {
        throw ConversionError(token, "Expected a list of bytes to write.");
      }

      bytes.emplace_back(static_cast<char>(std::get<k_int>(item)));
    }

    File::writeBytes(token, fileName, bytes);
    return true;
  }
};

#endif