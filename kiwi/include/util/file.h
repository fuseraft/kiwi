#ifndef KIWI_UTIL_FILE_H
#define KIWI_UTIL_FILE_H

#include <fstream>
#include <filesystem>
#include <regex>
#include <string>
#include "parsing/tokens.h"
#include "tracing/error.h"
#include "typing/serializer.h"
#include "typing/value.h"
#include "util/glob.h"
#include "util/string.h"

namespace fs = std::filesystem;

/// @brief A file utility.
class File {
 public:
  // File operations
  static bool createFile(const Token& token, const k_string& filePath);
  static k_string getFileExtension(const Token& token,
                                   const k_string& filePath);
  static k_string getFileName(const Token& token, const k_string& filePath);
  static bool fileExists(const Token& token, const k_string& filePath);
  static bool directoryExists(const Token& token, const k_string& path);
  static bool makeDirectory(const Token& token, const k_string& path);
  static bool makeDirectoryP(const Token& token, const k_string& path);
  static bool removePath(const Token& token, const k_string& path);
  static int removePathF(const Token& token, const k_string& path);
  static bool copyFile(const Token& token, const k_string& sourcePath,
                       const k_string& destinationPath, bool overwrite);
  static bool copyR(const Token& token, const k_string& sourcePath,
                    const k_string& destinationPath);
  static std::vector<k_string> listDirectory(const Token& token,
                                             const k_string& path);
  static bool movePath(const Token& token, const k_string& sourcePath,
                       const k_string& destinationPath);

  // New Fuction CanRead/CanWrite
  static bool CanRead(const k_string& path);
  static bool CanWrite(const k_string& path);

  // File content manipulation
  static k_int getFileSize(const Token& token, const k_string& filePath);
  static bool writeToFile(const Token& token, const k_string& filePath,
                          const KValue& content, bool appendMode,
                          bool addNewLine);
  static void writeBytes(const Token& token, const k_string& filePath,
                         const std::vector<char>& data);
  static k_string readFile(const Token& token, const k_string& filePath);
  static std::vector<k_string> readLines(const Token& token,
                                         const k_string& filePath);
  static std::vector<char> readBytes(const Token& token,
                                     const k_string& filePath);
  static std::vector<char> readBytes(const Token& token,
                                     const k_string& filePath,
                                     const k_int& offset, const k_int& size);

  // Path manipulation
  static k_string getAbsolutePath(const Token& token, const k_string& path);
  static k_string getCurrentDirectory();
  static bool setCurrentDirectory(const k_string& path);
  static k_string getParentPath(const Token& token, const k_string& path);
  static k_string joinPath(const k_string& directoryPath,
                           const k_string& filePath);

  // File type checks
  static bool isSymLink(const k_string& path);
  static bool isScript(const Token& token, const k_string& path);
  static k_string tryGetExtensionless(const Token& token, const k_string& path);
  static k_string tryGetExtensionlessSpecific(const Token& token,
                                              const k_string& path,
                                              const k_string& extension);

  // Directory and path utilities
  static k_string getTempDirectory(const Token& token);
  static fs::path getExecutablePath();
  static k_string getLibraryPath(const Token& token);
  static std::vector<k_string> expandGlob(const Token& token,
                                          const k_string& globString);
  static k_string getLocalPath(const k_string& path);
};

/// @brief Create a file.
/// @param filePath The file path.
/// @return Boolean indicating success.
bool File::createFile(const Token& token, const k_string& filePath) {
  bool success = false;
  try {
    std::ofstream outputFile(filePath);
    success = outputFile.is_open();
    outputFile.close();
  } catch (const std::exception& e) {
    throw FileSystemError(token, "Could not create file: " + filePath);
  }

  return success;
}

/// @brief Get a file extension.
/// @param filePath The file path.
/// @return String containing a file extension.
k_string File::getFileExtension(const Token& token, const k_string& filePath) {
  try {
    return fs::path(filePath).extension().string();
  } catch (const std::exception& e) {
    throw FileSystemError(token,
                          "Could not get extension for file: " + filePath);
  }

  return "";
}

