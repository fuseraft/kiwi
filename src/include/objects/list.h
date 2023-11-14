class List : public Collectable
{
private:
    vector<string> contents;
    vector<string> reversion;
    string listName;
    bool collectable;

public:
    List() {}

    List(string name)
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

    void setName(string s)
    {
        listName = s;
    }

    void listSort()
    {
        reversion = contents;
        sort(contents.begin(), contents.end());
    }

    void listReverse()
    {
        reversion = contents;
        reverse(contents.begin(), contents.end());
    }

    void listRevert()
    {
        contents = reversion;
    }

    void add(string line)
    {
        contents.push_back(line);
    }

    void remove(string line)
    {
        vector<string> newContents;

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

    string at(int index)
    {
        if (index < (int)contents.size())
            return contents.at(index);

        return "#!=no_line";
    }

    string name()
    {
        return listName;
    }

    int size()
    {
        return (int)contents.size();
    }
};