class Module {
  private:
    std::vector<std::string> lines;
    std::string moduleName;

  public:
    Module() {}

    Module(std::string name) { moduleName = name; }

    ~Module() { clear(); }

    void clear() { lines.clear(); }

    void add(std::string line) { lines.push_back(line); }

    std::vector<std::string> get() { return lines; }

    std::string at(int index) { return lines.at(index); }

    int size() { return get().size(); }

    std::string name() { return moduleName; }
};