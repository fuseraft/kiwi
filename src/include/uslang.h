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
#include "parsing/interp.h"

int uslang(int c, char **v) {
    RNG::getInstance();

    std::string usl(v[0]), opt, script;
    initialize_state(usl);

    State.InitialDirectory = FileIO::getCurrentDirectory();

    // WIP: new interpreter logic
    InterpSession session;
    bool xmode = false, startxrepl = false;
    std::regex xargPattern("-X(.*?)=");

    for (int i = 0; i < c; ++i) {
        opt = v[i];

        if (is(opt, "h") || is(opt, "help")) {
            help(usl);
            return 0;
        } else if (is(opt, "v") || is(opt, "version")) {
            show_version();
            return 0;
        } else if (is(opt, "x") || is(opt, "experimental")) {
            xmode = true;
        } else if (is(opt, "xr") || is(opt, "x-repl")) {
            xmode = true;
            startxrepl = true;
        } else if (is(opt, "r") || is(opt, "repl")) {
            if (xmode) continue;
            return load_repl();
        } else if (is(opt, "p") || is(opt, "parse")) {
            if (xmode) continue;
            if (i + 1 > c) {
                help(usl);
                break;   
            }

            script = v[i + 1];
            parse(script);
            return State.LastErrorCode;
        }
        else if (is_script(opt)) {
            if (xmode) {
                session.registerScript(opt);
                continue;
            }

            engine.addArg(opt);
            engine.loadScript(opt);
            break;
        }
        else if (xmode && begins_with(opt, "-X") && contains(opt, "=")) {
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
        else if (!xmode)
            engine.addArg(opt);
    }

    if (xmode)
        return session.start(startxrepl);

    if (State.CurrentScript != usl)
        exec.executeScript();

    return State.LastErrorCode;
}