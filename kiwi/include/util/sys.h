#ifndef KIWI_SYSTEM_SYS_H
#define KIWI_SYSTEM_SYS_H

#include <array>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#ifdef _WIN64
#include <WinSock2.h>
#include "Windows.h"
#include <stdio.h>
#include <stdlib.h>
#else
#include <unistd.h>
#endif
#include "typing/value.h"

class Sys {
 public:
  static k_int exec(const k_string& command) {
#ifdef _WIN64
    std::wstring wide_command(command.begin(), command.end());
    return static_cast<k_int>(_wsystem(wide_command.c_str()));
#else
    return static_cast<k_int>(std::system(command.c_str()));
#endif
  }

#ifdef __linux__
  static int closeFile(FILE* file) { return pclose(file); }
#endif

  static k_string execOut(const k_string& command) {
    k_string result;
#ifdef _WIN64
    const int MAX_BUFFER = 128;
    std::array<char, MAX_BUFFER> buffer;
    FILE* stream = _popen(command.c_str(), "r");
    if (!stream)
      return "";
    while (fgets(buffer.data(), buffer.size(), stream) != NULL) {
      result.append(buffer.data());
    }
    _pclose(stream);
#else
    std::array<char, 128> buffer;
    std::unique_ptr<FILE, decltype(&closeFile)> pipe(
        popen(command.c_str(), "r"), closeFile);
    if (!pipe) {
      return "";
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
      result += buffer.data();
    }
#endif
    return result;
  }

  static int getEffectiveUserId() {
#ifdef _WIN64
    return -1;
#else
    return geteuid();
#endif
  }
};

#endif
