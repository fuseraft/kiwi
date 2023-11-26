#ifndef IO_H
#define IO_H

class IO
{
public:
    IO() {}
    ~IO() {}

    static void print(std::string s)
    {
        std::cout << s;
    }

    static void printerr(std::string s)
    {
        std::cerr << s;
    }

    static void println(std::string s)
    {
        std::cout << s << std::endl;
    }

    static void printerrln(std::string s)
    {
        std::cerr << s << std::endl;
    }

    static void println()
    {
        std::cout << std::endl;
    }
};

#endif