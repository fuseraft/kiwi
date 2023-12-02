#ifndef NOCTIS_ENGINE_H
#define NOCTIS_ENGINE_H

class Engine
{
private:
    std::vector<Method> methods;
    std::vector<Class> classes;
    std::vector<Variable> variables;
    std::vector<List> lists;
    std::vector<Constant> constants;
    std::vector<Method> ifStatements;
    std::vector<Method> forLoops;
    std::vector<Method> whileLoops;
    std::vector<std::string> args;
    std::vector<Module> modules;
    std::vector<Script> scripts;
    Switch mainSwitch;

public:
    Engine();
    ~Engine();

    void clearAll();
    void clearArgs();
    void clearConstants();
    void clearFor();
    void clearIf();
    void clearLists();
    void clearMethods();
    void clearClasses();
    void clearVariables();
    void clearWhile();
    void gc();

    void loadScript(std::string script);

    int indexOfConstant(std::string s);
    int indexOfList(std::string s);
    int indexOfMethod(std::string s);
    int indexOfModule(std::string s);
    int indexOfClass(std::string s);
    int indexOfScript(std::string s);
    int indexOfVariable(std::string s);

    Constant& getConstant(int index);
    Constant& getConstant(std::string s);
    List& getList(int index);
    List& getList(std::string s);
    Method& getMethod(int index);
    Method& getMethod(std::string s);
    Module& getModule(std::string s);
    Class& getClass(int index);
    Class& getClass(std::string s);
    Variable getClassVariable(std::string className, std::string variableName);
    Script& getScript();
    Variable& getVar(int index);
    Variable& getVar(std::string s);
    Switch& getMainSwitch();
    Method& getIfStatement(int index);
    Method& getForLoop(int index);
    Method& getWhileLoop(int index);

    void createMethod(std::string arg0, std::string arg1);

    void createFailedWhileLoop();
    void createWhileLoop(std::string v1, std::string op, std::string v2);

    void createFailedForLoop();
    void createForLoop();
    void createForLoop(double a, double b, std::string op);
    void createForLoop(List list);

    void createVariable(std::string name, double value);
    void createVariable(std::string name, std::string value);
    void setVariable(std::string name, double value);
    void setVariable(std::string name, std::string value);
    void replaceElement(std::string before, std::string after, std::string replacement);

    bool isNumber(std::string s);
    bool isNumber(Variable var);
    bool isString(std::string s);
    bool isString(Variable var);

    bool notClassMethod(std::string s);
    bool constantExists(std::string s);
    bool moduleExists(std::string s);
    bool variableExists(std::string s);
    bool classExists(std::string s);
    bool methodExists(std::string s);
    bool listExists(std::string s);

    void redefine(std::string target, std::string name);
    void globalize(std::string arg1);

    void removeConstant(std::string s);
    void removeList(std::string s);
    void removeMethod(std::string s);
    void removeModule(std::string s);
    void removeClass(std::string s);
    void removeVariable(std::string s);

    std::string varNumberString(std::string s);
    double varNumber(std::string s);
    std::string varString(std::string s);

    int getMethodCount();
    int getVariableCount();
    int getClassCount();
    int getListCount();
    int getConstantCount();

    void addConstant(Constant c);
    void addList(List l);
    void addMethod(Method m);
    void addModule(Module m);
    void addClass(Class c);
    void addVariable(Variable v);

    void addIfStatement(Method ifStatement);
    int getIfStatementCount();
    int getForLoopCount();
    int getWhileLoopCount();
    
    void addArg(std::string arg);
    int getArgCount();
    std::string getArg(int index);

    void createIfStatement(bool value);
    void createModule(std::string s);
    void createClass(std::string className);

    void addToCurrentForLoop(std::string line);
    void addToCurrentMethod(std::string s);
    void addToCurrentClassMethod(std::string s);

    void addItemToList(std::string listName, std::string value);
};

Engine::Engine() {}
Engine::~Engine() {}

void Engine::addToCurrentForLoop(std::string line)
{
    forLoops[forLoops.size() - 1].add(line);
}

void Engine::addItemToList(std::string listName, std::string value)
{
    this->getList(listName).add(value);
}

