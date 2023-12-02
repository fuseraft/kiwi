class List : public Collectable
{
private:
    std::vector<std::string> contents;
    std::vector<std::string> reversion;
    std::string listName;
    bool collectable;

public:
    List() {}

    List(std::string name)
    {
        collectable = false;
        listName = name;
    }

    ~List()
    {
        clear();
    }

    void setCollectable(bool value) override
    {
        collectable = value;
    }

    bool isCollectable() const override
    {
        return collectable;
    }

    void setName(std::string s)
    {
        listName = s;
    }

    void sort()
    {
        reversion = contents;
        std::sort(contents.begin(), contents.end());
    }

    void reverse()
    {
        reversion = contents;
        std::reverse(contents.begin(), contents.end());
    }

    void revert()
    {
        contents = reversion;
    }

    void add(std::string line)
    {
        contents.push_back(line);
    }

    void remove(std::string line)
    {
        std::vector<std::string> newContents;

        for (int i = 0; i < size(); i++)
        {
            if (at(i) != line)
                newContents.push_back(at(i));
        }

        clear();

        contents = newContents;
    }

    void clear()
    {
        contents.clear();
    }

    std::string at(int index)
    {
        if (index < (int)contents.size())
            return contents.at(index);

        return "#!=no_line";
    }

    std::string name()
    {
        return listName;
    }

    int size()
    {
        return (int)contents.size();
    }
};