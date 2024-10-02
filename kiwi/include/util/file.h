#ifndef KIWI_UTIL_FILE_H
#define KIWI_UTIL_FILE_H

#include <fstream>
#include <filesystem>
#include <regex>
#include <string>
#include "tracing/error.h"
#include "typing/serializer.h"
#include "typing/value.h"
#include "util/glob.h"
#include "util/string.h"
#ifdef _WIN64
#include <WinSock2.h>
#include "Windows.h"
#endif

namespace fs = std::filesystem;

/// @brief A file utility.
class File {
 public:
  // File operations
  static bool createFile(const k_string& filePath);
  static k_string getFileExtension(const k_string& filePath);
  static k_string getFileName(const k_string& filePath);
  static bool fileExists(const k_string& filePath);
  static bool directoryExists(const k_string& path);
  static bool makeDirectory(const k_string& path);
  static bool makeDirectoryP(const k_string& path);
  static bool removePath(const k_string& path);
  static int removePathF(const k_string& path);
  static bool copyFile(const k_string& sourcePath,
                       const k_string& destinationPath, bool overwrite);
  static bool copyR(const k_string& sourcePath,
                    const k_string& destinationPath);
  static std::vector<k_string> listDirectory(const k_string& path);
  static bool movePath(const k_string& sourcePath,
                       const k_string& destinationPath);

  // New Fuction CanRead/CanWrite
  static bool CanRead(const k_string& path);
  static bool CanWrite(const k_string& path);

  // File content manipulation
  static k_int getFileSize(const k_string& filePath);
  static bool writeToFile(const k_string& filePath, const k_value& content,
                          bool appendMode, bool addNewLine);
  static void writeBytes(const std::string& filePath,
                         const std::vector<char>& data);
  static k_string readFile(const k_string& filePath);
  static std::vector<k_string> readLines(const k_string& filePath);
  static std::vector<char> readBytes(const k_string& filePath,
                                     const k_int& offset, const k_int& size);

  // Path manipulation
  static k_string getAbsolutePath(const k_string& path);
  static k_string getCurrentDirectory();
  static bool setCurrentDirectory(const k_string& path);
  static k_string getParentPath(const k_string& path);
  static k_string joinPath(const k_string& directoryPath,
                           const k_string& filePath);

  // File type checks
  static bool isSymLink(const k_string& path);
  static bool isScript(const k_string& path);
  static k_string tryGetExtensionless(const k_string& path);

  // Directory and path utilities
  static k_string getTempDirectory();
  static fs::path getExecutablePath();
  static k_string getLibraryPath();
  static std::vector<k_string> expandGlob(const k_string& globString);
  static k_string getLocalPath(const k_string& path);

#ifdef _WIN64
  static k_string wstring_tos(const std::wstring& wstring);
#endif
};

/// @brief Create a file.
/// @param filePath The file path.
/// @return Boolean indicating success.
bool File::createFile(const k_string& filePath) {
  bool success = false;
  try {
    std::ofstream outputFile(filePath);
    success = outputFile.is_open();
    outputFile.close();
  } catch (const std::exception& e) {
    Thrower<FileSystemError> thrower;
    thrower.throwError(e.what());
  }

  return success;
}

/// @brief Get a file extension.
/// @param filePath The file path.
/// @return String containing a file extension.
k_string File::getFileExtension(const k_string& filePath) {
  try {
    return fs::path(filePath).extension().string();
  } catch (const std::exception& e) {
    Thrower<FileSystemError> thrower;
    thrower.throwError(e.what());
  }

  return "";
}

/// @brief Get a file name.
/// @param filePath The file path.
/// @return String containing a file name.
k_string File::getFileName(const k_string& filePath) {
  try {
    return fs::path(filePath).filename().string();
  } catch (const std::exception& e) {
    Thrower<FileSystemError> thrower;
    thrower.throwError(e.what());
  }

  return "";
}

/// @brief Checks if a file exists.
/// @param filePath The file path.
/// @return Boolean indicating existence.
bool File::fileExists(const k_string& filePath) {
  try {
    return fs::exists(filePath);
  } catch (const std::exception& e) {
    Thrower<FileSystemError> thrower;
    thrower.throwError(e.what());
  }

  return false;
}

/// @brief Checks if a directory exists.
/// @param path The path.
/// @return Boolean indicating existence.
bool File::directoryExists(const k_string& path) {
  try {
    return fs::exists(path) && fs::is_directory(path);
  } catch (const fs::filesystem_error& e) {
    Thrower<FileSystemError> thrower;
    thrower.throwError(e.what());
  }

  return false;
}

