#ifndef STACKTRACE_H
#define STACKTRACE_H

#include <iostream>
#include <execinfo.h>
#include <cstdlib>
#include <cxxabi.h>

void printStackTrace()
{
    void *array[5];
    size_t size;

    size = backtrace(array, 5);

    char **messages = backtrace_symbols(array, size);

    std::cout << "Stacktrace:" << std::endl;
    for (size_t i = 0; i < size; i++)
    {
        std::cout << i << ": " << messages[i] << std::endl;

        // Demangle C++ function names
        size_t sz = 256; // just a guess, could be too small, too large?
        char *function = static_cast<char *>(malloc(sz));
        int status = 0;
        char *demangled = abi::__cxa_demangle(messages[i], function, &sz, &status);
        if (status == 0)
        {
            std::cout << "    " << demangled << std::endl;
        }

        free(function);
        free(demangled);
    }

    free(messages);
}

void printError(const std::exception &e)
{
    std::cerr << "Exception caught: " << e.what() << std::endl;
    printStackTrace();
}

#endif