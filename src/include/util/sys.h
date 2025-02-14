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
#include "typing/value.h"

class Sys {
 public:
  static k_int exec(const k_string& command) {
    return static_cast<k_int>(std::system(command.c_str()));
  }

  static int closeFile(FILE* file) { return pclose(file); }

  static k_string execOut(const k_string& command) {
    k_string result;
    std::array<char, 128> buffer;
    std::unique_ptr<FILE, decltype(&closeFile)> pipe(
        popen(command.c_str(), "r"), closeFile);

    if (!pipe) {
      return "";
    }

    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
      result += buffer.data();
    }

    return result;
  }

  static int getEffectiveUserId() { return geteuid(); }

  static void printAt(int x, int y, char ch) {
    std::cout << "\033[" << y + 1 << ";" << x + 1 << "H" << ch << std::flush;
  }
};

#endif
