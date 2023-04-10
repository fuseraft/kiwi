class Module
{
private:
    vector<string>  lines;
    string          moduleName;

public:
    Module() {}

    Module(string name)
    {
        moduleName = name;
    }

    ~Module()
    {
        clear();
    }

    void clear()
    {
        lines.clear();
    }

    void add(string line)
    {
        lines.push_back(line);
    }

    vector<string> get()
    {
        return lines;
    }

    string at(int index)
    {
        if (index < (int)lines.size())
            return lines.at(index);

        return "[no_line]";
    }

    int size()
    {
        return get().size();
    }

    string name()
    {
        return moduleName;
    }
};