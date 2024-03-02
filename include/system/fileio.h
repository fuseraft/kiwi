#ifndef KIWI_SYSTEM_FILEIO_H
#define KIWI_SYSTEM_FILEIO_H

#include <fstream>
#include <filesystem>
#include <regex>
#include <string>
#include "typing/valuetype.h"
#include "glob.h"

namespace fs = std::filesystem;

class FileIO {
 public:
  static bool createFile(const std::string& filePath) {
    std::ofstream outputFile(filePath);
    bool isSuccess = outputFile.is_open();
    outputFile.close();
    return isSuccess;
  }

  static std::string getFileExtension(const std::string& filePath) {
    fs::path path(filePath);
    return path.extension().string();
  }

  static std::string getFileName(const std::string& filePath) {
    fs::path path(filePath);
    return path.filename().string();
  }

  static bool fileExists(const std::string& filePath) {
    try {
      return fs::exists(filePath);
    } catch (const std::exception&) {}
    return false;
  }

  static bool directoryExists(const std::string& path) {
    try {
      return fs::exists(path) && fs::is_directory(path);
    } catch (const fs::filesystem_error&) {}
    return false;
  }

  static bool makeDirectory(const std::string& path) {
    try {
      return fs::create_directory(path);
    } catch (const fs::filesystem_error&) {}
    return false;
  }

  static bool makeDirectoryP(const std::string& path) {
    try {
      return fs::create_directories(path);
    } catch (const fs::filesystem_error&) {}
    return false;
  }

  static bool removePath(const std::string& path) {
    try {
      return fs::remove(path);
    } catch (const fs::filesystem_error&) {}
    return false;
  }

  static int removePathF(const std::string& path) {
    try {
      return static_cast<int>(fs::remove_all(path));
    } catch (const fs::filesystem_error&) {}
    return false;
  }

  static std::string getTempDirectory() {
    return fs::temp_directory_path().string();
  }

  static bool copyFile(const std::string& sourcePath,
                       const std::string& destinationPath,
                       bool overwrite = true) {
    auto options = overwrite ? fs::copy_options::overwrite_existing
                             : fs::copy_options::none;

    try {
      return fs::copy_file(sourcePath, destinationPath, options);
    } catch (const fs::filesystem_error&) {}
    return false;
  }

  static bool copyR(const std::string& sourcePath,
                    const std::string& destinationPath) {
    try {
      fs::copy(sourcePath, destinationPath, fs::copy_options::recursive);
      return true;
    } catch (const fs::filesystem_error&) {}
    return false;
  }

  static std::vector<std::string> listDirectory(const std::string& path) {
    std::vector<std::string> paths;

    try {
      for (const auto& x : std::filesystem::directory_iterator(path)) {
        paths.push_back(x.path().string());
      }
    } catch (const fs::filesystem_error&) {}

    return paths;
  }

  static bool moveFile(const std::string& sourcePath,
                       const std::string& destinationPath) {
    try {
      fs::rename(sourcePath, destinationPath);
      return true;
    } catch (const fs::filesystem_error&) {}
    return false;
  }

  static double getFileSize(const std::string& filePath) {
    try {
      if (!fileExists(filePath)) {
        throw std::runtime_error("File does not exist.");
      }

      return static_cast<double>(fs::file_size(filePath));
    } catch (const fs::filesystem_error&) {
      throw;
    }
  }

  static bool writeToFile(const std::string& filePath, Value& value,
                          bool appendMode, bool addNewLine) {
    std::ios_base::openmode mode = appendMode ? std::ios::app : std::ios::out;
    std::ofstream file(filePath, mode);

    if (!file.is_open()) {
      throw std::runtime_error("Unable to open file for writing.");
    }

    std::visit(
        [&file, &addNewLine](auto&& arg) {
          file << arg;
          if (addNewLine) {
            file << std::endl;
          }
        },
        value);

    file.close();
    return true;
  }

  static std::string getAbsolutePath(const std::string& filePath) {
    fs::path absolutePath = fs::absolute(filePath);
    return absolutePath.lexically_normal().string();
  }

  static std::string getCurrentDirectory() {
    return fs::current_path().string();
  }

  static bool setCurrentDirectory(const std::string& path) {
    std::error_code ec;
    fs::current_path(path, ec);

    if (ec) {
      return false;
    }

    return true;
  }

  static std::string getParentPath(const std::string& filePath) {
    fs::path path(filePath);
    fs::path parentPath = path.parent_path();

    return parentPath.string();
  }

  static bool isSymLink(const std::string& symLinkPath) {
    std::error_code ec;
    bool result = fs::is_symlink(fs::path(symLinkPath), ec);

    if (ec) {
      return false;
    }

    return result;
  }

  static std::string getKiwiPath() {
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

  static std::string getKiwiLibraryPath() {
    fs::path kiwiPath(getKiwiPath());
    fs::path kiwilibPath = (kiwiPath / "../lib/kiwilib").lexically_normal();

    if (!fs::exists(kiwilibPath)) {
      return "";
    }

    return kiwilibPath.string();
  }

  static std::vector<std::string> expandGlob(const std::string& globString) {
    Glob glob = parseGlob(globString);
    std::string basePath = glob.path;
    std::regex filenameRegex(
        glob.regexPattern,
        std::regex_constants::ECMAScript | std::regex_constants::icase);

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

  static std::string getLocalPath(const std::string& path) {
    return joinPath(getCurrentDirectory(), path);
  }

  static std::string joinPath(const std::string& directoryPath,
                              const std::string& filePath) {
    fs::path dir(directoryPath);
    fs::path file(filePath);
    fs::path fullPath = dir / file;
    return fullPath.string();
  }

  static std::string readFile(const std::string& filePath) {
    std::ifstream inputFile(filePath, std::ios::binary);

    if (!inputFile.is_open()) {
      throw std::runtime_error("Cannot open file: " + filePath);
    }

    inputFile.seekg(0, std::ios::end);
    size_t size = inputFile.tellg();
    inputFile.seekg(0);

    std::string buffer;
    buffer.resize(size);

    inputFile.read(&buffer[0], size);

    return buffer;
  }

  static std::shared_ptr<List> readLines(const std::string& filePath) {
    std::ifstream inputFile(filePath);
    if (!inputFile.is_open()) {
      throw std::runtime_error("Cannot open file: " + filePath);
    }

    std::shared_ptr<List> list = std::make_shared<List>();
    std::string line;
    while (getline(inputFile, line)) {
      list->elements.push_back(Value(line));
    }

    return list;
  }
};

#endif