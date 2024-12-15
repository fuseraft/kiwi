#ifndef KIWI_ERRORS_HANDLER_H
#define KIWI_ERRORS_HANDLER_H

#include <vector>
#include "error.h"
#include "fileregistry.h"
#include "parsing/tokens.h"

class ErrorHandler {
 public:
  static int handleError(const KiwiError& e) {
    printKiwiError(e);

    const Token& token = e.getToken();
    printErrorLocation(token);

    return 1;
  }

  static int handleError(const KiwiError& e, const std::stack<std::string>& stack) {
    printKiwiError(e);

    const Token& token = e.getToken();
    printErrorLocation(token);

    std::stack<std::string> tmpStack(stack);
    std::vector<std::string> funcStack;
    funcStack.reserve(stack.size());
    while (!tmpStack.empty()) {
      funcStack.emplace_back(tmpStack.top());
      tmpStack.pop();
    }
    
    funcStack.erase(funcStack.end());
    for (const auto& func : funcStack) {
      std::cout << "  " << func << std::endl;
    }
      
    return 1;
  }

  static void printKiwiError(const KiwiError& e) {
    std::string message = e.getMessage();

    std::cout << e.getError() << ": ";
    if (!message.empty()) {
      std::cout << e.getMessage() << std::endl;
    }
  }

  static void printError(const std::exception& e) {
    std::cout << "Error: " << e.what() << std::endl;
  }

  static void handleFatalError(const std::exception& e) {
    printError(e);
    exit(1);
  }

  static void printErrorLocation(const Token& token) {
    if (token.getType() == KTokenType::ENDOFFILE) {
      return;
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

    if (!file.empty()) {
      std::cout << "File: " << file;
    }

    std::cout << ":" << 1 + lineNumber << ":" << linePosition << std::endl;

    int length = line.length();
    if (!line.empty()) {
      std::cout << "Line: " << line << std::endl;

      for (int i = 0; i < length + 6; ++i) {
        std::cout << (i == 6 + linePosition - 1 ? "^" : " ");
      }

      std::cout << std::endl;
    }
  }
};

#endif