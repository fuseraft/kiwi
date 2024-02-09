#ifndef KIWI_ERRORS_HANDLER_H
#define KIWI_ERRORS_HANDLER_H

#include <vector>
#include <cstdlib>
#include <cxxabi.h>
#include <execinfo.h>
#include "error.h"
#include "parsing/tokens.h"

class ErrorHandler {
 public:
  static int handleError(const KiwiError& e) {
    const Token& token = e.getToken();
    std::string message = e.getMessage();

    std::cerr << "Exception: " << e.getError() << std::endl;
    if (!message.empty()) {
      std::cerr << "Message:   " << e.getMessage() << std::endl;
    }

    return 1;
  }

  static int handleError(const KiwiError& e, std::vector<std::string> lines) {
    handleError(e);

    const Token& token = e.getToken();
    int lineNumber = token.getLineNumber();
    int linePosition = token.getLinePosition();

    std::string line = lines.at(lineNumber);
    int length = line.length();

    std::cerr << "File: " << token.getFile() << std::endl;
    std::cerr << "Location:  Line " << 1 + lineNumber << ", Column "
              << linePosition << "." << std::endl;
    std::cerr << "Code: " << std::endl;
    std::cerr << "```" << std::endl << line << std::endl;

    for (int i = 0; i < length; ++i) {
      std::cerr << (i == linePosition - 1 ? "^" : " ");
    }

    std::cerr << std::endl << "```" << std::endl << std::endl;

    return 1;
  }

  static void printError(const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    // printStackTrace();
  }

  static void handleFatalError(const std::exception& e) {
    printError(e);
    exit(1);
  }

 private:
  static void printStackTrace() {
    void* array[5];
    size_t size;

    size = backtrace(array, 5);

    char** messages = backtrace_symbols(array, size);

    std::cout << "Stacktrace:" << std::endl;
    for (size_t i = 0; i < size; i++) {
      std::cout << i << ": " << messages[i] << std::endl;

      // Demangle C++ function names
      size_t sz = 256;  // just a guess, could be too small, too large?
      char* function = static_cast<char*>(malloc(sz));
      int status = 0;
      char* demangled =
          abi::__cxa_demangle(messages[i], function, &sz, &status);
      if (status == 0) {
        std::cout << "    " << demangled << std::endl;
      }

      free(function);
      free(demangled);
    }

    free(messages);
  }
};

#endif