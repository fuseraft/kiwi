#ifndef KIWI_ERRORS_HANDLER_H
#define KIWI_ERRORS_HANDLER_H

#include <vector>
#include "error.h"
#include "parsing/tokens.h"
#include "system/fileregistry.h"

class ErrorHandler {
 public:
  static int handleError(const AstralError& e) {
    const Token& token = e.getToken();
    std::string message = e.getMessage();

    std::cerr << e.getError() << ": ";
    if (!message.empty()) {
      std::cerr << e.getMessage() << std::endl;
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
      std::cerr << "File: " << file;
    }

    std::cerr << "  @ Line " << 1 + lineNumber << ", Column " << linePosition
              << "." << std::endl;

    if (!line.empty()) {
      std::cerr << "Line: " << line << std::endl;

      for (int i = 0; i < length + 6; ++i) {
        std::cerr << (i == 6 + linePosition - 1 ? "^" : " ");
      }

      std::cerr << std::endl;
    }

    return 1;
  }

  static void printError(const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
  }

  static void handleFatalError(const std::exception& e) {
    printError(e);
    exit(1);
  }
};

#endif