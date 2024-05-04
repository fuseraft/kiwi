#ifndef KIWI_INTERPSESSION_H
#define KIWI_INTERPSESSION_H

#include "parsing/keywords.h"
#include "util/file.h"
#include "tracing/error.h"
#include "interp.h"
#include "repl.h"

class Host {
 public:
  Host(Interpreter& interp) : interp(interp), scripts(), args() {}

  void disableLibraryLoad() { kiwilibEnabled = false; }

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
    interp.setKiwiArgs(args);

    // Always try to load kiwilib.
    loadKiwiLibrary();

    // Start REPL if no scripts are supplied.
    if (scripts.empty()) {
      Repl repl(interp);
      return repl.run();
    }

    return loadScripts();
  }

  int parse(const std::string& input) {
    interp.setKiwiArgs(args);

    // Always try to load kiwilib.
    loadKiwiLibrary();

    return interp.interpretKiwi(input);
  }

  std::string minify(const std::string& script, bool output = false) {
    return interp.minify(script, output);
  }

  bool hasScript() const {
    return !scripts.empty();
  }

 private:
  Interpreter& interp;
  std::unordered_set<std::string> scripts;
  std::unordered_map<std::string, std::string> args;
  bool kiwilibEnabled = true;

  void loadLibraryModules(const std::string& path) {
    std::vector<std::string> kiwilib;
#ifdef _WIN64
    kiwilib = File::expandGlob(path + "\\*" + kiwi_extension);
#else
    kiwilib = File::expandGlob(path + "/*" + kiwi_extension);
#endif

    for (const auto& script : kiwilib) {
      loadScript(script);
    }
  }

  void loadKiwiLibrary() {
    if (!kiwilibEnabled) {
      return;
    }

    try {
      auto kiwilibPath = File::getLibraryPath();

      if (!kiwilibPath.empty()) {
        loadLibraryModules(kiwilibPath);
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
    auto parentPath = File::getParentPath(path);
    auto libPath = File::joinPath(parentPath, "lib");

    if (File::directoryExists(libPath)) {
      loadLibraryModules(libPath);
    }

    return interp.interpretScript(path);
  }
};

#endif