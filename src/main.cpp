/*
 * 	uslang: an unorthodox scripting language 
 *
 *  hybrid-typed, object-oriented, interpreted
 */

#include "include/uslang.h"

int main(int c, char **v)
{
    int exit = 0;

    try
    {
        exit = uslang(c, v);
    }
    catch (const std::exception &e)
    {
        print_error(e);
        exit = -1;
    }

    return exit;
}