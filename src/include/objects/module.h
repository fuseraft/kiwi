class Module
{
private:
    std::vector<std::string> lines;
    string moduleName;

public:
    Module() {}

    Module(std::string name)
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

    void add(std::string line)
    {
        lines.push_back(line);
    }

    std::vector<std::string> get()
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