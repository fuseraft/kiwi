#ifndef KIWI_UTIL_FILE_H
#define KIWI_UTIL_FILE_H

#include <fstream>
#include <filesystem>
#include <regex>
#include <string>
#include "errors/error.h"
#include "typing/serializer.h"
#include "typing/valuetype.h"
#include "util/glob.h"
#include "util/string.h"

namespace fs = std::filesystem;

/// @brief A file utility.
class File {
 public:
  static bool createFile(const std::string& filePath);
  static std::string getFileExtension(const std::string& filePath);
  static std::string getFileName(const std::string& filePath);
  static bool fileExists(const std::string& filePath);
  static bool directoryExists(const std::string& path);
  static bool makeDirectory(const std::string& path);
  static bool makeDirectoryP(const std::string& path);
  static bool removePath(const std::string& path);
  static int removePathF(const std::string& path);
  static std::string getTempDirectory();
  static bool copyFile(const std::string& sourcePath,
                       const std::string& destinationPath, bool overwrite);
  static bool copyR(const std::string& sourcePath,
                    const std::string& destinationPath);
  static std::vector<std::string> listDirectory(const std::string& path);
  static bool movePath(const std::string& sourcePath,
                       const std::string& destinationPath);
  static k_int getFileSize(const std::string& filePath);
  static bool writeToFile(const std::string& filePath, const Value& content,
                          bool appendMode, bool addNewLine);
  static std::string getAbsolutePath(const std::string& path);
  static std::string getCurrentDirectory();
  static bool setCurrentDirectory(const std::string& path);
  static std::string getParentPath(const std::string& path);
  static bool isSymLink(const std::string& path);
  static bool isScript(const std::string& path);
  static std::string getExecutablePath();
  static std::string getLibraryPath();
  static std::vector<std::string> expandGlob(const std::string& globString);
  static std::string getLocalPath(const std::string& path);
  static std::string joinPath(const std::string& directoryPath,
                              const std::string& filePath);
  static std::string readFile(const std::string& filePath);
  static std::vector<std::string> readLines(const std::string& filePath);
};

/// @brief Create a file.
/// @param filePath The file path.
/// @return Boolean indicating success.
bool File::createFile(const std::string& filePath) {
  std::ofstream outputFile(filePath);
  bool isSuccess = outputFile.is_open();
  outputFile.close();
  return isSuccess;
}

/// @brief Get a file extension.
/// @param filePath The file path.
/// @return String containing a file extension.
std::string File::getFileExtension(const std::string& filePath) {
  fs::path path(filePath);
  return path.extension().string();
}

/// @brief Get a file name.
/// @param filePath The file path.
/// @return String containing a file name.
std::string File::getFileName(const std::string& filePath) {
  fs::path path(filePath);
  return path.filename().string();
}

/// @brief Checks if a file exists.
/// @param filePath The file path.
/// @return Boolean indicating existence.
bool File::fileExists(const std::string& filePath) {
  try {
    return fs::exists(filePath);
  } catch (const std::exception&) {}
  return false;
}

/// @brief Checks if a directory exists.
/// @param path The path.
/// @return Boolean indicating existence.
bool File::directoryExists(const std::string& path) {
  try {
    return fs::exists(path) && fs::is_directory(path);
  } catch (const fs::filesystem_error&) {}
  return false;
}

/// @brief Create a directory.
/// @param path The path.
/// @return Boolean indicating success.
bool File::makeDirectory(const std::string& path) {
  try {
    return fs::create_directory(path);
  } catch (const fs::filesystem_error&) {}
  return false;
}

/// @brief Create a directory containing sub-directories.
/// @param path The path.
/// @return Boolean indicating success.
bool File::makeDirectoryP(const std::string& path) {
  try {
    return fs::create_directories(path);
  } catch (const fs::filesystem_error&) {}
  return false;
}

/// @brief Remove a path.
/// @param path The path.
/// @return Boolean indicating success.
bool File::removePath(const std::string& path) {
  try {
    return fs::remove(path);
  } catch (const fs::filesystem_error&) {}
  return false;
}

