#ifndef KIWI_H
#define KIWI_H

#include <regex>
#include <vector>

#include "configuration/config.h"
#include "tracing/error.h"
#include "tracing/handler.h"
#include "logging/logger.h"
#include "math/rng.h"
#include "parsing/keywords.h"
#include "util/file.h"
#include "util/string.h"
#include "web/httplib.h"
#include "globals.h"
#include "host.h"

Logger logger;
TaskManager task;

std::unordered_map<std::string, Method> methods;
std::unordered_map<std::string, Module> modules;
std::unordered_map<std::string, Class> classes;
std::unordered_map<std::string, std::string> astralArgs;
std::stack<std::shared_ptr<CallStackFrame>> callStack;
std::stack<std::shared_ptr<TokenStream>> streamStack;
std::stack<std::string> moduleStack;
std::unordered_map<int, Method> astralWebServerHooks;
httplib::Server astralWebServer;
std::string astralWebServerHost;
k_int astralWebServerPort;

std::mutex methodsMutex;
std::mutex modulesMutex;
std::mutex classesMutex;
std::mutex astralArgsMutex;
std::mutex callStackMutex;
std::mutex streamStackMutex;
std::mutex moduleStackMutex;
std::mutex astralWebServerMutex;
std::mutex astralWebServerHooksMutex;
std::mutex astralWebServerHostMutex;
std::mutex astralWebServerPortMutex;

class Astral {
 public:
  static int run(int argc, char** argv);

 private:
  static bool configure(Config& config, Logger& logger, Host& host,
                        const std::string& path);
  static bool createNewFile(const std::string& path);
  static bool processXarg(std::string& opt, Host& host);

  static int run(std::vector<std::string>& v);
  static int printVersion();
  static int printHelp();
};

int Astral::run(int argc, char** argv) {
  std::vector<std::string> args;

  for (int i = 0; i < argc; ++i) {
    args.push_back(argv[i]);
  }

  return Astral::run(args);
}

int Astral::run(std::vector<std::string>& v) {
  RNG::getInstance();

  Config config;
  Interpreter interp;
  Host host(interp);

  if (DEBUG) {
    v.push_back("-C");
    v.push_back("/home/scs/astral/config/astral.conf");
  }

  size_t size = v.size();

  try {
    host.registerArg(astral_arg, v.at(0));

    bool help = false;

    for (size_t i = 1; i < size; ++i) {
      if (String::isCLIFlag(v.at(i), "h", "help")) {
        help = true;
      } else if (String::isCLIFlag(v.at(i), "v", "version")) {
        return printVersion();
      } else if (String::isCLIFlag(v.at(i), "C", "config")) {
        if (i + 1 < size) {
          help = !configure(config, logger, host, v.at(++i));
        } else {
          help = true;
        }
      } else if (String::isCLIFlag(v.at(i), "n", "new")) {
        if (i + 1 < size) {
          return createNewFile(v.at(++i));
        }

        help = true;
      } else if (File::isScript(v.at(i))) {
        host.registerScript(v.at(i));
      } else if (String::isXArg(v.at(i))) {
        help = !processXarg(v.at(i), host);
      } else {
        host.registerArg("argv_" + RNG::getInstance().random16(), v.at(i));
      }
    }

    if (help) {
      return printHelp();
    }

    return host.start();
  } catch (const AstralError& e) {
    return ErrorHandler::handleError(e);
  }
}

bool Astral::createNewFile(const std::string& path) {
  const std::string DefaultExtension = ".🚀";
  auto filePath = path;

  if (File::getFileExtension(path).empty()) {
#ifdef _WIN64
    filePath += ".astral";
#else
    filePath += DefaultExtension;
#endif
  }

  if (File::fileExists(filePath)) {
    std::cout << "The file already exists." << std::endl;
    return false;
  }

  std::cout << "Creating " << filePath << std::endl;
  return File::createFile(filePath);
}

bool Astral::configure(Config& config, Logger& logger, Host& host,
                     const std::string& path) {
  if (!String::endsWith(path, ".conf")) {
    std::cout << "I can be configured with a `.conf` file." << std::endl;
    return false;
  } else if (!config.read(path)) {
    std::cout << "I cannot read `" << path << "`." << std::endl;
    return false;
  }

  std::string logPath = config.get("LOGGER_PATH");
  std::string logMode = config.get("LOGGER_MODE");
  std::string logLevel = config.get("LOGGER_LEVEL");
  std::string scriptPath = config.get("SCRIPT_PATH");
  std::string astrallibEnabled = config.get("KIWILIB_ENABLED", "true");

  if (!logPath.empty()) {
    logger.setLogFilePath(logPath);
  }

  if (!logMode.empty()) {
    logger.setLogMode(Logger::logmode_from_string(logMode));
  }

  if (!logLevel.empty()) {
    logger.setMinimumLogLevel(Logger::loglevel_from_string(logLevel));
  }

  if (!scriptPath.empty()) {
    host.registerScript(scriptPath);
  }

  if (!astrallibEnabled.empty() && astrallibEnabled == Keywords.False) {
    host.disableLibraryLoad();
  }

  return true;
}

bool Astral::processXarg(std::string& opt, Host& host) {
  std::regex xargPattern("-X(.*?)=");
  std::string name, value;
  std::smatch match;

  if (std::regex_search(opt, match, xargPattern)) {
    name = match[1].str();
  }

  size_t pos = opt.find('=');
  if (pos != std::string::npos) {
    value = opt.substr(pos + 1);
  }

  if (!name.empty() && !value.empty()) {
    host.registerArg(name, value);
    return true;
  }

  return false;
}

int Astral::printVersion() {
  std::cout << astral_name << " v" << astral_version << std::endl << std::endl;
  return 0;
}

int Astral::printHelp() {
  struct CommandInfo {
    std::string command;
    std::string description;
  };

  std::vector<CommandInfo> commands = {
      {"-h, --help", "print this message"},
      {"-v, --version", "print the current version"},
      {"-n, --new <filename>", "create a `.🚀` file"},
      {"-C, --config <conf_path>", "configure with a `.conf` file"},
      {"-X<key>:<value>", "specify an argument as a key-value pair"}};

#ifdef _WIN64
  commands = {{"-h, --help", "print this message"},
              {"-v, --version", "print the current version"},
              {"-n, --new <filename>", "create a `.astral` file"},
              {"-C, --config <conf_path>", "configure with a `.conf` file"},
              {"-X<key>:<value>", "specify an argument as a key-value pair"}};
#endif

  printVersion();

  std::cout << "Usage: astral [--flags] <script|args>" << std::endl
            << "Options:" << std::endl;

  for (const auto& cmd : commands) {
    std::cout << "  " << std::left << std::setw(40) << cmd.command
              << cmd.description << std::endl;
  }

  std::cout << std::endl;
  return 0;
}

#endif