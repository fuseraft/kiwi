#ifndef KIWI_INTERPSESSION_H
#define KIWI_INTERPSESSION_H

#include <fstream>
#include <sstream>
#include <stdexcept>
#include "errors/error.h"
#include "logging/logger.h"
#include "parsing/keywords.h"
#include "system/fileio.h"
#include "interp.h"

class InterpSession {
 public:
  InterpSession(Logger& logger, Interpreter& interp)
      : logger(logger), interp(interp), scripts(), args() {
    setReplMode(false);
  }

  void registerScript(const std::string& scriptPath) {
    if (!FileIO::fileExists(scriptPath)) {
      throw FileNotFoundError(scriptPath);
    }

    std::string absolutePath = FileIO::getAbsolutePath(scriptPath);

    if (scripts.find(absolutePath) != scripts.end()) {
      return;
    }

    scripts.insert(absolutePath);
  }

  void registerArg(const std::string& name, const std::string& value) {
    args[name] = value;
  }

  void setReplMode(bool value) { replMode = value; }
  bool getReplMode() { return replMode; }

  int start() {
    int ret = loadScripts();

    if (replMode) {
      return loadRepl();
    }

    return ret;
  }

 private:
  Logger& logger;
  Interpreter& interp;
  std::unordered_set<std::string> scripts;
  std::map<std::string, std::string> args;
  bool replMode;

  int loadRepl() {
    const std::string repl = "repl";
    std::string input;

    while (true) {
      try {
        std::cout << "> ";
        std::getline(std::cin, input);

        if (input == Keywords.Exit) {
          break;
        }

        Lexer lexer(logger, repl, input);
        interp.interpret(lexer);
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
        std::string content = FileIO::readFile(script);
        if (content.empty()) {
          continue;
        }

        std::string parentPath = FileIO::getParentPath(script);
        std::string absolutePath = FileIO::getAbsolutePath(script);

        Lexer lexer(logger, absolutePath, content);
        returnCode = interp.interpret(lexer, parentPath);

        // If one script fails, we need to stop here.
        if (returnCode != 0) {
          break;
        }
      }
    } catch (const std::exception& e) {
      ErrorHandler::printError(e);
      return 1;
    }

    return returnCode;
  }
};

#endif