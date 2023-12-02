class Script
{
private:
    std::vector<std::string> lines;
    std::vector<std::string> marks;
    std::string scriptName;

public:
    Script() {}

    Script(std::string name)
    {
        scriptName = name;
    }

    ~Script()
    {
        clear();
    }

    void clear()
    {
        lines.clear();
        marks.clear();
    }

    void add(std::string line)
    {
        lines.push_back(line);
    }

    void addMark(std::string mark)
    {
        marks.push_back(mark);
    }

    std::vector<std::string> get()
    {
        return lines;
    }

    std::string at(int index)
    {
        if (index < (int)lines.size())
            return lines.at(index);

        return "[no_line]";
    }

    std::string markAt(int index)
    {
        if (index < (int)marks.size())
            return marks.at(index);

        return "[no_line]";
    }

    bool markExists(std::string mark)
    {
        for (int i = 0; i < (int)marks.size(); i++)
        {
            if (marks.at(i) == mark)
                return true;
        }

        return false;
    }

    int markSize()
    {
        return (int)marks.size();
    }

    int size()
    {
        return get().size();
    }

    std::string name()
    {
        return scriptName;
    }
};