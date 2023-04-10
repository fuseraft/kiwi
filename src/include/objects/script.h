class Script
{
private:
    vector<string> lines;
    vector<string> marks;
    string scriptName;

public:
    Script() {}

    Script(string name)
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

    void add(string line)
    {
        lines.push_back(line);
    }

    void addMark(string mark)
    {
        marks.push_back(mark);
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

    string markAt(int index)
    {
        if (index < (int)marks.size())
            return marks.at(index);

        return "[no_line]";
    }

    bool markExists(string mark)
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

    string name()
    {
        return scriptName;
    }
};