/// @brief Create a directory.
/// @param path The path.
/// @return Boolean indicating success.
bool File::makeDirectory(const k_string& path) {
  try {
    return fs::create_directory(path);
  } catch (const fs::filesystem_error& e) {
    Thrower<FileSystemError> thrower;
    thrower.throwError(e.what());
  }

  return false;
}

/// @brief Create a directory containing sub-directories.
/// @param path The path.
/// @return Boolean indicating success.
bool File::makeDirectoryP(const k_string& path) {
  try {
    return fs::create_directories(path);
  } catch (const fs::filesystem_error& e) {
    Thrower<FileSystemError> thrower;
    thrower.throwError(e.what());
  }

  return false;
}

/// @brief Remove a path.
/// @param path The path.
/// @return Boolean indicating success.
bool File::removePath(const k_string& path) {
  try {
    return fs::remove(path);
  } catch (const fs::filesystem_error& e) {
    Thrower<FileSystemError> thrower;
    thrower.throwError(e.what());
  }

  return false;
}

/// @brief Remove a path along with all its content.
/// @param path The path.
/// @return Integer containing count of items removed.
int File::removePathF(const k_string& path) {
  try {
    return static_cast<int>(fs::remove_all(path));
  } catch (const fs::filesystem_error& e) {
    Thrower<FileSystemError> thrower;
    thrower.throwError(e.what());
  }

  return false;
}

k_string File::getTempDirectory() {
  try {
    return fs::temp_directory_path().string();
  } catch (const std::exception& e) {
    Thrower<FileSystemError> thrower;
    thrower.throwError(e.what());
  }

  return "";
}

/// @brief Copy a file.
/// @param sourcePath The source path.
/// @param destinationPath The destination path.
/// @param overwrite A flag to toggle overwriting files.
/// @return Boolean indicating success.
bool File::copyFile(const k_string& sourcePath, const k_string& destinationPath,
                    bool overwrite = true) {
  try {
    auto options = overwrite ? fs::copy_options::overwrite_existing
                             : fs::copy_options::none;
    return fs::copy_file(sourcePath, destinationPath, options);
  } catch (const fs::filesystem_error& e) {
    Thrower<FileSystemError> thrower;
    thrower.throwError(e.what());
  }

  return false;
}

/// @brief Copy a directory and all of its content recursively.
/// @param sourcePath The source path.
/// @param destinationPath The destination path.
/// @return Boolean indicating success.
bool File::copyR(const k_string& sourcePath, const k_string& destinationPath) {
  try {
    fs::copy(sourcePath, destinationPath, fs::copy_options::recursive);
    return true;
  } catch (const fs::filesystem_error& e) {
    Thrower<FileSystemError> thrower;
    thrower.throwError(e.what());
  }

  return false;
}

/// @brief Get a vector of entries within a directory.
/// @param path The path.
/// @return A vector of entries within a directory.
std::vector<k_string> File::listDirectory(const k_string& path) {
  std::vector<k_string> paths;

  try {
    for (const auto& x : fs::directory_iterator(path)) {
#ifdef _WIN64
      const std::wstring entryPath = x.path().c_str();
      paths.emplace_back(wstring_tos(entryPath));
#else
      paths.emplace_back(x.path().string());
#endif
    }
  } catch (const fs::filesystem_error& e) {
    Thrower<FileSystemError> thrower;
    thrower.throwError(e.what());
  }

  return paths;
}

/// @brief Move or rename a path.
/// @param sourcePath The source path.
/// @param destinationPath The destination path.
/// @return Boolean indicating success.
bool File::movePath(const k_string& sourcePath,
                    const k_string& destinationPath) {
  try {
    fs::rename(sourcePath, destinationPath);
    return true;
  } catch (const fs::filesystem_error& e) {
    Thrower<FileSystemError> thrower;
    thrower.throwError(e.what());
  }

  return false;
}

// CanRead/CanWrite Funtion
bool File::CanRead(const k_string& path) {
    if (!fileExists(path)) {
        return false;
    }
}
bool File::CanWrite(const k_string& path) {
    if (!fileExists(path)) {
        return false;
    }
}

/// @brief Get file size in bytes.
/// @param filePath The file path.
/// @return Integer containing number of bytes in a file.
k_int File::getFileSize(const k_string& filePath) {
  try {
    return static_cast<k_int>(fs::file_size(filePath));
  } catch (const fs::filesystem_error& e) {
    Thrower<FileSystemError> thrower;
    thrower.throwError(e.what());
  }

  return 0;
}

