/**
 *  uslang: an unorthodox scripting language
 *  hybrid-typed, object-oriented, interpreted
 **/

#include <vector>
#include "include/uslang.h"

int main(int c, char **v) {
    int exit = 0;

    try {
        std::vector<std::string> args;
        for (int i = 0; i < c; ++i)
            args.push_back(v[i]);

        // args.push_back("--config");
        // args.push_back("/home/scott/work/usl/uslang/.uslconfig");

        exit = uslang(args.size(), args);
    } catch (const std::exception &e) {
        print_error(e);
        exit = -1;
    }

    return exit;
}