/// @brief Remove a path along with all its content.
/// @param path The path.
/// @return Integer containing count of items removed.
int File::removePathF(const std::string& path) {
  try {
    return static_cast<int>(fs::remove_all(path));
  } catch (const fs::filesystem_error&) {}
  return false;
}

std::string File::getTempDirectory() {
  return fs::temp_directory_path().string();
}

/// @brief Copy a file.
/// @param sourcePath The source path.
/// @param destinationPath The destination path.
/// @param overwrite A flag to toggle overwriting files.
/// @return Boolean indicating success.
bool File::copyFile(const std::string& sourcePath,
                    const std::string& destinationPath, bool overwrite = true) {
  auto options =
      overwrite ? fs::copy_options::overwrite_existing : fs::copy_options::none;

  try {
    return fs::copy_file(sourcePath, destinationPath, options);
  } catch (const fs::filesystem_error&) {}
  return false;
}

/// @brief Copy a directory and all of its content recursively.
/// @param sourcePath The source path.
/// @param destinationPath The destination path.
/// @return Boolean indicating success.
bool File::copyR(const std::string& sourcePath,
                 const std::string& destinationPath) {
  try {
    fs::copy(sourcePath, destinationPath, fs::copy_options::recursive);
    return true;
  } catch (const fs::filesystem_error&) {}
  return false;
}

/// @brief Get a vector of entries within a directory.
/// @param path The path.
/// @return A vector of entries within a directory.
std::vector<std::string> File::listDirectory(const std::string& path) {
  std::vector<std::string> paths;

  try {
    for (const auto& x : std::filesystem::directory_iterator(path)) {
      paths.push_back(x.path().string());
    }
  } catch (const fs::filesystem_error&) {}

  return paths;
}

/// @brief Move or rename a path.
/// @param sourcePath The source path.
/// @param destinationPath The destination path.
/// @return Boolean indicating success.
bool File::movePath(const std::string& sourcePath,
                    const std::string& destinationPath) {
  try {
    fs::rename(sourcePath, destinationPath);
    return true;
  } catch (const fs::filesystem_error&) {}
  return false;
}

/// @brief Get file size in bytes.
/// @param filePath The file path.
/// @return Integer containing number of bytes in a file.
k_int File::getFileSize(const std::string& filePath) {
  try {
    if (!fileExists(filePath)) {
      Thrower<FileNotFoundError> thrower;
      thrower.throwError(filePath);
    }

    return static_cast<k_int>(fs::file_size(filePath));
  } catch (const fs::filesystem_error& e) {
    Thrower<FileSystemError> thrower;
    thrower.throwError(e.what());
    return -1;
  }
}

/// @brief Writes or appends content to a file.
/// @param filePath The file path.
/// @param value The string content.
/// @param appendMode A flag to toggle append mode.
/// @param addNewLine A flag to toggle appending a newline.
/// @return Boolean indicating success.
bool File::writeToFile(const std::string& filePath, const Value& content,
                       bool appendMode, bool addNewLine) {
  std::ios_base::openmode mode = appendMode ? std::ios::app : std::ios::out;
  std::ofstream file(filePath, mode);

  if (!file.is_open()) {
    Thrower<FileWriteError> thrower;
    thrower.throwError(filePath);
  }

  file << Serializer::serialize(content);
  if (addNewLine) {
    file << std::endl;
  }

  file.close();
  return true;
}

/// @brief Get absolute path of a relative path.
/// @param path The path.
/// @return String containing absolute path..
std::string File::getAbsolutePath(const std::string& path) {
  fs::path absolutePath = fs::absolute(path);
  return absolutePath.lexically_normal().string();
}

/// @brief Get current directory path.
/// @return String containing current directory path.
std::string File::getCurrentDirectory() {
  return fs::current_path().string();
}

/// @brief Change the current directory path.
/// @param path The path.
/// @return Boolean indicating success.
bool File::setCurrentDirectory(const std::string& path) {
  std::error_code ec;
  fs::current_path(path, ec);

  if (ec) {
    return false;
  }

  return true;
}

/// @brief Get the parent directory of a path.
/// @param path
/// @return
std::string File::getParentPath(const std::string& path) {
  fs::path childPath(path);
  return childPath.parent_path().string();
}

