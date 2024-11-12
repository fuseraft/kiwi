#ifndef KIWI_CLI_H
#define KIWI_CLI_H

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
#include "typing/value.h"
#include "globals.h"
#include "host.h"
#include "stackframe.h"

//TaskManager task;

std::unordered_map<k_string, k_string> kiwiArgs;
bool SAFEMODE = false;
const Token cliToken = Token::createExternal();

class KiwiCLI {
 public:
  static int run(int argc, char** argv);

 private:
  static bool createNewFile(const k_string& path);
  static bool createMinified(Host& host, const k_string& path);
  static bool processOption(k_string& opt, Host& host);
  static bool parse(Host& host, const k_string& content);
  static bool tokenize(Host& host, const k_string& path);
  static bool printAST(Host& host, const k_string& path);

  static int printVersion();
  static int printHelp();
  static int run(std::vector<k_string>& v);
};

int KiwiCLI::run(int argc, char** argv) {
  std::vector<k_string> args;

  for (int i = 0; i < argc; ++i) {
    args.emplace_back(argv[i]);
  }

  return KiwiCLI::run(args);
}

int KiwiCLI::run(std::vector<k_string>& v) {
  RNG::getInstance();

  Engine engine;
  Host host(engine);

  size_t size = v.size();

  try {
    host.registerArg(kiwi_arg, v.at(0));

    bool help = false;

    for (size_t i = 1; i < size; ++i) {
      if (String::isCLIFlag(v.at(i), "h", "help")) {
        help = true;
      } else if (String::isCLIFlag(v.at(i), "v", "version")) {
        return KiwiCLI::printVersion();
      } else if (String::isCLIFlag(v.at(i), "n", "new")) {
        if (i + 1 < size) {
          return KiwiCLI::createNewFile(v.at(++i));
        }

        help = true;
      } else if (String::isCLIFlag(v.at(i), "m", "minify")) {
        if (i + 1 < size) {
          return KiwiCLI::createMinified(host, v.at(++i));
        }

        help = true;
      } else if (String::isCLIFlag(v.at(i), "p", "parse")) {
        if (i + 1 < size) {
          return KiwiCLI::parse(host, v.at(++i));
        }

        help = true;
      } else if (String::isCLIFlag(v.at(i), "s", "safemode")) {
        SAFEMODE = true;
      } else if (String::isCLIFlag(v.at(i), "a", "ast")) {
        if (i + 1 < size) {
          return KiwiCLI::printAST(host, v.at(++i));
        }

        help = true;
      } else if (String::isCLIFlag(v.at(i), "t", "tokenize")) {
        if (i + 1 < size) {
          return KiwiCLI::tokenize(host, v.at(++i));
        }

        help = true;
      } else if (File::isScript(cliToken, v.at(i))) {
        host.registerScript(v.at(i));
      } else if (String::isOptionKVP(v.at(i))) {
        help = !KiwiCLI::processOption(v.at(i), host);
      } else {
        auto extless = host.hasScript()
                           ? k_string("")
                           : File::tryGetExtensionless(cliToken, v.at(i));
        if (!extless.empty()) {
          host.registerScript(extless);
        } else {
          host.registerArg("argv_" + RNG::getInstance().random16(), v.at(i));
        }
      }
    }

    if (help) {
      return KiwiCLI::printHelp();
    }

    return host.start();
  } catch (const KiwiError& e) {
    return ErrorHandler::handleError(e);
  }
}

bool KiwiCLI::parse(Host& host, const k_string& content) {
  return host.parse(content);
}

bool KiwiCLI::createMinified(Host& host, const k_string& path) {
  auto filePath = path;

  if (File::getFileExtension(cliToken, filePath).empty()) {
    filePath += kiwi_extension;
  }

  if (!File::fileExists(cliToken, filePath)) {
    std::cout << "The input file does not exists." << std::endl;
    return false;
  }

  filePath = File::getAbsolutePath(cliToken, filePath);
  auto fileName = String::replace(File::getFileName(cliToken, filePath),
                                  File::getFileExtension(cliToken, filePath),
                                  kiwi_min_extension);
  auto minFilePath =
      File::joinPath(File::getParentPath(cliToken, filePath), fileName);

  std::cout << "Creating " << minFilePath << std::endl;
  if (File::createFile(cliToken, minFilePath)) {
    auto minified = host.minify(filePath);
    File::writeToFile(cliToken, minFilePath, minified, false, false);
    return true;
  }

  return false;
}

