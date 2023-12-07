#include <iostream>
#include <sstream>
#include <fstream>
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

int uslang(int c, char **v) {
    RNG::getInstance();

    std::string usl(v[0]), opt, script;
    initialize_state(usl);

    State.InitialDirectory = FileIO::getCurrentDirectory();

    switch (c) {
    case 1:
        engine.addArg(usl);
        return load_repl();

    case 2:
        opt = v[1];

        if (is_script(opt)) {
            engine.addArg(opt);
            engine.loadScript(opt);
        } else if (is(opt, "h") || is(opt, "help"))
            help(usl);
        else if (is(opt, "v") || is(opt, "version"))
            show_version();
        else {
            engine.addArg(opt);
            return load_repl();
        }

        break;

    case 3:
        opt = v[1], script = v[2];

        if (is(opt, "p") || is(opt, "parse")) {
            std::string code;

            for (int i = 0; i < (int)script.length(); i++) {
                if (script[i] == '\'')
                    code.push_back('\"');
                else
                    code.push_back(script[i]);
            }

            parse(code);
        } else {
            engine.addArg(opt);
            engine.addArg(script);

            if (is_script(opt))
                engine.loadScript(opt);
            else
                return load_repl();
        }

        break;

    default:
        if (c < 3) {
            help(usl);
            break;
        }

        opt = v[1];

        for (int i = is_script(opt) ? 2 : 1; i < c; i++) {
            std::string arg(v[i]);
            engine.addArg(arg);
        }

        if (is_script(opt))
            engine.loadScript(opt);
        else
            return load_repl();

        break;
    }

    if (State.CurrentScript != usl)
        exec.executeScript();

    return State.LastErrorCode;
}