void Engine::addConstant(Constant c) { constants.push_back(c); }
void Engine::addList(List l) { lists.push_back(l); }
void Engine::addMethod(Method m) { methods.push_back(m); }
void Engine::addModule(Module m) { modules.push_back(m); }
void Engine::addClass(Class c) { classes.push_back(c); }
void Engine::addVariable(Variable v) { variables.push_back(v); }

void Engine::addIfStatement(Method ifStatement) { ifStatements.push_back(ifStatement); }
int Engine::getIfStatementCount() { return ifStatements.size(); }

int Engine::getMethodCount() { return methods.size(); }
int Engine::getVariableCount() { return variables.size(); }
int Engine::getClassCount() { return classes.size(); }
int Engine::getListCount() { return lists.size(); }
int Engine::getConstantCount() { return constants.size(); }
int Engine::getForLoopCount() { return forLoops.size(); }

Method& Engine::getMethod(int index) { return methods.at(index); }
Variable& Engine::getVar(int index) { return variables.at(index); }
Class& Engine::getClass(int index) { return classes.at(index); }
List& Engine::getList(int index) { return lists.at(index); }
Constant& Engine::getConstant(int index) { return constants.at(index); }

Method& Engine::getIfStatement(int index) { return ifStatements.at(index); }
Method& Engine::getForLoop(int index) { return forLoops.at(index); }

int Engine::getWhileLoopCount() { return whileLoops.size(); }
Method& Engine::getWhileLoop(int index) { return whileLoops.at(index); }

int Engine::getArgCount() { return args.size(); }
std::string Engine::getArg(int index) { return args.at(index); }
void Engine::addArg(std::string arg) { args.push_back(arg); }

void Engine::addToCurrentMethod(std::string s) { getMethod(getMethodCount() - 1).add(s); }
void Engine::addToCurrentClassMethod(std::string s) { getClass(getClassCount() - 1).addToCurrentMethod(s); }

void Engine::createClass(std::string className)
{
    if (classExists(className))
    {
        State.DefiningClass = true;
        State.CurrentClass = className;
    }
    else
    {
        Class newClass(className);
        State.CurrentClass = className;
        newClass.setCollectable(false);
        addClass(newClass);
        State.DefiningClass = true;
    }
}

void Engine::createModule(std::string s)
{
    std::string moduleName = s;
    moduleName = subtract_string(moduleName, "[");
    moduleName = subtract_string(moduleName, "]");

    Module newModule(moduleName);
    addModule(newModule);

    State.DefiningModule = true;
    State.CurrentModule = moduleName;
}

void Engine::createIfStatement(bool value)
{
    if (!value)
    {
        State.LastValue = Keywords.False;

        if (!State.DefiningNest)
        {
            Method ifMethod("[failif]");
            ifMethod.setBool(false);
            addIfStatement(ifMethod);

            State.DefiningIfStatement = true;
            State.FailedIfStatement = true;
            State.FailedNest = true;
        }
        else
            State.FailedNest = true;

        return;
    }

    State.LastValue = Keywords.True;

    if (State.DefiningNest)
    {
        getIfStatement(getIfStatementCount() - 1).buildNest();
        State.FailedNest = false;
    }
    else
    {
        Method ifMethod("[if#" + itos(State.IfStatementCount) + "]");
        ifMethod.setBool(true);
        State.DefiningIfStatement = true;
        addIfStatement(ifMethod);

        State.IfStatementCount++;
        State.FailedIfStatement = false;
        State.FailedNest = false;
    }
}

