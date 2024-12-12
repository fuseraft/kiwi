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
  static KValue execute(const Token& token, const KName& builtin,
                        const std::vector<KValue>& args) {
    if (SAFEMODE) {
      return {};
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
  static KValue executeAppendText(const Token& token,
                                  const std::vector<KValue>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.AppendText);
    }

    auto fileName = get_string(token, args.at(0));
    auto value = args.at(1);
    return KValue::createBoolean(
        File::writeToFile(token, fileName, value, true, false));
  }

  static KValue executeCopyFile(const Token& token,
                                const std::vector<KValue>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.CopyFile);
    }

    auto sourcePath = get_string(token, args.at(0));
    auto destinationPath = get_string(token, args.at(1));
    return KValue::createBoolean(
        File::copyFile(token, sourcePath, destinationPath));
  }

  static KValue executeCopyR(const Token& token,
                             const std::vector<KValue>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.CopyR);
    }

    auto sourcePath = get_string(token, args.at(0));
    auto destinationPath = get_string(token, args.at(1));
    return KValue::createBoolean(
        File::copyR(token, sourcePath, destinationPath));
  }

  static KValue executeCreateFile(const Token& token,
                                  const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.CreateFile);
    }

    auto fileName = get_string(token, args.at(0));
    return KValue::createBoolean(File::createFile(token, fileName));
  }

  static KValue executeListDirectory(const Token& token,
                                     const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.ListDirectory);
    }

    auto path = get_string(token, args.at(0));
    auto list = std::make_shared<List>();
    auto& elements = list->elements;
    const auto& listing = File::listDirectory(token, path);
    elements.reserve(listing.size());

    for (const auto& entry : listing) {
      elements.emplace_back(KValue::createString(entry));
    }

    return KValue::createList(list);
  }

  static KValue executeMakeDirectory(const Token& token,
                                     const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.MakeDirectory);
    }

    auto path = get_string(token, args.at(0));
    return KValue::createBoolean(File::makeDirectory(token, path));
  }

  static KValue executeMakeDirectoryP(const Token& token,
                                      const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.MakeDirectoryP);
    }

    auto path = get_string(token, args.at(0));
    return KValue::createBoolean(File::makeDirectoryP(token, path));
  }

  static KValue executeRemovePath(const Token& token,
                                  const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.RemoveDirectory);
    }

    auto path = get_string(token, args.at(0));
    return KValue::createBoolean(File::removePath(token, path));
  }

  static KValue executeRemovePathF(const Token& token,
                                   const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.RemoveDirectoryF);
    }

    auto path = get_string(token, args.at(0));
    return KValue::createInteger(File::removePathF(token, path));
  }

  static KValue executeGetTempDirectory(const Token& token,
                                        const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.TempDir);
    }

    return KValue::createString(File::getTempDirectory(token));
  }

  static KValue executeGetFileSize(const Token& token,
                                   const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.FileSize);
    }

    auto fileName = get_string(token, args.at(0));
    return KValue::createInteger(File::getFileSize(token, fileName));
  }

  static KValue executeFileExists(const Token& token,
                                  const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.FileExists);
    }

    auto fileName = get_string(token, args.at(0));
    return KValue::createBoolean(File::fileExists(token, fileName));
  }

  static KValue executeIsDirectory(const Token& token,
                                   const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.IsDirectory);
    }

    auto path = get_string(token, args.at(0));
    return KValue::createBoolean(File::directoryExists(token, path));
  }

  static KValue executeGetFileAbsolutePath(const Token& token,
                                           const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.GetFileAbsolutePath);
    }

    auto fileName = get_string(token, args.at(0));
    return KValue::createString(File::getAbsolutePath(token, fileName));
  }

  static KValue executeGetFileExtension(const Token& token,
                                        const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.GetFileExtension);
    }

    k_string fileName = get_string(token, args.at(0));
    return KValue::createString(File::getFileExtension(token, fileName));
  }

  static KValue executeGetFileName(const Token& token,
                                   const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.FileName);
    }

    auto fileName = get_string(token, args.at(0));
    return KValue::createString(File::getFileName(token, fileName));
  }

  static KValue executeGetCurrentDirectory(const Token& token,
                                           const std::vector<KValue>& args) {
    if (args.size() != 0) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.GetCurrentDirectory);
    }

    return KValue::createString(File::getCurrentDirectory());
  }

  static KValue executeChangeDirectory(const Token& token,
                                       const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token,
                                           FileIOBuiltIns.ChangeDirectory);
    }

    auto path = get_string(token, args.at(0));
    return KValue::createBoolean(File::setCurrentDirectory(path));
  }

  static KValue executeGetFilePath(const Token& token,
                                   const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.GetFilePath);
    }

    auto fileName = get_string(token, args.at(0));
    return KValue::createString(File::getParentPath(token, fileName));
  }

  static KValue executeGlob(const Token& token,
                            const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.Glob);
    }

    auto glob = get_string(token, args.at(0));
    auto matchedFiles = File::expandGlob(token, glob);
    auto matchList = std::make_shared<List>();
    auto& elements = matchList->elements;
    elements.reserve(matchedFiles.size());

    for (const auto& file : matchedFiles) {
      elements.emplace_back(KValue::createString(file));
    }

    return KValue::createList(matchList);
  }

  static KValue executeCombine(const Token& token,
                               const std::vector<KValue>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.Combine);
    }

    auto firstPath = get_string(token, args.at(0));
    auto secondPath = get_string(token, args.at(1));
    return KValue::createString(File::joinPath(firstPath, secondPath));
  }

  static KValue executeMoveFile(const Token& token,
                                const std::vector<KValue>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.MoveFile);
    }

    auto sourcePath = get_string(token, args.at(0));
    auto destinationPath = get_string(token, args.at(1));
    return KValue::createBoolean(
        File::movePath(token, sourcePath, destinationPath));
  }

  static KValue executeReadFile(const Token& token,
                                const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.ReadFile);
    }

    auto fileName = get_string(token, args.at(0));
    return KValue::createString(File::readFile(token, fileName));
  }

  static KValue executeReadLines(const Token& token,
                                 const std::vector<KValue>& args) {
    if (args.size() != 1) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.ReadLines);
    }

    auto fileName = get_string(token, args.at(0));
    auto lines = File::readLines(token, fileName);

    auto list = std::make_shared<List>();
    auto& elements = list->elements;
    elements.reserve(lines.size());

    for (const auto& line : lines) {
      elements.emplace_back(KValue::createString(line));
    }

    return KValue::createList(list);
  }

  static KValue executeReadBytes(const Token& token,
                                 const std::vector<KValue>& args) {
    if (args.size() != 1 && args.size() != 3) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.ReadBytes);
    }

    auto fileName = get_string(token, args.at(0));
    auto list = std::make_shared<List>();
    auto& elements = list->elements;
    KValue byteValue;

    if (args.size() == 1) {
      auto bytes = File::readBytes(token, fileName);
      elements.reserve(bytes.size());

      for (const auto& byte : bytes) {
        byteValue.setValue(
            static_cast<k_int>(static_cast<unsigned char>(byte)));
        elements.emplace_back(byteValue);
      }
    } else {
      auto offset = get_integer(token, args.at(1));
      auto size = get_integer(token, args.at(2));

      auto bytes = File::readBytes(token, fileName, offset, size);
      elements.reserve(bytes.size());

      for (const auto& byte : bytes) {
        byteValue.setValue(static_cast<k_int>(byte));
        elements.emplace_back(byteValue);
      }
    }

    return KValue::createList(list);
  }

  static KValue executeWriteLine(const Token& token,
                                 const std::vector<KValue>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.WriteLine);
    }

    auto fileName = get_string(token, args.at(0));
    auto value = args.at(1);
    return KValue::createBoolean(
        File::writeToFile(token, fileName, value, true, true));
  }

  static KValue executeWriteText(const Token& token,
                                 const std::vector<KValue>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.WriteText);
    }

    auto fileName = get_string(token, args.at(0));
    auto value = args.at(1);
    return KValue::createBoolean(
        File::writeToFile(token, fileName, value, false, false));
  }

  static KValue executeWriteBytes(const Token& token,
                                  const std::vector<KValue>& args) {
    if (args.size() != 2) {
      throw BuiltinUnexpectedArgumentError(token, FileIOBuiltIns.WriteBytes);
    }

    auto fileName = get_string(token, args.at(0));
    auto value = args.at(1);

    if (!value.isList()) {
      throw ConversionError(token, "Expected a list of bytes to write.");
    }

    auto elements = value.getList()->elements;
    std::vector<char> bytes;
    bytes.reserve(elements.size());

    for (const auto& item : elements) {
      if (!item.isInteger()) {
        throw ConversionError(token, "Expected a list of bytes to write.");
      }

      bytes.emplace_back(static_cast<char>(item.getInteger()));
    }

    File::writeBytes(token, fileName, bytes);
    return KValue::createBoolean(true);
  }
};

#endif