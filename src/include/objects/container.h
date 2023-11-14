class Container
{
private:
    vector<string> lines;
    string containerName,
        value;
    bool isNestedIF;

public:
    Container() {}

    Container(string name)
    {
        initialize(name);
    }

    ~Container()
    {
        clear();
    }

    void setName(string name)
    {
        containerName = name;
    }

    void add(string line)
    {
        lines.push_back(line);
    }

    void setValue(string val)
    {
        value = val;
    }

    string getCase()
    {
        return value;
    }

    string at(int index)
    {
        if (index < (int)lines.size())
            return lines.at(index);

        return "#!=no_line";
    }

    void clear()
    {
        lines.clear();
    }

    vector<string> getLines()
    {
        return lines;
    }

    void initialize(string name)
    {
        containerName = name;
        isNestedIF = false;
    }

    string name()
    {
        return containerName;
    }

    int size()
    {
        return (int)lines.size();
    }

    bool isIF()
    {
        return isNestedIF;
    }

    void setBool(bool b)
    {
        isNestedIF = b;
    }

    bool isBad()
    {
        if (startsWith(name(), "[bad_nest"))
            return true;

        return false;
    }
};