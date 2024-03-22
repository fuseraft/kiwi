#ifndef ASTRAL_SYSTEM_FILEREGISTRY_H
#define ASTRAL_SYSTEM_FILEREGISTRY_H

#include <fstream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>

class FileRegistry {
 public:
  static FileRegistry& getInstance() {
    static FileRegistry instance;
    return instance;
  }

  FileRegistry(const FileRegistry&) = delete;
  FileRegistry& operator=(const FileRegistry&) = delete;

  int registerFile(const std::string& filePath) {
    std::istringstream stream(filePath);
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(stream, line)) {
      lines.push_back(line);
    }
    int id = nextId++;
    registry[id] = filePath;
    linesRegistry[id] = lines;
    return id;
  }

  std::string getFilePath(int id) const {
    auto it = registry.find(id);
    if (it != registry.end()) {
      return it->second;
    }
    return "";
  }

  std::vector<std::string> getFileLines(int id) const {
    auto it = linesRegistry.find(id);
    if (it != linesRegistry.end()) {
      return it->second;
    }
    return {};
  }

 private:
  FileRegistry() : nextId(0) {}

  std::unordered_map<int, std::string> registry;
  std::unordered_map<int, std::vector<std::string>> linesRegistry;
  int nextId;
};

#endif