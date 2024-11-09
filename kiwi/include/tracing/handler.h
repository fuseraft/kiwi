#ifndef KIWI_ERRORS_HANDLER_H
#define KIWI_ERRORS_HANDLER_H

#include <vector>
#include "error.h"
#include "fileregistry.h"
#include "parsing/tokens.h"

class ErrorHandler {
 public:
  static int handleError(const KiwiError& e) {
    const Token& token = e.getToken();
    std::string message = e.getMessage();

    std::cout << e.getError() << ": ";
    if (!message.empty()) {
      std::cout << e.getMessage() << std::endl;
    }

    int lineNumber = token.getLineNumber();
    int linePosition = token.getLinePosition();
    std::string line;
    auto file = FileRegistry::getInstance().getFilePath(token.getFile());
    auto lines = FileRegistry::getInstance().getFileLines(token.getFile());
    if (!file.empty() && !lines.empty()) {
      if (lineNumber < static_cast<int>(lines.size())) {
        line = lines.at(lineNumber);
      }
    }

    int length = line.length();

    if (!file.empty()) {
      std::cout << "File: " << file;
    }

    std::cout << ":" << 1 + lineNumber << ":" << linePosition << std::endl;

    if (!line.empty()) {
      std::cout << "Line: " << line << std::endl;

      for (int i = 0; i < length + 6; ++i) {
        std::cout << (i == 6 + linePosition - 1 ? "^" : " ");
      }

      std::cout << std::endl;
    }

    return 1;
  }

  static void printError(const std::exception& e) {
    std::cout << "Error: " << e.what() << std::endl;
  }

  static void handleFatalError(const std::exception& e) {
    printError(e);
    exit(1);
  }
};

#endif