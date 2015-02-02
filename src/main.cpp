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

#include "include/strings.h"
#include "include/objects.h"
#include "include/fileio.h"
#include "include/errors.h"
#include "include/environment.h"
#include "include/datetime.h"
#include "include/prototypes.h"
#include "include/parser.h"
#include "include/core.h"

int main(int c, char ** v)
{
    string noctis = v[0];
    setup();
    __Noctis = noctis;
    __InitialDirectory = cwd();
    // __Logging = false;

#ifdef _WIN32
    SetConsoleTitle("noctis");
#endif

    srand((unsigned int)time(NULL));

    if (c == 1)
    {
        __CurrentScript = noctis;
        args.push_back(noctis);
        __ArgumentCount = (int)args.size();
        loop(false);
    }
    else if (c == 2)
    {
        string opt = v[1];

        if (isScript(opt))
        {
            __CurrentScript = opt;
            args.push_back(opt);
            __ArgumentCount = (int)args.size();
            loadScript(opt);
        }
        else if (is(opt, "h") || is(opt, "help"))
            help(noctis);
        else if (is(opt, "u") || is(opt, "uninstall"))
            uninstall();
        else if (is(opt, "sl") || is(opt, "skipload"))
        {
            __CurrentScript = noctis;
            args.push_back(opt);
            __ArgumentCount = (int)args.size();
            loop(true);
        }
        else if (is(opt, "n") || is(opt, "negligence"))
        {
            __Negligence = true;
            __CurrentScript = noctis;
            args.push_back(opt);
            __ArgumentCount = (int)args.size();
            loop(true);
        }
        else if (is(opt, "v") || is(opt, "version"))
            displayVersion();
        else
        {
            __CurrentScript = noctis;
            args.push_back(opt);
            __ArgumentCount = (int)args.size();
            loop(false);
        }
    }
    else if (c == 3)
    {
        string opt = v[1], script = v[2];

        if (is(opt, "sl") || is(opt, "skipload"))
        {
            __CurrentScript = noctis;

            if (isScript(script))
            {
                __CurrentScript = script;
                args.push_back(opt);
                args.push_back(script);
                __ArgumentCount = (int)args.size();
                loadScript(script);
            }
            else
            {
                args.push_back(opt);
                args.push_back(script);
                __ArgumentCount = (int)args.size();
                loop(true);
            }
        }
        else if (is(opt, "n") || is(opt, "negligence"))
        {
            __Negligence = true;
            args.push_back(opt);
            args.push_back(script);
            __ArgumentCount = (int)args.size();
            if (isScript(script))
            {
                __CurrentScript = script;
                loadScript(script);
            }
            else
            {
                __CurrentScript = noctis;
                loop(true);
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
                __CurrentScript = opt;
                args.push_back(opt);
                args.push_back(script);
                __ArgumentCount = (int)args.size();
                loadScript(opt);
            }
            else
            {
                __CurrentScript = noctis;
                args.push_back(opt);
                args.push_back(script);
                __ArgumentCount = (int)args.size();
                loop(false);
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
                args.push_back(tmpStr);
            }

            __ArgumentCount = (int)args.size();

            loadScript(opt);
        }
        else
        {
            for (int i = 1; i < c; i++)
            {
                string tmpStr = v[i];
                args.push_back(tmpStr);
            }

            __ArgumentCount = (int)args.size();

            __CurrentScript = noctis;
            loop(false);
        }
    }
    else
        help(noctis);

    clearAll();

    return (0);
}
