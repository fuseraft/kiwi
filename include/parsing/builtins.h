#ifndef KIWI_PARSING_KEYWORDS_H
#define KIWI_PARSING_KEYWORDS_H

#include <string>
#include <unordered_set>

struct {
  // File operations
  const std::string AppendFile = "__appendfile__";
  const std::string CopyFile = "__copyfile__";
  const std::string CreateFile = "__createfile__";
  const std::string MoveFile = "__movefile__";
  const std::string ReadFile = "__readfile__";
  const std::string WriteFile = "__writefile__";
  const std::string FileExists = "__fileexists__";
  const std::string FileName = "__filename__";
  const std::string FileSize = "__filesize__";

  // Directory operations
  const std::string DirectoryExists = "__direxists__";
  const std::string DirectoryName = "__dirname__";
  const std::string MakeDirectory = "__mkdir__";
  const std::string MakeDirectoryP = "__mkdirp__";

  std::unordered_set<std::string> builtins = {
      AppendFile, CopyFile, CreateFile, DirectoryExists, DirectoryName,
      FileExists, FileName, FileSize,   MakeDirectory,   MakeDirectoryP,
      MoveFile,   ReadFile, WriteFile};

  bool is_builtin(const std::string& arg) {
    return builtins.find(arg) != builtins.end();
  }
} FileIOBuiltIns;

#endif