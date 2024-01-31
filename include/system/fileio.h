#ifndef KIWI_SYSTEM_FILEIO_H
#define KIWI_SYSTEM_FILEIO_H

#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

class FileIO {
 public:
  static bool createFile(const std::string& filePath) {
    std::ofstream outputFile(filePath);
    bool isSuccess = outputFile.is_open();
    outputFile.close();
    return isSuccess;
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