/// @brief Get a file name.
/// @param filePath The file path.
/// @return String containing a file name.
k_string File::getFileName(const Token& token, const k_string& filePath) {
  try {
    return fs::path(filePath).filename().string();
  } catch (const std::exception& e) {
    throw FileSystemError(token,
                          "Could not get filename from path: " + filePath);
  }

  return "";
}

/// @brief Checks if a file exists.
/// @param filePath The file path.
/// @return Boolean indicating existence.
bool File::fileExists(const Token& token, const k_string& filePath) {
  try {
    return fs::exists(filePath);
  } catch (const std::exception& e) {
    throw FileSystemError(token,
                          "Could not determine if file exists: " + filePath);
  }

  return false;
}

/// @brief Checks if a directory exists.
/// @param path The path.
/// @return Boolean indicating existence.
bool File::directoryExists(const Token& token, const k_string& path) {
  try {
    return fs::exists(path) && fs::is_directory(path);
  } catch (const fs::filesystem_error& e) {
    throw FileSystemError(token,
                          "Could not determine if directory exists: " + path);
  }

  return false;
}

/// @brief Create a directory.
/// @param path The path.
/// @return Boolean indicating success.
bool File::makeDirectory(const Token& token, const k_string& path) {
  try {
    return fs::create_directory(path);
  } catch (const fs::filesystem_error& e) {
    throw FileSystemError(token, "Could not create directory: " + path);
  }

  return false;
}

/// @brief Create a directory containing sub-directories.
/// @param path The path.
/// @return Boolean indicating success.
bool File::makeDirectoryP(const Token& token, const k_string& path) {
  try {
    return fs::create_directories(path);
  } catch (const fs::filesystem_error& e) {
    throw FileSystemError(token, "Could not create directory: " + path);
  }

  return false;
}

/// @brief Remove a path.
/// @param path The path.
/// @return Boolean indicating success.
bool File::removePath(const Token& token, const k_string& path) {
  try {
    return fs::remove(path);
  } catch (const fs::filesystem_error& e) {
    throw FileSystemError(token, "Could not remove path: " + path);
  }

  return false;
}

/// @brief Remove a path along with all its content.
/// @param path The path.
/// @return Integer containing count of items removed.
int File::removePathF(const Token& token, const k_string& path) {
  try {
    return static_cast<int>(fs::remove_all(path));
  } catch (const fs::filesystem_error& e) {
    throw FileSystemError(token, "Could not remove path: " + path);
  }

  return false;
}

k_string File::getTempDirectory(const Token& token) {
  try {
    return fs::temp_directory_path().string();
  } catch (const std::exception& e) {
    throw FileSystemError(token,
                          "Could not create a temporary directory path.");
  }

  return "";
}

/// @brief Copy a file.
/// @param sourcePath The source path.
/// @param destinationPath The destination path.
/// @param overwrite A flag to toggle overwriting files.
/// @return Boolean indicating success.
bool File::copyFile(const Token& token, const k_string& sourcePath,
                    const k_string& destinationPath, bool overwrite = true) {
  try {
    auto options = overwrite ? fs::copy_options::overwrite_existing
                             : fs::copy_options::none;
    return fs::copy_file(sourcePath, destinationPath, options);
  } catch (const fs::filesystem_error& e) {
    throw FileSystemError(token, "Could not copy file: " + sourcePath);
  }

  return false;
}

/// @brief Copy a directory and all of its content recursively.
/// @param sourcePath The source path.
/// @param destinationPath The destination path.
/// @return Boolean indicating success.
bool File::copyR(const Token& token, const k_string& sourcePath,
                 const k_string& destinationPath) {
  try {
    fs::copy(sourcePath, destinationPath, fs::copy_options::recursive);
    return true;
  } catch (const fs::filesystem_error& e) {
    throw FileSystemError(token, "Could not copy file: " + sourcePath);
  }

  return false;
}

