#include <iostream>
#include <sstream>
#include <fstream>
#include <regex>
#include <dirent.h>
#include <algorithm>
#include <cfloat>
#include <vector>
#include <sys/stat.h>
#include <cmath>
#include <string.h>
#include <unistd.h>

const std::string uslang_name = "uslang";
const std::string uslang_version = "0.1.2";

void print_underconstruction() {
    std::cout << "under construction" << std::endl;
}

#include "parsing/dotsep.h"
#include "debug/stacktrace.h"
#include "parsing/keywords.h"
#include "parsing/strings.h"
#include "usl/state.h"
#include "usl/thread.h"
#include "usl/date.h"
#include "usl/rng.h"
#include "usl/error.h"
#include "usl/fileio.h"
#include "usl/objects.h"
#include "usl/prototypes.h"
#include "usl/env.h"
#include "usl/engine.h"
#include "usl/executor.h"
#include "usl/components.h"
#include "parsing/parser.h"
#include "usl/core.h"

// The new interpreter
#include "configuration/config.h"
#include "logging/logger.h"
#include "parsing/interp_session.h"

void handle_xarg(std::string &opt, std::__cxx11::regex &xargPattern,
                 InterpSession &session);

void configure_usl(Config &config, bool &xmode, Logger &logger,
                   InterpSession &session);

int uslang(int c, std::vector<std::string> v) {
    RNG::getInstance();

    std::string usl(v.at(0)), opt, script;
    initialize_state(usl);

    State.InitialDirectory = FileIO::getCurrentDirectory();

    bool xmode = false, startxrepl = false;
    std::regex xargPattern("-X(.*?)=");

    Config config;
    Logger logger;

    // WIP: new interpreter logic
    Interpreter interp(logger);
    InterpSession session(logger, interp);

    for (int i = 0; i < c; ++i) {
        if (i == 0)
            session.registerArg("USL", opt);

        opt = v.at(i);

        if (is(opt, "c") || is(opt, "config")) {
            if (i + 1 > c) {
                help(usl);
                break;
            }

            std::string configFilePath = v[i + 1];

            if (ends_with(configFilePath, ".uslconfig") && config.read(configFilePath)) {
                configure_usl(config, xmode, logger, session);
                break;
            }
        } else if (is(opt, "h") || is(opt, "help")) {
            help(usl);
            return 0;
        } else if (is(opt, "v") || is(opt, "version")) {
            show_version();
            return 0;
        } else if (is(opt, "x") || is(opt, "experimental"))
            xmode = true;
        else if (is(opt, "xr") || is(opt, "x-repl")) {
            xmode = true;
            startxrepl = true;
        } else if (is(opt, "r") || is(opt, "repl")) {
            if (xmode)
                continue;
            return load_repl();
        } else if (is(opt, "p") || is(opt, "parse")) {
            if (xmode)
                continue;
            if (i + 1 > c) {
                help(usl);
                break;
            }

            script = v[i + 1];
            parse(script);
            return State.LastErrorCode;
        } else if (is_script(opt)) {
            if (xmode) {
                session.registerScript(opt);
                continue;
            }

            engine.addArg(opt);
            engine.loadScript(opt);
            break;
        } else if (xmode && begins_with(opt, "-X") && contains(opt, "="))
            handle_xarg(opt, xargPattern, session);
        else if (!xmode)
            engine.addArg(opt);
    }

    // Start with the new interpreter.
    if (xmode)
        return session.start(startxrepl);

    // Start with the legacy interpreter.
    if (State.CurrentScript != usl)
        exec.executeScript();

    return State.LastErrorCode;
}

void configure_usl(Config &config, bool &xmode, Logger &logger,
                   InterpSession &session) {
    std::string logPath = config.get("LOGGER_PATH");
    std::string logMode = config.get("LOGGER_MODE");
    std::string logLevel = config.get("LOGGER_LEVEL");
    std::string scriptPath = config.get("SCRIPT_PATH");

    // #FILDIL
    xmode = true;

    if (!logPath.empty())
        logger.setLogFilePath(logPath);

    if (!logMode.empty())
        logger.setLogMode(Logger::logmode_from_string(logMode));

    if (!logLevel.empty())
        logger.setMinimumLogLevel(Logger::loglevel_from_string(logLevel));

    if (!scriptPath.empty())
        session.registerScript(scriptPath);
}

void handle_xarg(std::string &opt, std::__cxx11::regex &xargPattern,
                 InterpSession &session) {
    std::string xargName, xargValue;
    std::smatch match;
    if (std::regex_search(opt, match, xargPattern))
        xargName = match[1].str();

    size_t equalSignPos = opt.find('=');
    if (equalSignPos != std::string::npos)
        xargValue = opt.substr(equalSignPos + 1);

    if (!xargName.empty() && !xargValue.empty())
        session.registerArg(xargName, xargValue);
}
