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
#include <vector>
#include <sys/stat.h>
#include <cmath>
#include <string.h>
#include <unistd.h>

using namespace std;

#include "include/debug/stacktrace.h"
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

int noctis(int c, char **v);

int main(int c, char **v)
{
    try
    {
        noctis(c, v);
    }
    catch (const exception &e)
    {
        printError(e);
    }
}

int noctis(int c, char **v)
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
        return startREPL();
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
        else if (is(opt, "v") || is(opt, "version"))
            displayVersion();
        else
        {
            State.CurrentScript = noctis;
            mem.addArg(opt);
            State.ArgumentCount = mem.getArgCount();
            return startREPL();
        }
    }
    else if (c == 3)
    {
        string opt = v[1], script = v[2];

        if (is(opt, "p") || is(opt, "parse"))
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
                return startREPL();
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
            return startREPL();
        }
    }
    else
        help(noctis);

    if (State.CurrentScript != "")
    {
        exec.executeScript();
    }

    return 0;
}