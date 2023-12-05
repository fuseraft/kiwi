class Switch {
  private:
    std::vector<Container> cases;
    Container defaultCase;
    int count;

  public:
    Switch() { count = 0; }

    ~Switch() {
        cases.clear();
        defaultCase.clear();
    }

    void clear() {
        cases.clear();
        defaultCase.clear();
        count = 0;
    }

    Container rightCase(std::string value) {
        for (int i = 0; i < (int)cases.size(); i++) {
            if (cases.at(i).getCase() == value)
                return cases.at(i);
        }

        return defaultCase;
    }

    void addCase(std::string value) {
        Container newCase("[case#" + itos(count) + "]");
        newCase.setValue(value);
        cases.push_back(newCase);
        count++;
    }

    void addToCase(std::string line) { cases.at(count - 1).add(line); }

    void addToDefault(std::string line) { defaultCase.add(line); }
};