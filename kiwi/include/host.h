#ifndef KIWI_INTERPSESSION_H
#define KIWI_INTERPSESSION_H

#include "util/file.h"
#include "tracing/error.h"
#include "engine.h"
#include "repl.h"

const Token hostToken = Token::createExternal();

class Host {
 public:
  Host(Engine& engine) : engine(engine), scripts(), args(), executionPath("") {}

  void disableLibraryLoad() { kiwilibEnabled = false; }

  void registerScript(const std::string& script) {
    auto scriptPath = File::tryGetExtensionless(hostToken, script);
    if (!File::fileExists(hostToken, scriptPath)) {
      throw FileNotFoundError(scriptPath);
    }

    const auto& absolutePath = File::getAbsolutePath(hostToken, scriptPath);
    if (!File::fileExists(hostToken, absolutePath)) {
      throw FileNotFoundError(absolutePath);
    }

    if (scripts.find(absolutePath) != scripts.end()) {
      return;
    }

    if (!scripts.empty()) {
      throw KiwiError::create(
          hostToken, "The Kiwi runtime only supports single-script execution.");
    }

    executionPath = File::getParentPath(hostToken, absolutePath);

    scripts.insert(absolutePath);
  }

  void registerParseRequest(const std::string& code) {
    parseRequests.push_back(code);
  }

  void registerArg(const std::string& name, const std::string& value) {
    args[name] = value;
  }

  bool hasWork() { return !scripts.empty() || !parseRequests.empty(); }

  int start() {
    engine.setProgramArgs(args);

    // Always try to load kiwilib.
    loadKiwiLibrary();

    // If nothing was supplied, just start the REPL.
    if (!hasWork()) {
      Repl repl(engine);
      return repl.run();
    }

    int returnCode = 0;

    if (!scripts.empty()) {
      returnCode = loadScripts();
    }

    if (returnCode == 0 && !parseRequests.empty()) {
      returnCode = loadParseRequests();
    }

    return returnCode;
  }

  std::string minify(const std::string& script, bool output = false) {
    return Lexer::minify(script, output);
  }

  void printAST(const std::string& script) { engine.printAST(script); }

  bool hasScript() const { return !scripts.empty(); }

 private:
  Engine& engine;
  std::unordered_set<std::string> scripts;
  std::vector<std::string> parseRequests;
  std::unordered_map<std::string, std::string> args;
  std::string executionPath;
  bool kiwilibEnabled = true;

  void loadLibraryPackages(const std::string& path) {
    std::vector<std::string> kiwilib;
    kiwilib = File::expandGlob(hostToken, path + "/*" + kiwi_extension);
    auto extras = File::expandGlob(hostToken, path + "/*.kiwi");
    kiwilib.insert(kiwilib.end(), extras.begin(), extras.end());

    for (const auto& script : kiwilib) {
      loadScript(script);
    }
  }

  void loadKiwiLibrary() {
    if (!kiwilibEnabled) {
      return;
    }

    try {
      auto kiwilibPath = File::getLibraryPath(hostToken);

      if (!kiwilibPath.empty()) {
        loadLibraryPackages(kiwilibPath);
      }
    } catch (const std::exception& e) {
      ErrorHandler::printError(e);
    }
  }

  int loadParseRequests() {
    int returnCode = 0;

    try {
      for (const auto& code : parseRequests) {
        returnCode = engine.interpretKiwi(code);

        // If one parse fails, we need to stop here.
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

    const auto& cwd = File::getCurrentDirectory();
    if (!executionPath.empty() &&
        File::directoryExists(hostToken, executionPath)) {
      File::setCurrentDirectory(executionPath);
    }

    returnCode = engine.runStreamCollection();
    File::setCurrentDirectory(cwd);

    return returnCode;
  }

  int loadScript(const std::string& script) {
    auto path = File::getAbsolutePath(hostToken, script);
    auto parentPath = File::getParentPath(hostToken, path);
    auto libPath = File::joinPath(parentPath, "lib");

    if (File::directoryExists(hostToken, libPath)) {
      loadLibraryPackages(libPath);
    }

    return engine.parseScript(path);
  }
};

#endif