void Engine::createMethod(std::string arg0, std::string arg1)
{
    bool indestructable = arg0 == Keywords.LockedMethod;

    if (State.DefiningClass)
    {
        if (getClass(State.CurrentClass).hasMethod(arg1))
        {
            error(ErrorCode::METHOD_DEFINED, arg1, false);
            return;
        }

        if (!has_params(arg1))
        {
            Method method(arg1);

            if (State.DefiningPublicCode)
                method.setPublic();
            else if (State.DefiningPrivateCode)
                method.setPrivate();

            method.setClass(State.CurrentClass);
            getClass(State.CurrentClass).addMethod(method);
            getClass(State.CurrentClass).setCurrentMethod(arg1);
            State.DefiningMethod = true;
            State.DefiningClassMethod = true;
            return;
        }

        std::vector<std::string> params = parse_params(arg1);

        Method method(before_params(arg1));

        if (State.DefiningPublicCode)
            method.setPublic();
        else if (State.DefiningPrivateCode)
            method.setPrivate();

        method.setClass(State.CurrentClass);

        for (int i = 0; i < (int)params.size(); i++)
        {
            if (variableExists(params.at(i)))
            {
                if (is_dotless(params.at(i)))
                {
                    if (isString(params.at(i)))
                        method.addMethodVariable(varString(params.at(i)), getVar(params.at(i)).name());
                    else if (isNumber(params.at(i)))
                        method.addMethodVariable(varNumber(params.at(i)), getVar(params.at(i)).name());
                    else
                        error(ErrorCode::IS_NULL, params.at(i), false);
                    return;
                }

                std::string before(before_dot(params.at(i))), after(after_dot(params.at(i)));

                if (!classExists(before))
                {
                    error(ErrorCode::CLS_METHOD_UNDEFINED, before, false);
                    return;
                }

                if (!getClass(before).hasVariable(after))
                {
                    error(ErrorCode::CLS_VAR_UNDEFINED, after, false);
                    return;
                }

                if (getClass(before).getVariable(after).getString() != State.Null)
                    method.addMethodVariable(getClass(before).getVariable(after).getString(), after);
                else if (getClass(before).getVariable(after).getNumber() != State.NullNum)
                    method.addMethodVariable(getClass(before).getVariable(after).getNumber(), after);
                else
                    error(ErrorCode::IS_NULL, params.at(i), false);
            }
            else
            {
                if (is_alpha(params.at(i)))
                {
                    Variable newVariable("@[pm#" + itos(State.ParamVarCount) + "]", params.at(i));
                    method.addMethodVariable(newVariable);
                    State.ParamVarCount++;
                }
                else
                {
                    Variable newVariable("@[pm#" + itos(State.ParamVarCount) + "]", stod(params.at(i)));
                    method.addMethodVariable(newVariable);
                    State.ParamVarCount++;
                }
            }
        }

        getClass(State.CurrentClass).addMethod(method);
        getClass(State.CurrentClass).setCurrentMethod(before_params(arg1));
        State.DefiningMethod = true;
        State.DefiningParameterizedMethod = true;
        State.DefiningClassMethod = true;
    }
    else
    {
        if (methodExists(arg1))
            error(ErrorCode::METHOD_DEFINED, arg1, false);
        else
        {
            if (!is_dotless(arg1))
            {
                std::string before(before_dot(arg1)), after(after_dot(arg1));

                if (classExists(before))
                {
                    Method method(after);

                    if (State.DefiningPublicCode)
                        method.setPublic();
                    else if (State.DefiningPrivateCode)
                        method.setPrivate();

                    method.setClass(before);
                    getClass(before).addMethod(method);
                    getClass(before).setCurrentMethod(after);
                    State.DefiningMethod = true;
                    State.DefiningClassMethod = true;
                }
                else
                    error(ErrorCode::CLS_UNDEFINED, "", false);
            }
            else if (has_params(arg1))
            {
                std::vector<std::string> params = parse_params(arg1);

                Method method(before_params(arg1));
                method.setIndestructible(indestructable);

                for (int i = 0; i < (int)params.size(); i++)
                {
                    if (variableExists(params.at(i)))
                    {
                        if (!is_dotless(params.at(i)))
                        {
                            std::string before(before_dot(params.at(i))), after(after_dot(params.at(i)));

                            if (classExists(before))
                            {
                                if (getClass(before).hasVariable(after))
                                {
                                    if (getClass(before).getVariable(after).getString() != State.Null)
                                        method.addMethodVariable(getClass(before).getVariable(after).getString(), after);
                                    else if (getClass(before).getVariable(after).getNumber() != State.NullNum)
                                        method.addMethodVariable(getClass(before).getVariable(after).getNumber(), after);
                                    else
                                        error(ErrorCode::IS_NULL, params.at(i), false);
                                }
                                else
                                    error(ErrorCode::CLS_VAR_UNDEFINED, after, false);
                            }
                            else
                                error(ErrorCode::CLS_METHOD_UNDEFINED, before, false);
                        }
                        else
                        {
                            if (isString(params.at(i)))
                                method.addMethodVariable(varString(params.at(i)), getVar(params.at(i)).name());
                            else if (isNumber(params.at(i)))
                                method.addMethodVariable(varNumber(params.at(i)), getVar(params.at(i)).name());
                            else
                                error(ErrorCode::IS_NULL, params.at(i), false);
                        }
                    }
                    else
                    {
                        if (is_alpha(params.at(i)))
                        {
                            Variable newVariable("@" + params.at(i), "");
                            newVariable.setNull();
                            method.addMethodVariable(newVariable);
                            State.ParamVarCount++;
                        }
                        else
                        {
                            Variable newVariable("@" + params.at(i), 0);
                            newVariable.setNull();
                            method.addMethodVariable(newVariable);
                            State.ParamVarCount++;
                        }
                    }
                }

                methods.push_back(method);
                State.DefiningMethod = true;
                State.DefiningParameterizedMethod = true;
            }
            else
            {
                Method method(arg1);
                method.setIndestructible(indestructable);

                methods.push_back(method);
                State.DefiningMethod = true;
            }
        }
    }
}

