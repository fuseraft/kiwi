#ifndef KIWI_SYSTEM_SYS_H
#define KIWI_SYSTEM_SYS_H

#include <array>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <unistd.h>

class Sys {
 public:
  static long long exec(const std::string& command) {
    return static_cast<long long>(std::system(command.c_str()));
  }

  static std::string execOut(const std::string& command) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"),
                                                  pclose);
    if (!pipe) {
      return "";
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
      result += buffer.data();
    }
    return result;
  }

  static int getEffectiveUserId() { return geteuid(); }
};

#endif