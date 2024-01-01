#include <algorithm>
#include <cfloat>
#include <cmath>
#include <dirent.h>
#include <fstream>
#include <iostream>
#include <regex>
#include <sstream>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

const std::string uslang_name    = "uslang";
const std::string uslang_version = "1.0.0";

#include "configuration/config.h"
#include "debug/stacktrace.h"
#include "logging/logger.h"
#include "math/rng.h"
#include "parsing/interp_session.h"
#include "parsing/keywords.h"
#include "parsing/strings.h"

void handle_xarg(
    std::string &opt, std::__cxx11::regex &xargPattern, InterpSession &session);

void configure_usl(Config &config, Logger &logger, InterpSession &session);

int show_version();
int help(std::string app);

int uslang(int c, std::vector<std::string> v) {
    RNG::getInstance();

    std::string usl(v.at(0)), opt, script;

    std::regex xargPattern("-X(.*?)=");
    bool       startxrepl = false;

    Config config;
    Logger logger;

    // WIP: new interpreter logic
    Interpreter   interp(logger);
    InterpSession session(logger, interp);

    for (int i = 0; i < c; ++i) {
        if (i == 0) {
            session.registerArg("USL", opt);
        }

        opt = v.at(i);

        if (begins_with(opt, "-X") && contains(opt, "=")) {
            handle_xarg(opt, xargPattern, session);
        } else if (is_flag(opt, "h", "help")) {
            return help(usl);
        } else if (is_flag(opt, "v", "version")) {
            return show_version();
        } else if (is_flag(opt, "R", "repl")) {
            startxrepl = true;
        } else if (is_flag(opt, "C", "config")) {
            if (i + 1 > c)
                return help(usl);

            std::string configFilePath = v[i + 1];

            if (!ends_with(configFilePath, ".conf")) {
                logger.error("I can be configured with a `.conf` file.");
            } else if (!config.read(configFilePath)) {
                logger.error("I cannot read `" + configFilePath + "`.");
            } else {
                configure_usl(config, logger, session);
            }
        } else if (is_script(opt)) {
            session.registerScript(opt);
        } else {
            logger.debug("Unknown option: " + opt);
        }
    }

    return session.start(startxrepl);
}

void configure_usl(Config &config, Logger &logger, InterpSession &session) {
    std::string logPath    = config.get("LOGGER_PATH");
    std::string logMode    = config.get("LOGGER_MODE");
    std::string logLevel   = config.get("LOGGER_LEVEL");
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

void handle_xarg(
    std::string &opt, std::__cxx11::regex &xargPattern,
    InterpSession &session) {
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
        session.registerArg(name, value);
    }
}

int show_version() {
    std::cout << uslang_name << " interpreter "
              << "v" << uslang_version << std::endl
              << std::endl;
    return 0;
}

int help(std::string app) {
    struct CommandInfo {
        std::string command;
        std::string description;
    };

    std::vector<CommandInfo> commands = {
        {"-h, --help", "show this message"},
        {"-v, --version", "show current version"},
        {"-C, --config <.uslconfig>", "use a configuration file"},
        {"-R, --repl", "start the REPL"},
        {"-X<arg_key>=<arg_value>", "pass an argument as key-value pair"}};

    show_version();

    std::cout << "Usage: usl [--flags] <script|args>" << std::endl
              << "Options:" << std::endl;

    for (const auto &cmd : commands) {
        std::cout << std::left << std::setw(30) << (app + " " + cmd.command)
                  << cmd.description << std::endl;
    }

    std::cout << std::endl;
    return 0;
}