#ifndef KIWI_PARSING_BUILTINS_H
#define KIWI_PARSING_BUILTINS_H

#include <string>
#include <unordered_set>

struct {
  // File operations
  const std::string AppendFile = "__appendfile__";
  const std::string CopyFile = "__copyfile__";
  const std::string CreateFile = "__createfile__";       // Done
  const std::string DeleteFile = "__deletefile__";       // Done
  const std::string MoveFile = "__movefile__";
  const std::string ReadFile = "__readfile__";
  const std::string RenameFile = "__renamefile__";
  const std::string WriteFile = "__writefile__";
  const std::string FileExists = "__fileexists__";       // Done
  const std::string GetFileExtension = "__fileext__";    // Done
  const std::string FileName = "__filename__";           // Done
  const std::string FileSize = "__filesize__";
  const std::string GetFilePath = "__filepath__";        // Done
  const std::string GetFileAbsolutePath = "__fileabspath__";
  const std::string GetFileAttributes = "__fileattrs__";

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
      AppendFile,    CopyFile,          CreateFile,         DeleteFile,
      MoveFile,      ReadFile,          RenameFile,         WriteFile,
      FileExists,    FileName,          FileSize,           GetFileExtension,
      GetFilePath,   GetFileAttributes, DirectoryExists,    DirectoryName,
      ListDirectory, MakeDirectory,     MakeDirectoryP,     RemoveDirectory,
      IsDirectory,   ChangeDirectory,   GetCurrentDirectory, 
      GetFileAbsolutePath};

  bool is_builtin(const std::string& arg) {
    return builtins.find(arg) != builtins.end();
  }
} FileIOBuiltIns;

#endif
