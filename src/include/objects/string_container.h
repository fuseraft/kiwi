class StringContainer
{
private:
    std::vector<std::string> strings;

public:
    StringContainer() {}

    ~StringContainer()
    {
        clear();
    }

    void clear()
    {
        strings.clear();
    }

    void add(std::string line)
    {
        strings.push_back(line);
    }

    std::vector<std::string> get()
    {
        return strings;
    }

    std::string at(int index)
    {
        if (index < (int)strings.size())
            return strings.at(index);

        return "[no_line]";
    }
};