/// @brief Check if a path is a kiwi script.
/// @param path The path.
/// @return Boolean indicating success.
bool File::isScript(const std::string& path) {
  return (String::endsWith(path, "🥝") || String::endsWith(path, ".kiwi")) &&
         File::fileExists(path);
}

/// @brief Checks if a path is a symlink.
/// @param path The path.
/// @return Boolean indicating success.
bool File::isSymLink(const std::string& path) {
  std::error_code ec;
  bool result = fs::is_symlink(fs::path(path), ec);

  if (ec) {
    return false;
  }

  return result;
}

/// @brief Get the executable path.
/// @return String containing executable path.
std::string File::getExecutablePath() {
  const std::string executablePath = "/proc/self/exe";

  if (!isSymLink(executablePath)) {
    return "";
  }

  fs::path symLinkPath = fs::read_symlink(executablePath).parent_path();

  if (!fs::exists(symLinkPath)) {
    return "";
  }

  return symLinkPath.string();
}

std::string File::getLibraryPath() {
  fs::path kiwiPath(getExecutablePath());
  fs::path kiwilibPath = (kiwiPath / "../lib/kiwi").lexically_normal();

  if (!fs::exists(kiwilibPath)) {
    return "";
  }

  return kiwilibPath.string();
}

/// @brief Get a vector of paths matching a glob pattern.
/// @param globString The glob pattern.
/// @return A vector of strings containing paths matched by glob pattern.
std::vector<std::string> File::expandGlob(const std::string& globString) {
  Glob glob = parseGlob(globString);
  std::string basePath = glob.path;
  std::regex filenameRegex(glob.regexPattern, std::regex_constants::ECMAScript |
                                                  std::regex_constants::icase);

  std::vector<std::string> matchedFiles;

  basePath = fs::absolute(basePath);

  if (!directoryExists(basePath)) {
    return matchedFiles;
  }

  if (glob.recursiveTraversal) {
    for (const auto& entry : fs::recursive_directory_iterator(basePath)) {
      if (entry.is_regular_file() &&
          std::regex_match(entry.path().filename().string(), filenameRegex)) {
        matchedFiles.push_back(entry.path().lexically_normal().string());
      }
    }
  } else {
    for (const auto& entry : fs::directory_iterator(basePath)) {
      if (entry.is_regular_file() &&
          std::regex_match(entry.path().filename().string(), filenameRegex)) {
        matchedFiles.push_back(entry.path().lexically_normal().string());
      }
    }
  }

  return matchedFiles;
}

/// @brief Get local path.
/// @param path The path.
/// @return String containing local path.
std::string File::getLocalPath(const std::string& path) {
  return joinPath(getCurrentDirectory(), path);
}

/// @brief Combine two paths.
/// @param directoryPath The first path.
/// @param filePath The second path.
/// @return String containing combined path.
std::string File::joinPath(const std::string& directoryPath,
                           const std::string& filePath) {
  fs::path dir(directoryPath);
  fs::path file(filePath);
  fs::path fullPath = dir / file;
  return fullPath.string();
}

/// @brief Read a file into a string.
/// @param filePath The file path.
/// @return String containing file content.
std::string File::readFile(const std::string& filePath) {
  std::ifstream inputFile(filePath, std::ios::binary);

  if (!inputFile.is_open()) {
    Thrower<FileReadError> thrower;
    thrower.throwError(filePath);
  }

  inputFile.seekg(0, std::ios::end);
  size_t size = inputFile.tellg();
  inputFile.seekg(0);

  std::string buffer;
  buffer.resize(size);

  inputFile.read(&buffer[0], size);

  return buffer;
}

/// @brief Read lines from a file into a vector.
/// @param filePath The file path.
/// @return A vector of strings containing file content.
std::vector<std::string> File::readLines(const std::string& filePath) {
  std::ifstream inputFile(filePath);
  if (!inputFile.is_open()) {
    Thrower<FileReadError> thrower;
    thrower.throwError(filePath);
  }

  std::vector<std::string> list;
  std::string line;
  while (getline(inputFile, line)) {
    list.push_back(line);
  }

  return list;
}

#endif