void Engine::loadScript(std::string script)
{
    std::string s("");
    std::ifstream f(script.c_str());
    State.CurrentScript = script;

    scripts.clear();

    Script newScript(script);

    if (f.is_open())
    {
        while (!f.eof())
        {
            std::getline(f, s);

            if (s.length() > 0)
            {
                newScript.add(ltrim_ws(s));
            }
        }
    }

    scripts.push_back(newScript);
}

Switch& Engine::getMainSwitch()
{
    return mainSwitch;
}

void Engine::createWhileLoop(std::string v1, std::string op, std::string v2)
{
    Method whileMethod("[while#" + itos(State.WhileLoopCount) + "]");
    whileMethod.setWhile(true);
    whileMethod.setWhileValues(v1, op, v2);
    State.DefiningWhileLoop = true;
    whileLoops.push_back(whileMethod);
    State.WhileLoopCount++;
}

void Engine::createForLoop(List list)
{
    Method forMethod("[for#" + itos(State.ForLoopCount) + "]");
    forMethod.setFor(true);
    forMethod.setForList(list);
    forMethod.setListLoop();
    forMethod.setSymbol(State.DefaultLoopSymbol);
    State.DefiningForLoop = true;
    forLoops.push_back(forMethod);
    State.ForLoopCount++;
    State.SuccessFlag = true;
}

void Engine::createForLoop(double a, double b, std::string op)
{
    Method forMethod("[for#" + itos(State.ForLoopCount) + "]");
    forMethod.setFor(true);
    forMethod.setSymbol(State.DefaultLoopSymbol);

    if (op == Operators.LessThanOrEqual || op == Operators.GreaterThanOrEqual)
        forMethod.setForValues((int)a, (int)b);
    else if (op == Operators.LessThan || op == Operators.GreaterThan)
        forMethod.setForValues((int)a, (int)b - 1);

    State.DefiningForLoop = true;
    forLoops.push_back(forMethod);
    State.ForLoopCount++;
    State.SuccessFlag = true;
}

void Engine::createForLoop()
{
    Method forMethod("[for#" + itos(State.ForLoopCount) + "]");
    forMethod.setFor(true);
    forMethod.setInfinite();
    State.DefiningForLoop = true;
    forLoops.push_back(forMethod);
    State.ForLoopCount++;
    State.SuccessFlag = true;
}

void Engine::createFailedForLoop()
{
    Method forMethod("[for#" + itos(State.ForLoopCount) + "]");
    forMethod.setFor(false);
    State.DefiningForLoop = true;
    forLoops.push_back(forMethod);
    State.DefaultLoopSymbol = "$";
    State.SuccessFlag = false;
}

void Engine::createFailedWhileLoop()
{
    Method whileMethod("[while#" + itos(State.WhileLoopCount) + "]");
    whileMethod.setWhile(false);
    State.DefiningWhileLoop = true;
    whileLoops.push_back(whileMethod);
}

