class StringContainer
{
private:
    vector<string> strings;

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

    void add(string line)
    {
        strings.push_back(line);
    }

    vector<string> get()
    {
        return strings;
    }

    string at(int index)
    {
        if (index < (int)strings.size())
            return strings.at(index);

        return "[no_line]";
    }
};