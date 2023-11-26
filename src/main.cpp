/*
 * 	uslang: unorthodox scripting language
 *  hybrid-typed, object-oriented, repl shell, fuseraft
 */

#include "include/uslang.h"

int uslang(int c, char **v);

int main(int c, char **v)
{
    try
    {
        uslang(c, v);
    }
    catch (const exception &e)
    {
        print_error(e);
    }
}

int uslang(int c, char **v)
{
    RNG::seed();

    string usl(v[0]), opt(""), script("");
    initialize_state(usl);

    State.InitialDirectory = Env::cwd();

    switch (c)
    {
        case 1:
            mem.addArg(usl);
            return startREPL();

        case 2:
            opt = v[1];

            if (isScript(opt))
            {
                mem.addArg(opt);
                mem.loadScript(opt);
            }
            else if (is(opt, "h") || is(opt, "help"))
                help(usl);
            else if (is(opt, "v") || is(opt, "version"))
                show_version();
            else
            {
                mem.addArg(opt);
                return startREPL();
            }

            break;

        case 3:
            opt = v[1], script = v[2];

            if (is(opt, "p") || is(opt, "parse"))
            {
                string code("");

                for (int i = 0; i < (int)script.length(); i++)
                {
                    if (script[i] == '\'')
                        code.push_back('\"');
                    else
                        code.push_back(script[i]);
                }

                parse(code);
            }
            else
            {
                mem.addArg(opt);
                mem.addArg(script);
                    
                if (isScript(opt))
                    mem.loadScript(opt);
                else
                    return startREPL();
            }

            break;

        default:
            if (c < 3)
            {
                help(usl);
                break;
            }
            
            opt = v[1];

            for (int i = isScript(opt) ? 2 : 1; i < c; i++)
            {
                string arg(v[i]);
                mem.addArg(arg);
            }

            if (isScript(opt))
                mem.loadScript(opt);
            else
                return startREPL();

            break;
    }

    if (State.CurrentScript != usl)
    {
        exec.executeScript();
    }

    return 0;
}