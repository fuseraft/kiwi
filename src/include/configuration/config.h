#ifndef CONFIG_H
#define CONFIG_H

#include <fstream>
#include <iostream>
#include <map>
#include <string>

class Config {
 public:
  Config() {}

  bool read(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cerr << "Failed to open the configuration file: " << filename
                << std::endl;
      return false;
    }

    std::string line;
    while (std::getline(file, line)) {
      size_t pos = line.find('=');
      if (pos != std::string::npos) {
        std::string key = line.substr(0, pos);
        std::string value = line.substr(pos + 1);
        config_[key] = value;
      }
    }

    file.close();
    return true;
  }

  std::string get(const std::string& key) const {
    auto it = config_.find(key);
    if (it != config_.end()) {
      return it->second;
    }

    // Throw an error instead?
    return "";
  }

 private:
  std::map<std::string, std::string> config_;
};

#endif