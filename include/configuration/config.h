#ifndef ASTRAL_CONFIGURATION_CONFIG_H
#define ASTRAL_CONFIGURATION_CONFIG_H

#include <fstream>
#include <iostream>
#include <string>
#include <unordered_map>

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

  std::string get(const std::string& key,
                  const std::string& defaultValue = "") const {
    auto it = config_.find(key);
    if (it != config_.end()) {
      return it->second;
    }

    return defaultValue;
  }

 private:
  std::unordered_map<std::string, std::string> config_;
};

#endif