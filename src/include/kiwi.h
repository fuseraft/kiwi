#include <regex>
#include <vector>

const std::string kiwi_name = "Unorthodox Scripting Language";
const std::string kiwi_version = "1.0.1";

#include "configuration/config.h"
#include "errors/error.h"
#include "errors/error_handler.h"
#include "logging/logger.h"
#include "math/rng.h"
#include "parsing/keywords.h"
#include "parsing/strings.h"
#include "interp_session.h"

bool has_script_extension(std::string path);
bool has_conf_extension(std::string path);
void handle_xarg(std::string& opt, std::__cxx11::regex& xargPattern,
                 InterpSession& session);
void configure_kiwi(Config& config, Logger& logger, InterpSession& session);
int process_args(int c, std::string& opt, std::vector<std::string>& v,
                 std::__cxx11::regex& xargPattern, InterpSession& session,
                 bool& replMode, Logger& logger, Config& config, bool& retFlag);
int print_version();
int print_help();

int kiwi(int c, std::vector<std::string> v) {
  RNG::getInstance();

  std::string opt;

  std::regex xargPattern("-X(.*?)=");
  bool replMode = false;

  Logger logger;
  Interpreter interp(logger);
  InterpSession session(logger, interp);
  Config config;

  try {
    session.registerArg("kiwi", v.at(0));

    bool retFlag;
    int retVal = process_args(c, opt, v, xargPattern, session, replMode, logger,
                              config, retFlag);
    if (retFlag)
      return retVal;

    return session.start(replMode);
  } catch (const KiwiError& e) {
    return ErrorHandler::handleError(e);
  }
}

int process_args(int c, std::string& opt, std::vector<std::string>& v,
                 std::__cxx11::regex& xargPattern, InterpSession& session,
                 bool& replMode, Logger& logger, Config& config,
                 bool& retFlag) {
  retFlag = true;
  for (int i = 1; i < c; ++i) {
    opt = v.at(i);

    if (begins_with(opt, "-X") && contains(opt, "=")) {
      handle_xarg(opt, xargPattern, session);
    } else if (is_flag(opt, "h", "help")) {
      return print_help();
    } else if (is_flag(opt, "v", "version")) {
      return print_version();
    } else if (is_flag(opt, "R", "repl")) {
      replMode = true;
    } else if (is_flag(opt, "C", "config")) {
      if (i + 1 > c) {
        return print_help();
      }

      std::string conf = v[i + 1];

      if (!has_conf_extension(conf)) {
        logger.error("I can be configured with a `.conf` file.");
      } else if (!config.read(conf)) {
        logger.error("I cannot read `" + conf + "`.");
      } else {
        configure_kiwi(config, logger, session);
        ++i;
      }
    } else if (has_script_extension(opt)) {
      session.registerScript(opt);
    } else {
      // logger.debug("Unknown option: " + opt);
    }
  }
  retFlag = false;
  return {};
}

void configure_kiwi(Config& config, Logger& logger, InterpSession& session) {
  std::string logPath = config.get("LOGGER_PATH");
  std::string logMode = config.get("LOGGER_MODE");
  std::string logLevel = config.get("LOGGER_LEVEL");
  std::string scriptPath = config.get("SCRIPT_PATH");

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
    session.registerScript(scriptPath);
  }
}

void handle_xarg(std::string& opt, std::__cxx11::regex& xargPattern,
                 InterpSession& session) {
  std::string name, value;
  std::smatch match;

  if (std::regex_search(opt, match, xargPattern)) {
    name = match[1].str();
  }

  size_t pos = opt.find(':');
  if (pos != std::string::npos) {
    value = opt.substr(pos + 1);
  }

  if (!name.empty() && !value.empty()) {
    session.registerArg(name, value);
  }
}

bool has_script_extension(std::string path) {
  return ends_with(path, ".kiwi") || ends_with(path, ".kiwi");
}

bool has_conf_extension(std::string path) {
  return ends_with(path, ".conf");
}

int print_version() {
  std::cout << kiwi_name << " v" << kiwi_version << std::endl << std::endl;
  return 0;
}

int print_help() {
  struct CommandInfo {
    std::string command;
    std::string description;
  };

  std::vector<CommandInfo> commands = {
      {"-R, --repl", "start REPL mode"},
      {"-h, --help", "print this message"},
      {"-v, --version", "print the current version"},
      {"-C, --config <conf_path>", "configure with a `.conf` file"},
      {"-X<key>:<value>", "specify an argument as a key-value pair"}};

  print_version();

  std::cout << "Usage: kiwi [--flags] <script|args>" << std::endl
            << "Options:" << std::endl;

  for (const auto& cmd : commands) {
    std::cout << "  " << std::left << std::setw(40) << cmd.command
              << cmd.description << std::endl;
  }

  std::cout << std::endl;
  return 0;
}