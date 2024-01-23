#ifndef SYS_IO_H
#define SYS_IO_H

#include <fstream>
#include <filesystem>
namespace fs = std::filesystem;

class FileIO {
 public:
  static bool fileExists(const std::string& filePath) {
    return fs::exists(filePath);
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
    std::ifstream input_file(filePath, std::ios::binary);

    if (!input_file.is_open()) {
      throw std::runtime_error("Cannot open file: " + filePath);
    }

    input_file.seekg(0, std::ios::end);
    size_t size = input_file.tellg();
    input_file.seekg(0);

    std::string buffer;
    buffer.resize(size);

    input_file.read(&buffer[0], size);

    return buffer;
  }
};

#endif