class Class : public Collectable {
  private:
    std::vector<Method> methods;
    std::vector<Variable> variables;

    std::string className;
    std::string currentMethod;

    bool collectable;

  public:
    Class() {}

    Class(std::string name) {
        initialize(name);
        currentMethod = "";
    }

    ~Class() { clear(); }

    void initialize(std::string name) { currentMethod = "", className = name; }

    void setName(std::string name) { className = name; }

    void setCollectable(bool value) override { collectable = true; }

    bool isCollectable() const override { return collectable; }

    Method getCurrentMethod() { return getMethod(currentMethod); }

    void setCurrentMethod(std::string methodName) {
        currentMethod = methodName;
    }

    void setPublic() {
        if (hasMethod(currentMethod))
            methods.at(methodAt(currentMethod)).setPublic();
    }

    void setPrivate() {
        if (hasMethod(currentMethod))
            methods.at(methodAt(currentMethod)).setPrivate();
    }

    void addToCurrentMethod(std::string line) {
        if (hasMethod(currentMethod))
            methods.at(methodAt(currentMethod)).add(line);
    }

    int methodSize() { return (int)methods.size(); }

    int variableSize() { return (int)variables.size(); }

    int methodAt(std::string methodName) {
        for (int i = 0; i < methodSize(); i++) {
            if (methods.at(i).name() == methodName)
                return i;
        }

        return -1;
    }

    int variableAt(std::string variableName) {
        for (int i = 0; i < variableSize(); i++) {
            if (variables.at(i).name() == variableName)
                return i;
        }

        return -1;
    }

    std::string getMethodName(int index) {
        if (index < (int)methods.size())
            return methods.at(index).name();

        return "[undefined]";
    }

    std::string getVariableName(int index) {
        if (index < (int)variables.size())
            return variables.at(index).name();

        return "[undefined]";
    }

    void addMethod(Method method) {
        if (!method.isBad())
            methods.push_back(method);
    }

    void addVariable(Variable variable) {
        if (!variable.isBad())
            variables.push_back(variable);
    }

    void clear() {
        clearMethods();
        clearVariables();
    }

    void clearMethods() { methods.clear(); }

    void clearVariables() { variables.clear(); }

    void removeVariable(std::string variableName) {
        std::vector<Variable> oldVariables = getVariables();

        clearVariables();

        for (int i = 0; i < (int)oldVariables.size(); i++)
            if (oldVariables.at(i).name() != variableName)
                variables.push_back(oldVariables.at(i));
    }

    Method getMethod(std::string methodName) {
        for (int i = 0; i < (int)methods.size(); i++)
            if (methods.at(i).name() == methodName)
                return methods.at(i);

        Method badMethod;
        badMethod.setIsBad(true);
        return badMethod;
    }

    std::vector<Method> getMethods() { return methods; }

    Variable getVariable(std::string variableName) {
        for (int i = 0; i < (int)variables.size(); i++)
            if (variables.at(i).name() == variableName)
                return variables.at(i);

        Variable badVariable;
        badVariable.setIsBad(true);
        return badVariable;
    }

    std::vector<Variable> getVariables() { return variables; }

    bool hasMethod(std::string methodName) {
        for (int i = 0; i < (int)methods.size(); i++)
            if (methods.at(i).name() == methodName)
                return true;

        return false;
    }

    std::string name() { return className; }

    bool hasVariable(std::string variableName) {
        for (int i = 0; i < (int)variables.size(); i++)
            if (variables.at(i).name() == variableName)
                return true;

        return false;
    }
};