class Switch
{
private:
    vector<Container>   cases;
    Container           defaultCase;
    int                 count;

public:
    Switch()
    {
        count = 0;
    }

    ~Switch()
    {
        cases.clear();
        defaultCase.clear();
    }

    void clear()
    {
        cases.clear();
        defaultCase.clear();
        count = 0;
    }

    Container rightCase(string value)
    {
        for (int i = 0; i < (int)cases.size(); i++)
        {
            if (cases.at(i).getCase() == value)
                return cases.at(i);
        }

        return defaultCase;
    }

    void addCase(string value)
    {
        Container newCase("[case#" + itos(count) + "]");
        newCase.setValue(value);
        cases.push_back(newCase);
        count++;
    }

    void addToCase(string line)
    {
        cases.at(count - 1).add(line);
    }

    void addToDefault(string line)
    {
        defaultCase.add(line);
    }
};