bool KiwiCLI::createNewFile(const k_string& path) {
  auto filePath = path;

  if (File::getFileExtension(cliToken, path).empty()) {
    filePath += kiwi_extension;
  }

  if (File::fileExists(cliToken, filePath)) {
    std::cout << "The file already exists: " + filePath << std::endl;
    return false;
  }

  filePath = File::getAbsolutePath(cliToken, filePath);
  auto parentPath = File::getParentPath(cliToken, filePath);

  File::makeDirectoryP(cliToken, parentPath);

  std::cout << "Creating " << filePath << std::endl;
  return File::createFile(cliToken, filePath);
}

bool KiwiCLI::printAST(Host& host, const k_string& path) {
  if (!File::fileExists(cliToken, path)) {
    std::cout << "The input file does not exists." << std::endl;
    return false;
  }

  auto filePath = File::getAbsolutePath(cliToken, path);
  host.printAST(filePath);
  return true;
}

bool KiwiCLI::tokenize(Host& host, const k_string& path) {
  if (!File::fileExists(cliToken, path)) {
    std::cout << "The input file does not exists." << std::endl;
    return false;
  }

  auto filePath = File::getAbsolutePath(cliToken, path);
  auto minified = host.minify(filePath, true);
  return true;
}

bool KiwiCLI::processOption(k_string& opt, Host& host) {
  std::regex xargPattern("-(.*?)=");
  k_string name, value;
  std::smatch match;

  if (std::regex_search(opt, match, xargPattern)) {
    name = match[1].str();
  }

  size_t pos = opt.find('=');
  if (pos != k_string::npos) {
    value = opt.substr(pos + 1);
  }

  if (!name.empty() && !value.empty()) {
    host.registerArg(name, value);
    return true;
  }

  return false;
}

int KiwiCLI::printVersion() {
  std::cout << kiwi_arg << " " << kiwi_version << std::endl;
  return 0;
}

int KiwiCLI::printHelp() {
  struct CommandInfo {
    k_string command;
    k_string description;
  };

  std::vector<CommandInfo> commands = {
      {"-h, --help", "print this message"},
      {"-v, --version", "print the current version"},
      {"-n, --new <file_path>", "create a `.🥝` file"},
      {"-p, --parse <kiwi_code>", "parse kiwi code as an argument"},
      {"-s, --safemode", "run in safemode"},
      {"-a, --ast <input_file_path>",
       "print abstract syntax tree of `.🥝` file"},
      {"-m, --minify <input_file_path>", "create a `.min.🥝` file"},
      {"-t, --tokenize <input_file_path>",
       "tokenize a file with the kiwi lexer"},
      {"-X<key>=<value>", "specify an argument as a key-value pair"}};

#ifdef _WIN64
  commands = {
      {"-h, --help", "print this message"},
      {"-v, --version", "print the current version"},
      {"-n, --new <filename>", "create a `.kiwi` file"},
      {"-p, --parse <kiwi_code>", "parse code"},
      {"-s, --safemode", "run in safemode"},
      {"-a, --ast <input_file_path>",
       "print abstract syntax tree of `.kiwi` file"},
      {"-m, --minify <input_file_path>", "create a `.min.kiwi` file"},
      {"-t, --tokenize <input_file_path>", "tokenize a file as kiwi code"},
      {"-X<key>=<value>", "specify an argument as a key-value pair"}};
#endif

  printVersion();

  std::cout << "Usage: kiwi [--flags] <script|args>" << std::endl
            << "Options:" << std::endl;

  for (const auto& cmd : commands) {
    std::cout << "  " << std::left << std::setw(40) << cmd.command
              << cmd.description << std::endl;
  }

  std::cout << std::endl;
  return 0;
}

#endif