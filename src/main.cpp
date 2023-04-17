/**
 * 	noctis: a hybrid-typed, object-oriented, interpreted, programmable command line shell.
 *
 *		scstauf@gmail.com
 **/

#include <iostream>
#include <sstream>
#include <fstream>
#include <dirent.h>
#include <algorithm>
#include <cfloat>

#ifdef __linux__
#include <vector>
#include <sys/stat.h>
#include <cmath>
#include <string.h>
#include <unistd.h>
#endif

#ifdef _WIN32
#include <windows.h>
#endif

using namespace std;

void doNothing() { }

#include "include/constants.h"
#include "include/noctisenv.h"
#include "include/state.h"
#include "include/strings.h"
#include "include/rng.h"
#include "include/objects.h"
#include "include/prototypes.h"
#include "include/error.h"
#include "include/env.h"
#include "include/memory.h"
#include "include/datetime.h"
#include "include/executor.h"
#include "include/components.h"
#include "include/fileio.h"
#include "include/parser.h"
#include "include/core.h"

int main(int c, char ** v)
{
    RNG::seed();
    
    setup();

    string noctis = v[0];
    State.Noctis = noctis;
    NoctisEnv.InitialDirectory = Env::cwd();

    if (c == 1)
    {
        State.CurrentScript = noctis;
        mem.addArg(noctis);
        State.ArgumentCount = mem.getArgCount();
        startREPL(false);
    }
    else if (c == 2)
    {
        string opt = v[1];

        if (isScript(opt))
        {
            State.CurrentScript = opt;
            mem.addArg(opt);
            State.ArgumentCount = mem.getArgCount();
            mem.loadScript(opt);
        }
        else if (is(opt, "h") || is(opt, "help"))
            help(noctis);
        else if (is(opt, "u") || is(opt, "uninstall"))
            uninstall();
        else if (is(opt, "sl") || is(opt, "skipload"))
        {
            State.CurrentScript = noctis;
            mem.addArg(opt);
            State.ArgumentCount = mem.getArgCount();
            startREPL(true);
        }
        else if (is(opt, "n") || is(opt, "negligence"))
        {
            State.Negligence = true;
            State.CurrentScript = noctis;
            mem.addArg(opt);
            State.ArgumentCount = mem.getArgCount();
            startREPL(true);
        }
        else if (is(opt, "v") || is(opt, "version"))
            displayVersion();
        else
        {
            State.CurrentScript = noctis;
            mem.addArg(opt);
            State.ArgumentCount = mem.getArgCount();
            startREPL(false);
        }
    }
    else if (c == 3)
    {
        string opt = v[1], script = v[2];

        if (is(opt, "sl") || is(opt, "skipload"))
        {
            State.CurrentScript = noctis;

            if (isScript(script))
            {
                State.CurrentScript = script;
                mem.addArg(opt);
                mem.addArg(script);
                State.ArgumentCount = mem.getArgCount();
                mem.loadScript(script);
            }
            else
            {
                mem.addArg(opt);
                mem.addArg(script);
                State.ArgumentCount = mem.getArgCount();
                startREPL(true);
            }
        }
        else if (is(opt, "n") || is(opt, "negligence"))
        {
            State.Negligence = true;
            mem.addArg(opt);
            mem.addArg(script);
            State.ArgumentCount = mem.getArgCount();
            if (isScript(script))
            {
                State.CurrentScript = script;
                mem.loadScript(script);
            }
            else
            {
                State.CurrentScript = noctis;
                startREPL(true);
            }
        }
        else if (is(opt, "p") || is(opt, "parse"))
        {
            string stringBuilder("");

            for (int i = 0; i < (int)script.length(); i++)
            {
                if (script[i] == '\'')
                    stringBuilder.push_back('\"');
                else
                    stringBuilder.push_back(script[i]);
            }

            parse(stringBuilder);
        }
        else
        {
            if (isScript(opt))
            {
                State.CurrentScript = opt;
                mem.addArg(opt);
                mem.addArg(script);
                State.ArgumentCount = mem.getArgCount();
                mem.loadScript(opt);
            }
            else
            {
                State.CurrentScript = noctis;
                mem.addArg(opt);
                mem.addArg(script);
                State.ArgumentCount = mem.getArgCount();
                startREPL(false);
            }
        }
    }
    else if (c > 3)
    {
        string opt = v[1];

        if (isScript(opt))
        {
            for (int i = 2; i < c; i++)
            {
                string tmpStr = v[i];
                mem.addArg(tmpStr);
            }

            State.ArgumentCount = mem.getArgCount();

            mem.loadScript(opt);
        }
        else
        {
            for (int i = 1; i < c; i++)
            {
                string tmpStr = v[i];
                mem.addArg(tmpStr);
            }

            State.ArgumentCount = mem.getArgCount();

            State.CurrentScript = noctis;
            startREPL(false);
        }
    }
    else
        help(noctis);

    if (State.CurrentScript != "")
    {
        exec.executeScript();
    }

    mem.clearAll();

    return 0;
}
