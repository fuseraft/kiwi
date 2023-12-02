class Container
{
private:
    std::vector<std::string> lines;
    std::string containerName,
        value;
    bool isNestedIF;
    bool _isBad;

public:
    Container() {}

    Container(std::string name)
    {
        initialize(name);
    }

    ~Container()
    {
        clear();
    }

    void setName(std::string name)
    {
        containerName = name;
    }

    void add(std::string line)
    {
        lines.push_back(line);
    }

    void setValue(std::string val)
    {
        value = val;
    }

    std::string getCase()
    {
        return value;
    }

    std::string at(int index)
    {
        if (index < (int)lines.size())
            return lines.at(index);

        return "#!=no_line";
    }

    void clear()
    {
        lines.clear();
    }

    std::vector<std::string> getLines()
    {
        return lines;
    }

    void initialize(std::string name)
    {
        containerName = name;
        isNestedIF = false;
    }

    std::string name()
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
        return _isBad;
    }

    void setIsBad(bool value)
    {
        _isBad = value;
    }
};