bool Engine::isNumber(std::string s) { return varNumber(s) != State.NullNum; }
bool Engine::isNumber(Variable var) { return var.getNumber() != State.NullNum; }
bool Engine::isString(std::string s) { return varString(s) != State.Null; }
bool Engine::isString(Variable var) { return var.getString() != State.Null; }

int Engine::indexOfConstant(std::string s)
{
    for (unsigned i = 0; i < constants.size(); ++i)
    {
        if (constants.at(i).name() == s)
            return i;
    }

    return -1;
}

int Engine::indexOfList(std::string s)
{
    for (unsigned i = 0; i < lists.size(); ++i)
    {
        if (lists.at(i).name() == s)
            return i;
    }

    return -1;
}

int Engine::indexOfMethod(std::string s)
{
    for (unsigned i = 0; i < methods.size(); ++i)
    {
        if (methods.at(i).name() == s)
            return i;
    }

    return -1;
}

int Engine::indexOfModule(std::string s)
{
    for (unsigned i = 0; i < modules.size(); ++i)
    {
        if (modules.at(i).name() == s)
            return i;
    }

    return -1;
}

int Engine::indexOfClass(std::string s)
{
    for (unsigned i = 0; i < classes.size(); ++i)
    {
        if (classes.at(i).name() == s)
            return i;
    }

    return -1;
}

int Engine::indexOfScript(std::string s)
{
    for (unsigned i = 0; i < scripts.size(); ++i)
    {
        if (scripts.at(i).name() == s)
            return i;
    }

    return -1;
}

int Engine::indexOfVariable(std::string s)
{
    for (unsigned i = 0; i < variables.size(); ++i)
    {
        if (variables.at(i).name() == s)
            return i;
    }

    return -1;
}

Constant& Engine::getConstant(std::string s) { return constants.at(indexOfConstant(s)); }
List& Engine::getList(std::string s) { return lists.at(indexOfList(s)); }
Method& Engine::getMethod(std::string s) { return methods.at(indexOfMethod(s)); }
Module& Engine::getModule(std::string s) { return modules.at(indexOfModule(s)); }
Class& Engine::getClass(std::string s) { return classes.at(indexOfClass(s)); }
Script& Engine::getScript() { return scripts.at(0); }
Variable& Engine::getVar(std::string s) { return variables.at(indexOfVariable(s)); }
Variable Engine::getClassVariable(std::string className, std::string variableName)
{
    return getClass(className).getVariable(variableName);
}

void Engine::removeConstant(std::string s) { constants.erase(constants.begin() + indexOfConstant(s)); }
void Engine::removeList(std::string s) { lists.erase(lists.begin() + indexOfList(s)); }
void Engine::removeMethod(std::string s) { methods.erase(methods.begin() + indexOfMethod(s)); }
void Engine::removeModule(std::string s) { modules.erase(modules.begin() + indexOfModule(s)); }
void Engine::removeClass(std::string s) { classes.erase(classes.begin() + indexOfClass(s)); }
void Engine::removeVariable(std::string s) { variables.erase(variables.begin() + indexOfVariable(s)); }

std::string Engine::varString(std::string s) { return getVar(s).getString(); }
double Engine::varNumber(std::string s) { return getVar(s).getNumber(); }
std::string Engine::varNumberString(std::string s) { return dtos(varNumber(s)); }

