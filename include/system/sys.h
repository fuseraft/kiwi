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

class Sys {
 public:
  static k_int exec(const std::string& command) {
#ifdef _WIN64
    return static_cast<k_int>(
        _wsystem(std::wstring(command.begin(), command.end()).c_str()));
#else
    return static_cast<k_int>(std::system(command.c_str()));
#endif
  }

  static std::string execOut(const std::string& command) {
    std::string result;
#ifdef _WIN64
    const int MAX_BUFFER = 128;
    std::string data;
    FILE* stream;
    char buffer[MAX_BUFFER];

    stream = _popen(command.c_str(), "r");
    while (fgets(buffer, MAX_BUFFER, stream) != NULL)
      data.append(buffer);
    _pclose(stream);
#else
    std::array<char, 128> buffer;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command.c_str(), "r"),
                                                  pclose);
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
