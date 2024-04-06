#ifndef ASTRAL_H
#define ASTRAL_H

#include <regex>
#include <vector>

#include "tracing/error.h"
#include "tracing/handler.h"
#include "logging/logger.h"
#include "math/rng.h"
#include "parsing/keywords.h"
#include "parsing/tokens.h"
#include "util/file.h"
#include "util/string.h"
#include "web/httplib.h"
#include "globals.h"
#include "host.h"
#include "stackframe.h"

TaskManager task;

std::unordered_map<std::string, Method> methods;
std::unordered_map<std::string, Module> modules;
std::unordered_map<std::string, Class> classes;
std::unordered_map<std::string, std::string> astralArgs;
std::stack<std::shared_ptr<CallStackFrame>> callStack;
std::stack<k_stream> streamStack;
std::stack<std::string> moduleStack;
std::unordered_map<int, Method> astralWebServerHooks;
httplib::Server astralWebServer;
std::string astralWebServerHost;
k_int astralWebServerPort;

class Astral {
 public:
  static int run(int argc, char** argv);

 private:
  static bool createNewFile(const std::string& path);
  static bool createMinified(Host& host, const std::string& path);
  static bool processOption(std::string& opt, Host& host);
  static bool parse(Host& host, const std::string& content);
  static bool tokenize(Host& host, const std::string& path);

  static int printVersion();
  static int printHelp();
  static int run(std::vector<std::string>& v);
};

int Astral::run(int argc, char** argv) {
  std::vector<std::string> args;

  for (int i = 0; i < argc; ++i) {
    args.emplace_back(argv[i]);
  }

  return Astral::run(args);
}

int Astral::run(std::vector<std::string>& v) {
  RNG::getInstance();

  Interpreter interp;
  Host host(interp);

  //v.push_back("/home/scs/astral/play.🚀");

  size_t size = v.size();

  try {
    host.registerArg(astral_arg, v.at(0));

    bool help = false;

    for (size_t i = 1; i < size; ++i) {
      if (String::isCLIFlag(v.at(i), "h", "help")) {
        help = true;
      } else if (String::isCLIFlag(v.at(i), "v", "version")) {
        return Astral::printVersion();
      } else if (String::isCLIFlag(v.at(i), "n", "new")) {
        if (i + 1 < size) {
          return Astral::createNewFile(v.at(++i));
        }

        help = true;
      } else if (String::isCLIFlag(v.at(i), "m", "minify")) {
        if (i + 1 < size) {
          return Astral::createMinified(host, v.at(++i));
        }

        help = true;
      } else if (String::isCLIFlag(v.at(i), "p", "parse")) {
        if (i + 1 < size) {
          return Astral::parse(host, v.at(++i));
        }

        help = true;
      } else if (String::isCLIFlag(v.at(i), "t", "tokenize")) {
        if (i + 1 < size) {
          return Astral::tokenize(host, v.at(++i));
        }

        help = true;
      } else if (File::isScript(v.at(i))) {
        host.registerScript(v.at(i));
      } else if (String::isOptionKVP(v.at(i))) {
        help = !Astral::processOption(v.at(i), host);
      } else {
        auto extensionless = File::tryGetExtensionless(v.at(i));
        if (!extensionless.empty()) {
          host.registerScript(extensionless);
        } else {
          host.registerArg("argv_" + RNG::getInstance().random16(), v.at(i));
        }
      }
    }

    if (help) {
      return Astral::printHelp();
    }

    return host.start();
  } catch (const AstralError& e) {
    return ErrorHandler::handleError(e);
  }
}

bool Astral::parse(Host& host, const std::string& content) {
  return host.parse(content);
}

bool Astral::createMinified(Host& host, const std::string& path) {
  const std::string DefaultExtension = ".min.🚀";

  auto filePath = path;
  auto minFileExtension = DefaultExtension;
#ifdef _WIN64
  minFileExtension = ".min.astral";
#endif

  if (File::getFileExtension(filePath).empty()) {
#ifdef _WIN64
    filePath += ".astral";
#else
    filePath += ".🚀";
#endif
  }

  if (!File::fileExists(filePath)) {
    std::cout << "The input file does not exists." << std::endl;
    return false;
  }

  filePath = File::getAbsolutePath(filePath);
  auto fileName =
      String::replace(File::getFileName(filePath),
                      File::getFileExtension(filePath), minFileExtension);
  auto minFilePath = File::joinPath(File::getParentPath(filePath), fileName);

  std::cout << "Creating " << minFilePath << std::endl;
  if (File::createFile(minFilePath)) {
    auto minified = host.minify(filePath);
    File::writeToFile(minFilePath, minified, false, false);
    return true;
  }

  return false;
}

bool Astral::tokenize(Host& host, const std::string& path) {
  if (!File::fileExists(path)) {
    std::cout << "The input file does not exists." << std::endl;
    return false;
  }

  auto filePath = File::getAbsolutePath(path);
  auto minified = host.minify(filePath, true);
  return true;
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

  filePath = File::getAbsolutePath(filePath);

  std::cout << "Creating " << filePath << std::endl;
  return File::createFile(filePath);
}

bool Astral::processOption(std::string& opt, Host& host) {
  std::regex xargPattern("-(.*?)=");
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
      {"-p, --parse <astral_code>", "parse astral code as an argument"},
      {"-n, --new <file_path>", "create a `.🚀` file"},
      {"-m, --minify <input_file_path>", "create a `.min.🚀` file"},
      {"-t, --tokenize <input_file_path>",
       "tokenize a file with the astral lexer"},
      {"-X<key>=<value>", "specify an argument as a key-value pair"}};

#ifdef _WIN64
  commands = {
      {"-h, --help", "print this message"},
      {"-v, --version", "print the current version"},
      {"-p, --parse <astral_code>", "parse code"},
      {"-n, --new <filename>", "create a `.astral` file"},
      {"-m, --minify <input_file_path>", "create a `.min.astral` file"},
      {"-t, --tokenize <input_file_path>", "tokenize a file as astral code"},
      {"-X<key>=<value>", "specify an argument as a key-value pair"}};
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