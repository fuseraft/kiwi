#ifndef IO_H
#define IO_H

class IO
{
public:
    IO() {}
    ~IO() {}

    static void print(string s)
    {
        cout << s;
    }

    static void printerr(string s)
    {
        cerr << s;
    }

    static void println(string s)
    {
        cout << s << endl;
    }

    static void printerrln(string s)
    {
        cerr << s << endl;
    }

    static void println()
    {
        cout << endl;
    }
};

#endif