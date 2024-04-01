#ifndef ASTRAL_INTERPSESSION_H
#define ASTRAL_INTERPSESSION_H

#include <fstream>
#include <sstream>
#include <stdexcept>
#include "parsing/keywords.h"
#include "util/file.h"
#include "tracing/error.h"
#include "interp.h"
#include "repl.h"

class Host {
 public:
  Host(Interpreter& interp) : interp(interp), scripts(), args() {}

  void disableLibraryLoad() { astrallibEnabled = false; }

  void registerScript(const std::string& scriptPath) {
    if (!File::fileExists(scriptPath)) {
      throw FileNotFoundError(scriptPath);
    }

    std::string absolutePath = File::getAbsolutePath(scriptPath);
    if (!File::fileExists(absolutePath)) {
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
    // Start REPL if no scripts are supplied.
    if (scripts.empty()) {
      Repl repl(interp);
      return repl.run();
    }

    return loadScripts();
  }

  int parse(const std::string& input) {
    interp.setAstralArgs(args);

    // Always try to load astrallib.
    loadAstralLibrary();

    return interp.interpretAstral(input);
  }

 private:
  Interpreter& interp;
  std::unordered_set<std::string> scripts;
  std::unordered_map<std::string, std::string> args;
  bool astrallibEnabled = true;

  void loadAstralLibrary() {
    if (!astrallibEnabled) {
      return;
    }

    try {
      auto astrallibPath = File::getLibraryPath();

      if (!astrallibPath.empty()) {
        std::vector<std::string> astrallib;
#ifdef _WIN64
        astrallib = File::expandGlob(astrallibPath + "\\*.astral");
#else
        astrallib = File::expandGlob(astrallibPath + "/*.🚀");
#endif

        for (const auto& script : astrallib) {
          loadScript(script);
        }
      }
    } catch (const std::exception& e) {
      ErrorHandler::printError(e);
    }
  }

  int loadScripts() {
    int returnCode = 0;

    try {
      for (const auto& script : scripts) {
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
    auto path = File::getAbsolutePath(script);
    return interp.interpretScript(path);
  }
};

#endif