void Engine::redefine(std::string target, std::string name)
{
    if (variableExists(target))
    {
        if (Env::fileExists(varString(target)) || Env::directoryExists(varString(target)))
        {
            std::string old_name(varString(target)), new_name("");

            if (variableExists(name))
            {
                if (isString(name))
                {
                    new_name = varString(name);

                    if (Env::fileExists(old_name))
                    {
                        if (!Env::fileExists(new_name))
                        {
                            if (Env::fileExists(old_name))
                                rename(old_name.c_str(), new_name.c_str());
                            else
                                error(ErrorCode::FILE_NOT_FOUND, old_name, false);
                        }
                        else
                            error(ErrorCode::FILE_EXISTS, new_name, false);
                    }
                    else if (Env::directoryExists(old_name))
                    {
                        if (!Env::directoryExists(new_name))
                        {
                            if (Env::directoryExists(old_name))
                                rename(old_name.c_str(), new_name.c_str());
                            else
                                error(ErrorCode::DIR_NOT_FOUND, old_name, false);
                        }
                        else
                            error(ErrorCode::DIR_EXISTS, new_name, false);
                    }
                    else
                        error(ErrorCode::TARGET_UNDEFINED, old_name, false);
                }
                else
                    error(ErrorCode::NULL_STRING, name, false);
            }
            else
            {
                if (Env::fileExists(old_name))
                {
                    if (!Env::fileExists(name))
                        rename(old_name.c_str(), name.c_str());
                    else
                        error(ErrorCode::FILE_EXISTS, name, false);
                }
                else if (Env::directoryExists(old_name))
                {
                    if (!Env::directoryExists(name))
                        rename(old_name.c_str(), name.c_str());
                    else
                        error(ErrorCode::DIR_EXISTS, name, false);
                }
                else
                    error(ErrorCode::TARGET_UNDEFINED, old_name, false);
            }
        }
        else
        {
            if (begins_with(name, "@"))
            {
                if (!variableExists(name))
                    getVar(target).setName(name);
                else
                    error(ErrorCode::VAR_DEFINED, name, false);
            }
            else
                error(ErrorCode::INVALID_VAR_DECL, name, false);
        }
    }
    else if (listExists(target))
    {
        if (!listExists(name))
            getList(name).setName(name);
        else
            error(ErrorCode::LIST_UNDEFINED, name, false);
    }
    else if (classExists(target))
    {
        if (!classExists(name))
            getClass(target).setName(name);
        else
            error(ErrorCode::CLS_METHOD_UNDEFINED, name, false);
    }
    else if (methodExists(target))
    {
        if (!methodExists(name))
            getMethod(target).setName(name);
        else
            error(ErrorCode::METHOD_UNDEFINED, name, false);
    }
    else if (Env::fileExists(target) || Env::directoryExists(target))
        rename(target.c_str(), name.c_str());
    else
        error(ErrorCode::TARGET_UNDEFINED, target, false);
}

void Engine::globalize(std::string arg1)
{
    if (!(contains(arg1, Keywords.Dot) && methodExists(arg1) && !methodExists(after_dot(arg1))))
    {
        error(ErrorCode::CLS_METHOD_UNDEFINED, arg1, false);
        return;
    }

    Method method(after_dot(arg1));

    std::vector<std::string> lines = getClass(before_dot(arg1)).getMethod(after_dot(arg1)).getLines();

    for (int i = 0; i < (int)lines.size(); i++)
        method.add(lines[i]);

    methods.push_back(method);
}

void Engine::replaceElement(std::string before, std::string after, std::string replacement)
{
    std::vector<std::string> newList;

    for (int i = 0; i < (int)getList(before).size(); i++)
    {
        if (i == stoi(after))
            newList.push_back(replacement);
        else
            newList.push_back(getList(before).at(i));
    }

    getList(before).clear();

    for (int i = 0; i < (int)newList.size(); i++)
        addItemToList(before, newList.at(i));

    newList.clear();
}

void Engine::setVariable(std::string name, std::string value)
{
    getVar(name).setVariable(value);
    State.LastValue = value;
}

void Engine::setVariable(std::string name, double value)
{
    if (varString(name) != State.Null)
        getVar(name).setVariable(dtos(value));
    else if (varNumber(name) != State.NullNum)
        getVar(name).setVariable(value);
    else
    {
        if (getVar(name).waiting())
            getVar(name).stopWait();

        getVar(name).setVariable(value);
    }

    State.LastValue = dtos(value);
}

void Engine::createVariable(std::string name, std::string value)
{
    Variable newVariable(name, value);

    newVariable.setCollectable(State.ExecutedTemplate || State.ExecutedMethod || State.ExecutedTryBlock);

    variables.push_back(newVariable);
    State.LastValue = value;
}

void Engine::createVariable(std::string name, double value)
{
    Variable newVariable(name, value);

    newVariable.setCollectable(State.ExecutedTemplate || State.ExecutedMethod || State.ExecutedTryBlock);

    variables.push_back(newVariable);
    State.LastValue = dtos(value);
}

