#ifndef KIWI_INTERPSESSION_H
#define KIWI_INTERPSESSION_H

#include "parsing/keywords.h"
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
      throw KiwiError::create(hostToken, "The Kiwi runtime only supports single-script execution.");
    }

    executionPath = File::getParentPath(hostToken, absolutePath);

    scripts.insert(absolutePath);
  }

  void registerArg(const std::string& name, const std::string& value) {
    args[name] = value;
  }

  int start() {
    engine.setKiwiArgs(args);

    // Always try to load kiwilib.
    loadKiwiLibrary();

    // Start REPL if no scripts are supplied.
    if (scripts.empty()) {
      Repl repl(engine);
      return repl.run();
    }

    return loadScripts();
  }

  int parse(const std::string& input) {
    engine.setKiwiArgs(args);

    // Always try to load kiwilib.
    loadKiwiLibrary();

    return engine.interpretKiwi(input);
  }

  std::string minify(const std::string& script, bool output = false) {
    return Lexer::minify(script, output);
  }

  void printAST(const std::string& script) { engine.printAST(script); }

  bool hasScript() const { return !scripts.empty(); }

 private:
  Engine& engine;
  std::unordered_set<std::string> scripts;
  std::unordered_map<std::string, std::string> args;
  std::string executionPath;
  bool kiwilibEnabled = true;

  void loadLibraryPackages(const std::string& path) {
    std::vector<std::string> kiwilib;
#ifdef _WIN64
    kiwilib = File::expandGlob(hostToken, path + "\\*" + kiwi_extension);
#else
    kiwilib = File::expandGlob(hostToken, path + "/*" + kiwi_extension);
    auto extras = File::expandGlob(hostToken, path + "/*.kiwi");
    kiwilib.insert(kiwilib.end(), extras.begin(), extras.end());
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
        loadLibraryPackages(kiwilibPath);
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

    const auto& cwd = File::getCurrentDirectory();
    if (!executionPath.empty() && File::directoryExists(hostToken, executionPath)) {
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

    return engine.interpretScript(path);
  }
};

#endif