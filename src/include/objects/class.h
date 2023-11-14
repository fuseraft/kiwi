#include "../io.h"

class Class : public Collectable
{
private:
    vector<Method> methods;
    vector<Variable> variables;

    string className;
    string currentMethod;

    bool collectable;

public:
    Class() {}

    Class(string name)
    {
        initialize(name);
        currentMethod = "";
    }

    ~Class()
    {
        clear();
    }

    void initialize(string name)
    {
        currentMethod = "",
        className = name;
    }

    void setName(string name)
    {
        className = name;
    }

    void setCollectable(bool value) override
    {
        collectable = true;
    }

    bool isCollectable() const override
    {
        return collectable;
    }

    Method getCurrentMethod()
    {
        return getMethod(currentMethod);
    }

    void setCurrentMethod(string methodName)
    {
        currentMethod = methodName;
    }

    void setPublic()
    {
        if (hasMethod(currentMethod))
            methods.at(methodAt(currentMethod)).setPublic();
    }

    void setPrivate()
    {
        if (hasMethod(currentMethod))
            methods.at(methodAt(currentMethod)).setPrivate();
    }

    void addToCurrentMethod(string line)
    {
        if (hasMethod(currentMethod))
            methods.at(methodAt(currentMethod)).add(line);
    }

    int methodSize()
    {
        return (int)methods.size();
    }

    int variableSize()
    {
        return (int)variables.size();
    }

    int methodAt(string methodName)
    {
        for (int i = 0; i < methodSize(); i++)
        {
            if (methods.at(i).name() == methodName)
                return i;
        }

        return -1;
    }

    int variableAt(string variableName)
    {
        for (int i = 0; i < variableSize(); i++)
        {
            if (variables.at(i).name() == variableName)
                return i;
        }

        return -1;
    }

    string getMethodName(int index)
    {
        if (index < (int)methods.size())
            return methods.at(index).name();

        return "[undefined]";
    }

    string getVariableName(int index)
    {
        if (index < (int)variables.size())
            return variables.at(index).name();

        return "[undefined]";
    }

    void addMethod(Method method)
    {
        if (!method.isBad())
            methods.push_back(method);
    }

    void addVariable(Variable variable)
    {
        if (!variable.isNull())
            variables.push_back(variable);
    }

    void clear()
    {
        clearMethods();
        clearVariables();
    }

    void clearMethods()
    {
        methods.clear();
    }

    void clearVariables()
    {
        variables.clear();
    }

    void removeVariable(string variableName)
    {
        vector<Variable> oldVariables = getVariables();

        clearVariables();

        for (int i = 0; i < (int)oldVariables.size(); i++)
            if (oldVariables.at(i).name() != variableName)
                variables.push_back(oldVariables.at(i));
    }

    Method getMethod(string methodName)
    {
        for (int i = 0; i < (int)methods.size(); i++)
            if (methods.at(i).name() == methodName)
                return methods.at(i);

        Method badMethod;
        badMethod.setIsBad(true);
        return badMethod;
    }

    vector<Method> getMethods()
    {
        return methods;
    }

    Variable getVariable(string variableName)
    {
        for (int i = 0; i < (int)variables.size(); i++)
            if (variables.at(i).name() == variableName)
                return variables.at(i);

        Variable badVariable;
        badVariable.setIsBad(true);
        return badVariable;
    }

    vector<Variable> getVariables()
    {
        return variables;
    }

    bool hasMethod(string methodName)
    {
        for (int i = 0; i < (int)methods.size(); i++)
            if (methods.at(i).name() == methodName)
                return true;

        return false;
    }

    string name()
    {
        return className;
    }

    bool hasVariable(string variableName)
    {
        for (int i = 0; i < (int)variables.size(); i++)
            if (variables.at(i).name() == variableName)
                return true;

        return false;
    }
};