void Engine::clearAll()
{
    clearMethods();
    clearClasses();
    clearVariables();
    clearLists();
    clearArgs();
    clearIf();
    clearFor();
    clearWhile();
    clearConstants();
}

void Engine::clearConstants()
{
    constants.clear();
}

void Engine::clearArgs()
{
    args.clear();
}

void Engine::clearFor()
{
    forLoops.clear();
}

void Engine::clearWhile()
{
    whileLoops.clear();
}

void Engine::clearIf()
{
    ifStatements.clear();
}

void Engine::clearLists()
{
    lists.clear();
}

void Engine::clearMethods()
{
    std::vector<Method> indestructibleMethods;

    for (unsigned i = 0; i < methods.size(); ++i)
        if (methods.at(i).indestructible())
            indestructibleMethods.push_back(methods.at(i));

    methods.clear();

    for (unsigned i = 0; i < indestructibleMethods.size(); ++i)
        methods.push_back(indestructibleMethods[i]);
}

// TODO: Implement indestructible objects?
void Engine::clearClasses() { classes.clear(); }

void Engine::clearVariables()
{
    std::vector<Variable> indestructibleVariables;

    for (unsigned i = 0; i < variables.size(); ++i)
        if (variables.at(i).indestructible())
            indestructibleVariables.push_back(variables.at(i));

    variables.clear();

    for (unsigned i = 0; i < indestructibleVariables.size(); ++i)
        variables.push_back(indestructibleVariables[i]);
}

void Engine::gc()
{
    std::vector<std::string> garbageVars;

    for (unsigned i = 0; i < variables.size(); ++i)
        if (variables.at(i).isCollectable() && !State.ExecutedIfStatement)
            if (!State.DontCollectMethodVars)
                garbageVars.push_back(variables.at(i).name());

    for (unsigned i = 0; i < garbageVars.size(); ++i)
        removeVariable(garbageVars.at(i));

    std::vector<std::string> garbageLists;

    for (unsigned i = 0; i < lists.size(); ++i)
        if (lists.at(i).isCollectable() && !State.ExecutedIfStatement)
            garbageLists.push_back(lists.at(i).name());

    for (unsigned i = 0; i < garbageLists.size(); ++i)
        removeList(garbageLists.at(i));

    std::vector<std::string> garbageClasses;

    for (unsigned i = 0; i < classes.size(); ++i)
        if (classes.at(i).isCollectable() && !State.ExecutedIfStatement)
            garbageClasses.push_back(classes.at(i).name());

    for (unsigned i = 0; i < garbageClasses.size(); ++i)
        removeClass(garbageClasses.at(i));
}

bool Engine::listExists(std::string s)
{
    for (unsigned i = 0; i < lists.size(); ++i)
        if (lists.at(i).name() == s)
            return true;

    return false;
}

bool Engine::methodExists(std::string s)
{
    if (!is_dotless(s) && classExists(before_dot(s)))
    {
        return getClass(before_dot(s)).hasMethod(after_dot(s));
    }

    for (unsigned i = 0; i < methods.size(); ++i)
        if (methods.at(i).name() == s)
            return true;

    return false;
}

bool Engine::classExists(std::string s)
{
    for (unsigned i = 0; i < classes.size(); ++i)
        if (classes.at(i).name() == s)
            return true;

    return false;
}

bool Engine::variableExists(std::string s)
{
    if (!is_dotless(s))
    {
        std::string before(before_dot(s)), after(after_dot(s));
        return classExists(before) && getClass(before).hasVariable(after);
    }

    for (unsigned i = 0; i < variables.size(); ++i)
        if (variables.at(i).name() == s)
            return true;

    return false;
}

bool Engine::moduleExists(std::string s)
{
    for (unsigned i = 0; i < modules.size(); ++i)
        if (modules.at(i).name() == s)
            return true;

    return false;
}

bool Engine::constantExists(std::string s)
{
    for (unsigned i = 0; i < constants.size(); ++i)
        if (constants.at(i).name() == s)
            return true;

    return false;
}

bool Engine::notClassMethod(std::string s)
{
    if (!is_dotless(s))
    {
        std::string before(before_dot(s));
        return !classExists(before);
    }

    return true;
}

#endif
