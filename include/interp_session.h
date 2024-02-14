#ifndef KIWI_INTERPSESSION_H
#define KIWI_INTERPSESSION_H

#include <fstream>
#include <sstream>
#include <stdexcept>
#include "errors/error.h"
#include "logging/logger.h"
#include "parsing/keywords.h"
#include "system/fileio.h"
#include "globals.h"
#include "interp.h"

class InterpSession {
 public:
  InterpSession(Logger& logger, Interpreter& interp)
      : logger(logger), interp(interp), scripts(), args() {}

  void registerScript(const std::string& scriptPath) {
    if (!FileIO::fileExists(scriptPath)) {
      throw FileNotFoundError(scriptPath);
    }

    std::string absolutePath = FileIO::getAbsolutePath(scriptPath);
    if (!FileIO::fileExists(absolutePath)) {
      throw FileNotFoundError(absolutePath);
    }

    if (scripts.find(absolutePath) != scripts.end()) {
      return;
    }

    scripts.insert(absolutePath);
  }

  void registerArg(const std::string& name, const std::string& value) {
    args[name] = value;
  }

  int start() {
    interp.setKiwiArgs(args);

    // Always try to load kiwilib.
    loadKiwiLibrary();

    // Start REPL if no scripts are supplied.
    if (scripts.empty()) {
      return loadRepl();
    }

    return loadScripts();
  }

 private:
  Logger& logger;
  Interpreter& interp;
  std::unordered_set<std::string> scripts;
  std::map<std::string, std::string> args;
  bool replMode;

  void loadKiwiLibrary() {
    try {
      std::string kiwilibPath = FileIO::getKiwiLibraryPath();

      if (!kiwilibPath.empty()) {
        auto kiwilib = FileIO::expandGlob(kiwilibPath + "/**/*.kiwi");
        for (const auto& script : kiwilib) {
          std::cout << "Kiwilib Loading: " << script << std::endl;
          loadScript(script);
        }
      }
    } catch (const std::exception& e) {
      ErrorHandler::printError(e);
    }
  }

  int loadRepl() {
    const std::string repl = "repl";
    std::vector<std::string> lines;
    std::string input;

    std::cout << kiwi_name << " v" << kiwi_version << " REPL" << std::endl
              << std::endl;
    std::cout << "Use `go` to execute, `exit` to end the REPL session."
              << std::endl
              << std::endl;

    interp.preserveMainStackFrame();

    while (true) {
      try {
        std::cout << "kiwi> ";
        std::getline(std::cin, input);

        if (input == Keywords.Exit) {
          break;
        }

        if (input == Keywords.Go) {
          std::string kiwiCode;
          for (size_t i = 0; i < lines.size(); ++i) {
            kiwiCode += lines[i] + "\n";
          }

          if (!kiwiCode.empty()) {
            Lexer lexer(repl, kiwiCode);
            interp.interpret(lexer);
            lines.clear();
          }
          continue;
        }

        lines.push_back(input);

      } catch (const std::exception& e) {
        ErrorHandler::printError(e);
        return 1;
      }
    }

    return 0;
  }

  int loadScripts() {
    int returnCode = 0;

    try {
      for (const std::string& script : scripts) {
        returnCode = loadScript(script);

        // If one script fails, we need to stop here.
        if (returnCode != 0) {
          return returnCode;
        }
      }
    } catch (const std::exception& e) {
      ErrorHandler::printError(e);
      return 1;
    }

    return returnCode;
  }

  int loadScript(const std::string& script) {
    std::string content = FileIO::readFile(script);
    if (content.empty()) {
      return -1;
    }

    std::string parentPath = FileIO::getParentPath(script);
    std::string absolutePath = FileIO::getAbsolutePath(script);

    Lexer lexer(absolutePath, content);
    return interp.interpret(lexer, parentPath);
  }
};

#endif