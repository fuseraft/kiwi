#ifndef KIWI_PARSING_BUILTINS_H
#define KIWI_PARSING_BUILTINS_H

#include <string>
#include <unordered_set>

struct {
  // File operations
  const std::string AppendText = "__appendtext__";
  const std::string CopyFile = "__copyfile__";
  const std::string CreateFile = "__createfile__";
  const std::string DeleteFile = "__deletefile__";
  const std::string MoveFile = "__movefile__";
  const std::string ReadFile = "__readfile__";
  const std::string ReadLines = "__readlines__";
  const std::string WriteLine = "__writeline__";
  const std::string WriteText = "__writetext__";
  const std::string FileExists = "__fileexists__";
  const std::string GetFileExtension = "__fileext__";
  const std::string FileName = "__filename__";
  const std::string FileSize = "__filesize__";
  const std::string GetFilePath = "__filepath__";
  const std::string GetFileAbsolutePath = "__fileabspath__";
  const std::string GetFileAttributes = "__fileattrs__";  // TODO

  // Directory operations
  const std::string DirectoryExists = "__direxists__";
  const std::string DirectoryName = "__dirname__";
  const std::string ListDirectory = "__listdir__";
  const std::string MakeDirectory = "__mkdir__";
  const std::string MakeDirectoryP = "__mkdirp__";
  const std::string RemoveDirectory = "__rmdir__";
  const std::string IsDirectory = "__isdir__";
  const std::string ChangeDirectory = "__chdir__";
  const std::string GetCurrentDirectory = "__getcwd__";

  std::unordered_set<std::string> builtins = {
      AppendText,         CopyFile,      CreateFile,        DeleteFile,
      MoveFile,           ReadFile,      ReadLines,         WriteText,
      WriteLine,          FileExists,    FileName,          FileSize,
      GetFileExtension,   GetFilePath,   GetFileAttributes, DirectoryExists,
      DirectoryName,      ListDirectory, MakeDirectory,     MakeDirectoryP,
      RemoveDirectory,    IsDirectory,   ChangeDirectory,   GetCurrentDirectory,
      GetFileAbsolutePath};

  bool is_builtin(const std::string& arg) {
    return builtins.find(arg) != builtins.end();
  }
} FileIOBuiltIns;

#endif
