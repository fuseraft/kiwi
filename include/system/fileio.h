#ifndef KIWI_SYSTEM_FILEIO_H
#define KIWI_SYSTEM_FILEIO_H

#include <fstream>
#include <filesystem>
#include <string>
#include <variant>

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
    return fs::exists(filePath);
  }

  static bool deleteFile(const std::string& filePath) {
    try {
      if (!fileExists(filePath)) {
        return false;
      }

      return fs::remove(filePath);
    } catch (const fs::filesystem_error&) {
      return false;
    }
  }

  static bool copyFile(const std::string& sourcePath,
                       const std::string& destinationPath,
                       bool overwrite = true) {
    try {
      fs::copy_options options = overwrite
                                     ? fs::copy_options::overwrite_existing
                                     : fs::copy_options::none;
      fs::copy_file(sourcePath, destinationPath, options);
      return true;
    } catch (const fs::filesystem_error& e) {
      throw;
    } catch (const std::exception& e) {
      throw;
    }
    return false;
  }

  static bool moveFile(const std::string& sourcePath,
                       const std::string& destinationPath) {
    try {
      fs::rename(sourcePath, destinationPath);
      return true;
    } catch (const fs::filesystem_error& e) {
      throw;
    } catch (const std::exception& e) {
      throw;
    }
    return false;
  }

  static double getFileSize(const std::string& filePath) {
    try {
      if (!fileExists(filePath)) {
        throw std::runtime_error("File does not exist.");
      }
      auto size = fs::file_size(filePath);
      return static_cast<double>(size);
    } catch (const fs::filesystem_error& e) {
      throw;
    } catch (const std::exception& e) {
      throw;
    }
  }

  static bool writeToFile(const std::string& filePath,
                          std::variant<int, double, bool, std::string>& value,
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
    return absolutePath.string();
  }

  static std::string getParentPath(const std::string& filePath) {
    fs::path path(filePath);
    fs::path parentPath = path.parent_path();
    return parentPath.string();
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
};

#endif