/// @brief Writes or appends content to a file.
/// @param filePath The file path.
/// @param value The string content.
/// @param appendMode A flag to toggle append mode.
/// @param addNewLine A flag to toggle appending a newline.
/// @return Boolean indicating success.
bool File::writeToFile(const k_string& filePath, const k_value& content,
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
k_string File::getAbsolutePath(const k_string& path) {
  try {
    fs::path absolutePath = fs::absolute(path);
    return absolutePath.lexically_normal().string();
  } catch (const std::exception& e) {
    Thrower<FileSystemError> thrower;
    thrower.throwError(e.what());
  }

  return "";
}

/// @brief Get current directory path.
/// @return String containing current directory path.
k_string File::getCurrentDirectory() {
  return fs::current_path().string();
}

/// @brief Change the current directory path.
/// @param path The path.
/// @return Boolean indicating success.
bool File::setCurrentDirectory(const k_string& path) {
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
k_string File::getParentPath(const k_string& path) {
  try {
    return fs::path(path).parent_path().string();
  } catch (const std::exception& e) {
    Thrower<FileSystemError> thrower;
    thrower.throwError(e.what());
  }

  return "";
}

/// @brief Check if a path is a kiwi script.
/// @param path The path.
/// @return Boolean indicating success.
bool File::isScript(const k_string& path) {
  k_string extension = ".🥝";
#ifdef _WIN64
  extension = ".kiwi";
#endif
  return (String::endsWith(path, extension) ||
          String::endsWith(path, ".kiwi")) &&
         File::fileExists(path);
}

k_string tryGetExtensionlessSpecifc(const k_string& path,
                                    const k_string& extension) {
  auto scriptPath = path + extension;
  if (File::fileExists(scriptPath)) {
    return scriptPath;
  }
  return "";
}

/// @brief Check if a path is a kiwi script.
/// @param path The path.
/// @return Boolean indicating success.
k_string File::tryGetExtensionless(const k_string& path) {
  std::vector<k_string> extensions;
  extensions.emplace_back(".min.kiwi");
  extensions.emplace_back(".kiwi");
#ifndef _WIN64
  extensions.emplace_back(".min.🥝");
  extensions.emplace_back(".🥝");
#endif

  for (const auto& ext : extensions) {
    auto scriptPath = tryGetExtensionlessSpecifc(path, ext);
    if (!scriptPath.empty()) {
      return scriptPath;
    }
  }

  return "";
}

/// @brief Checks if a path is a symlink.
/// @param path The path.
/// @return Boolean indicating success.
bool File::isSymLink(const k_string& path) {
  std::error_code ec;
  bool result = fs::is_symlink(fs::path(path), ec);

  if (ec) {
    return false;
  }

  return result;
}

/// @brief Get the executable path.
/// @return String containing executable path.
fs::path File::getExecutablePath() {
#ifdef _WIN64
  wchar_t path[FILENAME_MAX] = {0};
  GetModuleFileNameW(nullptr, path, FILENAME_MAX);
  return fs::path(path);
#else
  const k_string executablePath = "/proc/self/exe";

  if (!isSymLink(executablePath)) {
    return "";
  }

  fs::path symLinkPath = fs::read_symlink(executablePath).parent_path();

  if (!fs::exists(symLinkPath)) {
    return "";
  }

  return symLinkPath;
#endif
}

k_string File::getLibraryPath() {
  fs::path kiwiPath(getExecutablePath());
  fs::path kiwilibPath;
#ifdef _WIN64
  k_string binPath = getParentPath(kiwiPath.string());
  k_string parentPath = getParentPath(binPath);
  kiwilibPath = (fs::path(parentPath) / "lib\\kiwi").lexically_normal();
#else
  kiwilibPath = (kiwiPath / "../lib/kiwi").lexically_normal();
#endif

  if (!fs::exists(kiwilibPath)) {
    return "";
  }

  return kiwilibPath.string();
}

#ifdef _WIN64
k_string File::wstring_tos(const std::wstring& wstring) {
  if (wstring.empty()) {
    return "";
  }

  const auto size =
      WideCharToMultiByte(CP_UTF8, 0, &wstring.at(0), (int)wstring.size(),
                          nullptr, 0, nullptr, nullptr);
  if (size <= 0) {
    throw std::runtime_error("WideCharToMultiByte() failed: " +
                             std::to_string(size));
  }

  k_string string(size, 0);
  WideCharToMultiByte(CP_UTF8, 0, &wstring.at(0), (int)wstring.size(),
                      &string.at(0), size, nullptr, nullptr);
  return string;
}
#endif

/// @brief Get a vector of paths matching a glob pattern.
/// @param globString The glob pattern.
/// @return A vector of strings containing paths matched by glob pattern.
std::vector<k_string> File::expandGlob(const k_string& globString) {
  Glob glob = parseGlob(globString);
  k_string basePath = glob.path;
  std::regex filenameRegex(glob.regexPattern, std::regex_constants::ECMAScript |
                                                  std::regex_constants::icase);

  std::vector<k_string> matchedFiles;

  basePath = fs::absolute(basePath).string();

  if (!directoryExists(basePath)) {
    return matchedFiles;
  }

  if (glob.recursiveTraversal) {
    for (const auto& entry : fs::recursive_directory_iterator(basePath)) {
#ifdef _WIN64
      const std::wstring entryPath = entry.path().c_str();
      auto pathString = wstring_tos(entryPath);
      if (std::regex_match(pathString, filenameRegex)) {
        matchedFiles.emplace_back(entry.path().lexically_normal().string());
      }
#else
      if (entry.is_regular_file() &&
          std::regex_match(entry.path().filename().string(), filenameRegex)) {
        matchedFiles.emplace_back(entry.path().lexically_normal().string());
      }
#endif
    }
  } else {
    for (const auto& entry : fs::directory_iterator(basePath)) {
#ifdef _WIN64
      const std::wstring entryPath = entry.path().c_str();
      if (std::regex_match(wstring_tos(entryPath), filenameRegex)) {
        matchedFiles.emplace_back(entry.path().lexically_normal().string());
      }
#else
      if (entry.is_regular_file() &&
          std::regex_match(entry.path().filename().string(), filenameRegex)) {
        matchedFiles.emplace_back(entry.path().lexically_normal().string());
      }
#endif
    }
  }

  return matchedFiles;
}

/// @brief Get local path.
/// @param path The path.
/// @return String containing local path.
k_string File::getLocalPath(const k_string& path) {
  return joinPath(getCurrentDirectory(), path);
}

/// @brief Combine two paths.
/// @param directoryPath The first path.
/// @param filePath The second path.
/// @return String containing combined path.
k_string File::joinPath(const k_string& directoryPath,
                        const k_string& filePath) {
  const auto& fullPath = fs::path(directoryPath) / fs::path(filePath);
  return fullPath.string();
}

/// @brief Read a file into a string.
/// @param filePath The file path.
/// @return String containing file content.
k_string File::readFile(const k_string& filePath) {
  std::ifstream inputFile(filePath, std::ios::binary);

  if (!inputFile.is_open()) {
    Thrower<FileReadError> thrower;
    thrower.throwError(filePath);
  }

  inputFile.seekg(0, std::ios::end);
  size_t size = inputFile.tellg();
  inputFile.seekg(0);

  k_string buffer;
  buffer.resize(size);

  inputFile.read(&buffer[0], size);

  return buffer;
}

/// @brief Read lines from a file into a vector.
/// @param filePath The file path.
/// @return A vector of strings containing file content.
std::vector<k_string> File::readLines(const k_string& filePath) {
  std::ifstream inputFile(filePath);
  if (!inputFile.is_open()) {
    Thrower<FileReadError> thrower;
    thrower.throwError(filePath);
  }

  std::vector<k_string> list;
  k_string line;
  while (getline(inputFile, line)) {
    list.emplace_back(std::move(line));
  }

  return list;
}

/// @brief Read bytes from a file.
/// @param filePath The file path.
/// @param offset The position to read from.
/// @param size The number of bytes to read.
/// @return A vector of bytes containing file content.
std::vector<char> File::readBytes(const k_string& filePath, const k_int& offset,
                                  const k_int& size) {
  std::vector<char> buffer(static_cast<size_t>(size));
  std::ifstream file(filePath, std::ios::binary);

  if (!file) {
    Thrower<FileReadError> thrower;
    thrower.throwError(filePath);
  }

  file.seekg(static_cast<std::streampos>(offset));

  if (!file) {
    Thrower<FileReadError> thrower;
    thrower.throwError(filePath);
  }

  file.read(buffer.data(), static_cast<size_t>(size));

  if (!file && !file.eof()) {
    Thrower<FileReadError> thrower;
    thrower.throwError(filePath);
  }

  return buffer;
}

/// @brief Write bytes to a file.
/// @param filePath The file path.
/// @param data The data to write.
void File::writeBytes(const std::string& filePath,
                      const std::vector<char>& data) {
  std::ofstream file(filePath, std::ios::binary | std::ios::trunc);

  if (!file) {
    Thrower<FileReadError> thrower;
    thrower.throwError(filePath);
  }

  file.write(data.data(), data.size());

  if (!file) {
    Thrower<FileReadError> thrower;
    thrower.throwError(filePath);
  }
}

#endif