/// @brief Get a vector of entries within a directory.
/// @param path The path.
/// @return A vector of entries within a directory.
std::vector<k_string> File::listDirectory(const Token& token,
                                          const k_string& path) {
  std::vector<k_string> paths;

  try {
    for (const auto& x : fs::directory_iterator(path)) {
      paths.emplace_back(x.path().string());
    }
  } catch (const fs::filesystem_error& e) {
    throw FileSystemError(token, "Could not list directory: " + path);
  }

  return paths;
}

/// @brief Move or rename a path.
/// @param sourcePath The source path.
/// @param destinationPath The destination path.
/// @return Boolean indicating success.
bool File::movePath(const Token& token, const k_string& sourcePath,
                    const k_string& destinationPath) {
  try {
    fs::rename(sourcePath, destinationPath);
    return true;
  } catch (const fs::filesystem_error& e) {
    throw FileSystemError(token, "Could not move path: " + sourcePath);
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
k_int File::getFileSize(const Token& token, const k_string& filePath) {
  try {
    return static_cast<k_int>(fs::file_size(filePath));
  } catch (const fs::filesystem_error& e) {
    throw FileSystemError(token, "Could not get file size: " + filePath);
  }

  return 0;
}

/// @brief Writes or appends content to a file.
/// @param filePath The file path.
/// @param value The string content.
/// @param appendMode A flag to toggle append mode.
/// @param addNewLine A flag to toggle appending a newline.
/// @return Boolean indicating success.
bool File::writeToFile(const Token& token, const k_string& filePath,
                       const KValue& content, bool appendMode,
                       bool addNewLine) {
  std::ios_base::openmode mode = appendMode ? std::ios::app : std::ios::out;
  std::ofstream file(filePath, mode);

  if (!file.is_open()) {
    throw FileWriteError(token, filePath);
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
k_string File::getAbsolutePath(const Token& token, const k_string& path) {
  try {
    fs::path absolutePath = fs::absolute(path);
    return absolutePath.lexically_normal().string();
  } catch (const std::exception& e) {
    throw FileSystemError(token, "Could not get absolute path: " + path);
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
k_string File::getParentPath(const Token& token, const k_string& path) {
  try {
    return fs::path(path).parent_path().string();
  } catch (const std::exception& e) {
    throw FileSystemError(token, "Could not get parent path: " + path);
  }

  return "";
}

/// @brief Check if a path is a kiwi script.
/// @param path The path.
/// @return Boolean indicating success.
bool File::isScript(const Token& token, const k_string& path) {
  static const std::unordered_set<k_string> validExtensions = {
      ".min.kiwi", ".kiwi", ".min.🥝", ".🥝", ".min.k", ".k"};

  auto extension = getFileExtension(token, path);
  return validExtensions.find(extension) != validExtensions.end();
}

k_string File::tryGetExtensionlessSpecific(const Token& token,
                                           const k_string& path,
                                           const k_string& extension) {
  auto scriptPath = path + extension;
  if (File::fileExists(token, scriptPath)) {
    return scriptPath;
  }
  return "";
}

/// @brief Check if a path is a kiwi script.
/// @param path The path.
/// @return Boolean indicating success.
k_string File::tryGetExtensionless(const Token& token, const k_string& path) {
  static const std::unordered_set<k_string> extensions = {
      ".min.kiwi", ".kiwi", ".min.🥝", ".🥝", ".min.k", ".k"};

  if (File::isScript(token, path) && File::fileExists(token, path) &&
      !File::directoryExists(token, path)) {
    return path;
  }

  for (const auto& ext : extensions) {
    auto scriptPath = tryGetExtensionlessSpecific(token, path, ext);
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
  const k_string executablePath = "/proc/self/exe";

  if (!isSymLink(executablePath)) {
    return "";
  }

  fs::path symLinkPath = fs::read_symlink(executablePath).parent_path();

  if (!fs::exists(symLinkPath)) {
    return "";
  }

  return symLinkPath;
}

k_string File::getLibraryPath(const Token& token) {
  fs::path kiwiPath(getExecutablePath());
  fs::path kiwilibPath;
  kiwilibPath = (kiwiPath / "../lib/kiwi").lexically_normal();

  if (!fs::exists(kiwilibPath)) {
    return token.getText();
  }

  return kiwilibPath.string();
}

/// @brief Get a vector of paths matching a glob pattern.
/// @param globString The glob pattern.
/// @return A vector of strings containing paths matched by glob pattern.
std::vector<k_string> File::expandGlob(const Token& token,
                                       const k_string& globString) {
  Glob glob = parseGlob(globString);
  k_string basePath = glob.path;
  std::regex filenameRegex(glob.regexPattern, std::regex_constants::ECMAScript |
                                                  std::regex_constants::icase);

  std::vector<k_string> matchedFiles;

  basePath = fs::absolute(basePath).string();

  if (!directoryExists(token, basePath)) {
    return matchedFiles;
  }

  if (glob.recursiveTraversal) {
    for (const auto& entry : fs::recursive_directory_iterator(basePath)) {
      if (entry.is_regular_file() &&
          std::regex_match(entry.path().filename().string(), filenameRegex)) {
        matchedFiles.emplace_back(entry.path().lexically_normal().string());
      }
    }
  } else {
    for (const auto& entry : fs::directory_iterator(basePath)) {
      if (entry.is_regular_file() &&
          std::regex_match(entry.path().filename().string(), filenameRegex)) {
        matchedFiles.emplace_back(entry.path().lexically_normal().string());
      }
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
k_string File::readFile(const Token& token, const k_string& filePath) {
  std::ifstream inputFile(filePath, std::ios::binary);

  if (!inputFile.is_open()) {
    throw FileReadError(token, filePath);
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
std::vector<k_string> File::readLines(const Token& token,
                                      const k_string& filePath) {
  std::ifstream inputFile(filePath);
  if (!inputFile.is_open()) {
    throw FileReadError(token, filePath);
  }

  std::vector<k_string> list;
  k_string line;
  while (getline(inputFile, line)) {
    list.emplace_back(std::move(line));
  }

  return list;
}

std::vector<char> File::readBytes(const Token& token,
                                  const k_string& filePath) {
  // Open the file in binary mode and at the end to get its size
  std::ifstream file(filePath, std::ios::binary | std::ios::ate);

  // Check if the file opened successfully
  if (!file) {
    throw FileReadError(token, filePath);
  }

  // Get the size of the file
  size_t fileSize = static_cast<size_t>(file.tellg());

  // Resize the buffer to the size of the file
  std::vector<char> buffer(fileSize);
  file.rdbuf()->pubsetbuf(nullptr, 64 * 1024);

  // Seek back to the beginning of the file and read the content
  file.seekg(0, std::ios::beg);
  if (!file.read(buffer.data(), fileSize)) {
    throw FileReadError(token, filePath);
  }

  return buffer;
}

/// @brief Read bytes from a file.
/// @param filePath The file path.
/// @param offset The position to read from.
/// @param size The number of bytes to read.
/// @return A vector of bytes containing file content.
std::vector<char> File::readBytes(const Token& token, const k_string& filePath,
                                  const k_int& offset, const k_int& size) {
  std::vector<char> buffer(static_cast<size_t>(size));
  std::ifstream file(filePath, std::ios::binary);

  if (!file) {
    throw FileReadError(token, filePath);
  }

  file.seekg(static_cast<std::streampos>(offset));

  if (!file) {
    throw FileReadError(token, filePath);
  }

  file.read(buffer.data(), static_cast<size_t>(size));

  if (!file && !file.eof()) {
    throw FileReadError(token, filePath);
  }

  return buffer;
}

/// @brief Write bytes to a file.
/// @param filePath The file path.
/// @param data The data to write.
void File::writeBytes(const Token& token, const k_string& filePath,
                      const std::vector<char>& data) {
  std::ofstream file(filePath, std::ios::binary | std::ios::trunc);

  if (!file) {
    throw FileReadError(token, filePath);
  }

  file.write(data.data(), data.size());

  if (!file) {
    throw FileReadError(token, filePath);
  }
}

#endif
